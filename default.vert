#version 330 core

// Vertex attributes: position, color, texture coordinates, and normal
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aColor;
layout (location = 2) in vec2 aTex;
layout (location = 3) in vec3 aNormal;

// Outputs to the fragment shader: color, texture coordinates, normal, and current position
out vec3 color;
out vec2 texCoord;
out vec3 Normal;
out vec3 crntPos;

// Uniform matrices: camera (view + projection) and model (object transformation)
uniform mat4 camMatrix;
uniform mat4 model;

void main()
{
   //calculates current vertex position to world space (model matrix)
   crntPos = vec3(model * vec4(aPos, 1.f));

   // Transform vertex position to clip space and set gl_Position
   gl_Position = camMatrix * vec4(crntPos, 1.0);
   
   // Pass color and texture coordinates to the fragment shader
   color = aColor;
   texCoord = aTex;
   // Pass the normal to the fragment shader (in object space)
   Normal = aNormal;
}