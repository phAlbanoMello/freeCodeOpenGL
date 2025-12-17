#ifndef SHADER_CLASS_H
#define SHADER_CLASS_H

#include<glad/glad.h>
#include<string>
#include<fstream>
#include<sstream>
#include<iostream>
#include<cerrno>
#include<filesystem>
#include <glm/ext/vector_float3.hpp>
#include <glm/ext/matrix_float4x4.hpp>

std::string get_file_contents(const char* filename);

class Shader {
public:
	GLuint ID;
	Shader(const char* vertexFile, const char* fragmentFile);

	void Activate();
	void Delete();

	void SetVec(const char* uniformName, const glm::vec2& value);
	void SetVec(const char* uniformName, const glm::vec3& value);
	void SetVec(const char* uniformName, const glm::vec4& value);
	void SetInt(const char* uniformName, int value);
	void SetFloat(const char* uniformName, float value);
	void SetMatrix4(const char* uniformName, glm::mat4 matrix);
	void compileErrors(unsigned int shader, const char* type);
};

#endif