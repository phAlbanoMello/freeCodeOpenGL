#version 330 core

layout (triangles) in;
layout (triangle_strip, max_vertices = 3) out;

out vec3 Normal;
out vec3 color;
out vec2 texCoord;
out vec3 crntPos;

in DATA {
    vec3 Normal;
    vec3 color;
    vec2 texCoord;
    mat4 projection;
} data_in[];

void main()
{
    // Reconstruct world-space positions from gl_in (assuming they are in world space)
    vec3 pos0 = vec3(gl_in[0].gl_Position);
    vec3 pos1 = vec3(gl_in[1].gl_Position);
    vec3 pos2 = vec3(gl_in[2].gl_Position);

    // Compute face normal using cross product of two edges
    vec3 edge1 = pos1 - pos0;
    vec3 edge2 = pos2 - pos0;
    vec3 faceNormal = normalize(cross(edge1, edge2));

    // Emit all three vertices with the same face normal
    for (int i = 0; i < 3; ++i)
    {
        gl_Position = data_in[i].projection * gl_in[i].gl_Position;

        Normal = faceNormal;               // Flat shading
        color = data_in[i].color;          // Vertex color
        texCoord = data_in[i].texCoord;    // Texture coordinates
        crntPos = vec3(gl_in[i].gl_Position); // Position for lighting

        EmitVertex();
    }

    EndPrimitive();

}