#version 330 core
// Vertex attributes: position, color, texture coordinates, and normal
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec3 aColor;
layout (location = 3) in vec2 aTex;
layout (location = 4) in vec3 aTangent;

// Outputs to the fragment shader: color, texture coordinates, normal, and current position
out vec3 crntPos;
//out vec3 Normal;
out vec3 color;
out vec2 texCoord;
out mat3 TBN;

uniform mat4 projection;
uniform mat4 view;
uniform mat4 model;
uniform mat4 lightSpaceMatrix;

void main()
{
    crntPos = vec3(model * vec4(aPos, 1.0));
    texCoord = aTex;
    gl_Position = projection * view * model * vec4(aPos, 1.0);
    
    vec3 N = normalize(transpose(inverse(mat3(model))) * aNormal);
    vec3 T = normalize(mat3(model) * aTangent);

    // T and N should be perfectly perpendicular, but in practice after interpolating the attributes
    // between the vertex and applying de model matrix, they might slightly lose this perpendicularity 
    // The Gram-Schmidt orthogonalization solves this by measuring how much of T is "leaking" towards N.
    // It then subtracts it from T so he final value to be normalized is perfectly perpendicular to N.

    T = normalize(T - dot(T, N) * N);

    vec3 B = cross(N, T); //Cross product to extract Bitangent from N and T, now that they are perpendicular.

    TBN = mat3(T, B, N); //Creates TBN matrix to be passed to the fragment shader.

    //Normal = transpose(inverse(mat3(model))) * aNormal;
}

