#ifndef VBO_CLASS_H
#define VBO_CLASS_H

#include<glm/glm.hpp>
#include<glad/glad.h>
#include<vector>
struct Vertex {
	glm::vec3 position;
	glm::vec3 normal;
	glm::vec3 color;
	glm::vec2 texUV;
};


class VBO {
public:
	GLuint ID;
	VBO(std::vector<Vertex>& vertices);
	//VBO(std::vector<glm::mat4>& mat4s);
	//constructor for initial data with DYNAMIC_DRAW
	VBO(const std::vector<glm::mat4>& mat4s);

	//max number of mat4s alocatted at the buffer
	size_t capacity; 

	//constructor for preallocating capacity without initializing data
	VBO(size_t maxCount);

	void UpdateData(const std::vector<glm::mat4>& mat4s);

	void Bind();
	void Unbind();
	void Delete();
};

#endif

