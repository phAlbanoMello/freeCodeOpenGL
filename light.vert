#version 330 core

layout (location = 0) in vec3 aPos;

uniform mat4 model;
uniform mat4 camMatrix;
uniform float sizeScale;

void main()
{
	vec3 scaledPos = aPos * sizeScale;
	gl_Position = camMatrix * model * vec4(aPos, 1.0f);
}