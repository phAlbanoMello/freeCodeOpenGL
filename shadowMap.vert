#version 330 core
layout (location = 0) in vec3 aPos;

uniform mat4 model;

out vec4 WorldPos;

void main()
{
    WorldPos = model * vec4(aPos, 1.0);
    gl_Position =  WorldPos;
}