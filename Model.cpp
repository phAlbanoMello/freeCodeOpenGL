#include "Model.h"

Model::Model(const char* file, bool flipUV_Y)
{
	this->flipUV_Y = flipUV_Y;
	//Abstract glTF file in a JSON structure
	std::string text = get_file_contents(file);
	JSON = json::parse(text);

	Model::file = file;
	data = getData();
	
	traverseNode(0);
}

void Model::Draw(
	Shader& shader,
	Camera& camera,
	glm::vec3 translation,
	glm::quat rotation,
	glm::vec3 scale)
{
	// Build transformation matrix from translation, rotation (as matrix), and scale
	glm::mat4 model = glm::translate(glm::mat4(1.0f), translation) *
		glm::toMat4(rotation) *
		glm::scale(glm::mat4(1.0f), scale);

	// Draw each mesh with the final transformation
	for (unsigned int i = 0; i < meshes.size(); i++)
	{
		glm::mat4 finalMatrix = model * matricesMeshes[i]; // Apply per-mesh transformation
		meshes[i].Mesh::Draw(shader, camera, finalMatrix);
	}
}

void Model::loadMesh(unsigned int indMesh)
{
	unsigned int posAccInd = JSON["meshes"][indMesh]["primitives"][0]["attributes"]["POSITION"];
	unsigned int normalAccInd = JSON["meshes"][indMesh]["primitives"][0]["attributes"]["NORMAL"];
	unsigned int texAccInd = JSON["meshes"][indMesh]["primitives"][0]["attributes"]["TEXCOORD_0"];
	unsigned int indAccInd = JSON["meshes"][indMesh]["primitives"][0]["indices"];

	std::vector<float> posVec = getFloats(JSON["accessors"][posAccInd]);
	std::vector<glm::vec3> positions = groupFloatsVecN<glm::vec3, 3>(posVec);
	std::vector<float> normalVec = getFloats(JSON["accessors"][normalAccInd]);

	std::vector<glm::vec3> normals = groupFloatsVecN<glm::vec3, 3>(normalVec);
	std::vector<float> texVec = getFloats(JSON["accessors"][texAccInd]);
	std::vector<glm::vec2> texUVs = groupFloatsVecN<glm::vec2, 2>(texVec);

	std::vector<Vertex> vertices = assembleVertices(positions, normals, texUVs);
	std::vector<GLuint> indices = getIndices(JSON["accessors"][indAccInd]);
	std::vector<Texture> textures = getTextures();

	meshes.push_back(Mesh(vertices, indices, textures));
}

void Model::traverseNode(unsigned int nextNode, glm::mat4 matrix)
{
	json node = JSON["nodes"][nextNode];

	glm::vec3 translation = glm::vec3(0., 0., 0.);
	if (node.find("translation") != node.end())
	{
		float transValues[3];
		for (unsigned int i = 0; i < node["translation"].size(); i++)
		{
			transValues[i] = (node["translation"][i]);
			translation = glm::make_vec3(transValues);
		}
	}

	glm::quat rotation = glm::quat(1., 0., 0., 0.);
	if (node.find("rotation") != node.end()) {
		float rotValues[4] = {
			node["rotation"][3],
			node["rotation"][0],
			node["rotation"][1],
			node["rotation"][2]
		};
		rotation = glm::make_quat(rotValues);
	}
	
	glm::vec3 scale = glm::vec3(1.f, 1.f, 1.f);
	if (node.find("scale") != node.end())
	{
		float scaleValues[3]; 
		for (unsigned int i = 0; i < node["scale"].size(); i++)
		{
			scaleValues[i] = (node["scale"][i]);
			scale = glm::make_vec3(scaleValues);
		}
	}

	glm::mat4 matNode = glm::mat4(1.f);
	if (node.find("Matrix") != node.end())
	{
		float matValues[16];
		for (unsigned int i = 0; i < node["matrix"].size(); i++)
		{
			matValues[i] = (node["matrix"][i]);
			matNode = glm::make_mat4(matValues);
		}
	}

	glm::mat4 trans = glm::mat4(1.f);
	glm::mat4 rot = glm::mat4(1.f);
	glm::mat4 sca = glm::mat4(1.f);

	trans = glm::translate(trans, translation);
	rot = glm::mat4_cast(rotation);
	sca = glm::scale(sca, scale);

	glm::mat4 matNextNode = matrix * matNode * trans * rot * sca;

	if (node.find("mesh") != node.end())
	{
		translationsMeshes.push_back(translation);
		rotationsMeshes.push_back(rotation);
		scalesMeshes.push_back(scale);
		matricesMeshes.push_back(matNextNode);

		loadMesh(node["mesh"]);
	}
	if (node.find("children") != node.end())
	{
		for (unsigned int i = 0; i < node["children"].size(); i++)
		{
			traverseNode(node["children"][i], matNextNode);
		}
	}
}

std::vector<unsigned char> Model::getData()
{
	//Getting the location of the file
	std::string bytesText;
	//uri key gives the name of a file with the binary data.
	//I'll access the first element of the "buffers" dictionary and read the value for the "uri" key.
	std::string uri = JSON["buffers"][0]["uri"];

	//Store raw text data into bytesText
	std::string fileStr = std::string(file);
	std::string fileDirectory = fileStr.substr(0, fileStr.find_last_of('/') + 1);
	bytesText = get_file_contents((fileDirectory + uri).c_str());

	//Transform the raw text data into bytes and put them in a vector
	std::vector<unsigned char> data(bytesText.begin(), bytesText.end());
	return data;
}

std::vector<float> Model::getFloats(json accessor)
{
	std::vector<float> floatVec;

	unsigned int bufferViewInd = accessor.value("bufferView", 1);
	unsigned int count = accessor["count"];
	unsigned int accByteOffset = accessor.value("byteOffset", 0);
	std::string type = accessor["type"];

	json bufferView = JSON["bufferViews"][bufferViewInd];
	unsigned int byteOffset = bufferView["byteOffset"];

	unsigned int numPerVert;
	if (type == "SCALAR") numPerVert = 1;
	else if (type == "VEC2") numPerVert = 2;
	else if (type == "VEC3") numPerVert = 3;
	else if (type == "VEC4") numPerVert = 4;
	else throw std::invalid_argument("Type is invalid (not SCALAR, VEC2, VEC3, or VEC4)");

	unsigned int beginningOfData = byteOffset + accByteOffset;
	unsigned int lenghtOfData = count * 4 * numPerVert;
	for (unsigned int i = beginningOfData; i < beginningOfData + lenghtOfData; i)
	{
		unsigned char bytes[] = { data[i++], data[i++], data[i++], data[i++] };
		float value;
		std::memcpy(&value, bytes, sizeof(float));
		floatVec.push_back(value);
	}
	return floatVec;
}

std::vector<GLuint> Model::getIndices(json accessor)
{
	std::vector<GLuint> indices;

	unsigned int bufferViewInd = accessor.value("bufferView", 0);
	unsigned int count = accessor["count"];
	unsigned int accByteOffset = accessor.value("byteOffset", 0);
	unsigned int componentType = accessor["componentType"];

	json bufferView = JSON["bufferViews"][bufferViewInd];
	unsigned int byteOffset = bufferView["byteOffset"];

	unsigned int beginningOfData = byteOffset + accByteOffset;
	if (componentType == 5125)
	{
		for (unsigned int i = beginningOfData; i < byteOffset + accByteOffset + count * 4; i)
		{
			unsigned char bytes[] = { data[i++], data[i++], data[i++], data[i++] };
			unsigned int value;
			std::memcpy(&value, bytes, sizeof(unsigned int));
			indices.push_back((GLuint)value);
		}
	}
	else if (componentType == 5123) {
		for (unsigned int i = beginningOfData; i < byteOffset + accByteOffset + count * 2; i) {
			unsigned char bytes[] = { data[i++], data[i++] };
			unsigned short value;
			std::memcpy(&value, bytes, sizeof(unsigned short));
			indices.push_back((GLuint)value);
		}
	}
	else if (componentType == 5123) {
		for (unsigned int i = beginningOfData; i < byteOffset + accByteOffset + count * 2; i) {
			unsigned char bytes[] = { data[i++], data[i++] };
			short value;
			std::memcpy(&value, bytes, sizeof(short));
			indices.push_back((GLuint)value);
		}
	}
	return indices;
}

std::vector<Texture> Model::getTextures()
{
	std::vector<Texture> textures;

	std::string fileStr = std::string(file);
	std::string fileDirectory = fileStr.substr(0, fileStr.find_last_of('/') + 1);

	for (unsigned int i = 0; i < JSON["images"].size(); i++)
	{
		std::string texPath = JSON["images"][i]["uri"];

		bool skip = false;
		for (unsigned int j = 0; j < loadedTexName.size(); j++)
		{
			if (loadedTexName[j] == texPath)
			{
				textures.push_back(loadedTex[j]);
				skip = true;
				break;
			}
		}

		if (!skip)
		{
			if (texPath.find("baseColor") != std::string::npos || texPath.find("diffuse") != std::string::npos)
			{
				Texture diffuse = Texture((fileDirectory + texPath).c_str(), "diffuse", loadedTex.size());
				textures.push_back(diffuse);
				loadedTex.push_back(diffuse);
				loadedTexName.push_back(texPath);
			}
			else if (texPath.find("metallicRoughness") != std::string::npos || texPath.find("specular") != std::string::npos) {
				Texture specular = Texture((fileDirectory + texPath).c_str(), "specular", loadedTex.size());
				textures.push_back(specular);
				loadedTex.push_back(specular);
				loadedTexName.push_back(texPath);
			}
		}
	}

	return textures;
}

std::vector<Vertex> Model::assembleVertices(std::vector<glm::vec3> positions, std::vector<glm::vec3> normals, std::vector<glm::vec2> texUVs)
{
	std::vector<Vertex> vertices;
	for (int i = 0; i < positions.size(); i++)
	{
		vertices.push_back(Vertex{ positions[i], normals[i], glm::vec3(1.f, 1.f, 1.f), texUVs[i] });
	}
	return vertices;
}

//std::vector<glm::vec2> Model::groupFloatsVec2(std::vector<float> floatVec)
//{
//	const unsigned int floatsPerVector = 2;
//
//	std::vector<glm::vec2> vectors;
//	for (unsigned int i = 0; i < floatVec.size(); i += floatsPerVector)
//	{
//		vectors.push_back(glm::vec2(0, 0));
//		for (unsigned int j = 0; j < floatsPerVector; j++)
//		{
//			vectors.back()[j] = floatVec[i + j];
//		}
//	}
//	return vectors;
//}
//
//std::vector<glm::vec3> Model::groupFloatsVec3(std::vector<float> floatVec)
//{
//	const unsigned int floatsPerVector = 3;
//
//	std::vector<glm::vec3> vectors;
//	for (unsigned int i = 0; i < floatVec.size(); i += floatsPerVector)
//	{
//		vectors.push_back(glm::vec3(0, 0, 0));
//		for (unsigned int j = 0; j < floatsPerVector; j++)
//		{
//			vectors.back()[j] = floatVec[i + j];
//		}
//	}
//	return vectors;
//}
//
//std::vector<glm::vec4> Model::groupFloatsVec4(std::vector<float> floatVec)
//{
//	const unsigned int floatsPerVector = 4;
//
//	std::vector<glm::vec4> vectors;
//	for (unsigned int i = 0; i < floatVec.size(); i += floatsPerVector)
//	{
//		vectors.push_back(glm::vec4(0, 0, 0, 0));
//		for (unsigned int j = 0; j < floatsPerVector; j++)
//		{
//			vectors.back()[j] = floatVec[i + j];
//		}
//	}
//	return vectors;
//}

template<typename VecType, int N>
std::vector<VecType> Model::groupFloatsVecN(const std::vector<float>& floatVec)
{
	std::vector<VecType> vectors;
	vectors.reserve(floatVec.size() / N);

	for (size_t i = 0; i < floatVec.size(); i += N)
	{
		VecType v;
		for (int j = 0; j < N; ++j)
		{
			v[j] = floatVec[i + j];
		}

		if(N == 2)
		{
			if (flipUV_Y)
			{
				v[1] = 1.f - v[1];
			}
		}

		vectors.push_back(v);
	}

	return vectors;
}
