#include "VBO.h"

VBO::VBO(std::vector<Vertex>& vertices)
{
	glGenBuffers(1, &ID);
	glBindBuffer(GL_ARRAY_BUFFER, ID);
	capacity = vertices.size();
	glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), vertices.data(), GL_STATIC_DRAW);
}
VBO::VBO(const std::vector<glm::mat4>& mat4s)
{
	glGenBuffers(1, &ID);
	glBindBuffer(GL_ARRAY_BUFFER, ID);
	capacity = mat4s.size();
	glBufferData(GL_ARRAY_BUFFER, mat4s.size() * sizeof(glm::mat4), mat4s.data(), GL_DYNAMIC_DRAW);
}

VBO::VBO(size_t maxCount)
{
	glGenBuffers(1, &ID);
	glBindBuffer(GL_ARRAY_BUFFER, ID);
	capacity = maxCount;
	//alloc without initializing to avoid future re-allocations
	glBufferData(GL_ARRAY_BUFFER, capacity * sizeof(glm::mat4), nullptr, GL_DYNAMIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void VBO::UpdateData(const std::vector<glm::mat4>& mat4s)
{
	glBindBuffer(GL_ARRAY_BUFFER, ID);
	size_t bytes = mat4s.size() * sizeof(glm::mat4);
	if (mat4s.size() > capacity) {
		//realloc to support multiple instances
		capacity = mat4s.size();
		glBufferData(GL_ARRAY_BUFFER, capacity * sizeof(glm::mat4), mat4s.data(), GL_DYNAMIC_DRAW);
	}
	else {
		//updates existing data without reallocations
		glBufferSubData(GL_ARRAY_BUFFER, 0, bytes, mat4s.data());
	}
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void VBO::Bind()
{
	glBindBuffer(GL_ARRAY_BUFFER, ID);
}

void VBO::Unbind()
{
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void VBO::Delete()
{
	glDeleteBuffers(1, &ID);
}
