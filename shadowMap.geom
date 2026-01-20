#version 330 core

layout (triangles) in;
layout (triangle_strip, max_vertices=18) out;

uniform mat4 shadowMatrices[6];

in vec4 WorldPos[];
out vec4 FragPos;

void main(){
	for(int face = 0; face < 6; ++face){
		gl_Layer = face;
		for(int i = 0; i < 3; ++i){
			FragPos = WorldPos[i];
			gl_Position = shadowMatrices[face] * FragPos;
			EmitVertex();
		}
		EndPrimitive();
	}
}