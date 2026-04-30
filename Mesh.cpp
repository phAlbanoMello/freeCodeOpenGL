#include "Mesh.h"

//Class Responsibility :
//Represent a Mesh object created with data processed by the model from glTF files
//Wraps logic for drawing each mesh, binding the VAO and textures

Mesh::Mesh(std::vector<Vertex>& vertices, std::vector<GLuint>& indices, std::vector<Texture>& textures, glm::vec3 localMin, glm::vec3 localMax)
{
	Mesh::vertices = vertices;
	Mesh::indices = indices;
	Mesh::textures = textures;

	localMinBounds = localMin;
	localMaxBounds = localMax;

	VAO.Bind();
	VBO VBO(vertices);
	EBO EBO(indices);

	VAO.LinkAttrib(VBO, 0, 3, GL_FLOAT, sizeof(Vertex), (void*)0); //Position
	VAO.LinkAttrib(VBO, 1, 3, GL_FLOAT, sizeof(Vertex), (void*)(3 * sizeof(float))); //Normals
	VAO.LinkAttrib(VBO, 2, 3, GL_FLOAT, sizeof(Vertex), (void*)(6 * sizeof(float))); //Colors
	VAO.LinkAttrib(VBO, 3, 2, GL_FLOAT, sizeof(Vertex), (void*)(9 * sizeof(float))); //Texture Coordinates
    VAO.LinkAttrib(VBO, 4, 3, GL_FLOAT, sizeof(Vertex), (void*)(11 * sizeof(float))); //Tangent

	VAO.Unbind();
	VBO.Unbind();
	EBO.Unbind();
}

void Mesh::Draw(Shader& shader,Camera& camera,const glm::mat4& meshLocalMatrix, const glm::mat4& modelWorldMatrix)
{
    shader.Activate();
    VAO.Bind();

    unsigned int diffuseCount = 0;
    unsigned int specularCount = 0;

    for (unsigned int i = 0; i < textures.size(); ++i)
    {
        std::string uniformName;
        const std::string& type = textures[i].type;

        if (type == "diffuse")
        {
            uniformName = type + std::to_string(diffuseCount++);
        }
        else if (type == "specular")
        {
            uniformName = type + std::to_string(specularCount++);
        }

        textures[i].texUnit(shader, uniformName.c_str(), i);
        textures[i].Bind();
    }

    glUniform3f(glGetUniformLocation(shader.ID, "camPos"),camera.Position.x,camera.Position.y,camera.Position.z);

    camera.Matrix(shader, "camMatrix");

    glm::mat4 finalModelMatrix = meshLocalMatrix * modelWorldMatrix;

    glUniformMatrix4fv(glGetUniformLocation(shader.ID, "model"),1, GL_FALSE,glm::value_ptr(finalModelMatrix));

    glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(indices.size()), GL_UNSIGNED_INT, 0);
}
