#ifndef EBO_CLASS_H
#define EBO_CLASS_H

#include<glad/glad.h>
#include<vector>

class EBO {
public:
	//ID reference of Elements Buffer Object
	GLuint ID;
	EBO(std::vector<GLuint>& indices);
	
	void Bind();
	void Unbind();
	void Delete();
};

#endif