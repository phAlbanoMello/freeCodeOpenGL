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

out vec4 fragPosLight;

uniform mat4 projection;
uniform mat4 view;
uniform mat4 model;
uniform mat4 lightSpaceMatrix;

void main()
{
    crntPos = vec3(model * vec4(aPos, 1.0));
    Normal = transpose(inverse(mat3(model))) * aNormal;
    texCoord = aTex;
    fragPosLight = lightSpaceMatrix * vec4(crntPos, 1.0);

    gl_Position = projection * view * model * vec4(aPos, 1.0);
}

//#version 330 core
//
//// Vertex attributes: position, color, texture coordinates, and normal
//layout (location = 0) in vec3 aPos;
//layout (location = 1) in vec3 aNormal;
//layout (location = 2) in vec3 aColor;
//layout (location = 3) in vec2 aTex;
//
//// Outputs to the fragment shader: color, texture coordinates, normal, and current position
//out vec3 crntPos;
//out vec3 Normal;
//out vec3 color;
//out vec2 texCoord;
//
//// Outputs the fragment position of the light
//out vec4 fragPosLight;
//
//// Uniform matrices: camera (view + projection) and model (object transformation)
//uniform mat4 camMatrix;
//uniform mat4 model;
//// Imports the light matrix
//uniform mat4 lightSpaceMatrix;
//
//void main()
//{
//    vec4 worldPos = model * vec4(aPos, 1.0);
//    crntPos = vec3(worldPos);
//
//    Normal = transpose(inverse(mat3(model))) * aNormal;
//    color = aColor;
//    texCoord = aTex;
//
//    // Calculates the position of the light fragment for the fragment shader
//	fragPosLight = lightSpaceMatrix * vec4(crntPos, 1.0f);
//
//    gl_Position = camMatrix * worldPos;
//}

