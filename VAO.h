#ifndef VAO_CLASS_H
#define VAO_CLASS_H

#include<glad/glad.h>
#include "VBO.h"

class VAO {
public:
	GLuint ID;
	VAO();

	//TODO : Change Link VBO to Link Attrib
	//Check video at 41:37
	void LinkAttrib(VBO& VBO, GLuint layout, GLuint numComponents, GLenum type, GLsizeiptr stride, void* offset);
	void Bind();
	void Unbind();
	void Delete();
};

#endif
