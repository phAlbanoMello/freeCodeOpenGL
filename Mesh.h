#ifndef MESH_CLASS_H
#define MESH_CLASS_H

#include <string>
#include <vector>

#include"VAO.h"
#include"EBO.h"
#include"Camera.h"
#include"Texture.h"

class Mesh
{
public:
	std::vector <Vertex> vertices;
	std::vector <GLuint> indices;
	std::vector <Texture> textures;
	std::vector <glm::mat4> instanceMatrix;

	VAO VAO;

	unsigned int instanceCount;

	Mesh
	(
		std::vector <Vertex>& vertices, 
		std::vector <GLuint>& indices, 
		std::vector <Texture>& textures,
		unsigned int instanceCount = 1,
		std::vector <glm::mat4> instanceMatrix = {}
	);

	void Draw
	(
		Shader& shader,
		Camera& camera,
		glm::mat4 matrix = glm::mat4(1.f),
		glm::vec3 translation = glm::vec3(0., 0., 0.),
		glm::quat rotation = glm::quat(1.f, 0.f, 0.f, 0.f),
		glm::vec3 scale = glm::vec3(1.f, 1.f, 1.f)
	);
	void BindTextures(Shader& shader);
};
#endif