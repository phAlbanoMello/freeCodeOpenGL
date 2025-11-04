#version 330 core

// Vertex attributes: position, color, texture coordinates, and normal
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec3 aColor;
layout (location = 3) in vec2 aTex;


// Outputs to the fragment shader: color, texture coordinates, normal, and current position
out vec3 crntPos;
out vec3 Normal;
out vec3 color;
out vec2 texCoord;

// Uniform matrices: camera (view + projection) and model (object transformation)
uniform mat4 camMatrix;
uniform mat4 model;

void main()
{
    vec4 worldPos = model * vec4(aPos, 1.0);
    crntPos = vec3(worldPos);

    Normal = aNormal;
    color = aColor;
    texCoord = aTex;

    gl_Position = camMatrix * worldPos;
}

