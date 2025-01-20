#ifndef EBO_CLASS_H
#define EBO_CLASS_H

#include<glad/glad.h>

class EBO {
public:
	//ID reference of Elements Buffer Object
	GLuint ID;
	EBO(GLuint* indices, GLsizeiptr size);
	
	void Bind();
	void Unbind();
	void Delete();
};

#endif