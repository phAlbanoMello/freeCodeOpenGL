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

uniform float u_time;

void main(){
    float windAmplitude = 0.2;
    float windFrequency = 1.5;
    float windSpeed = 2.0;

    float baseHeight = -0.7;
    float maxHeight = 1.0;

    for(int i = 0; i < 3; i++){
        vec4 originalPos = gl_in[i].gl_Position;

        float influence = clamp((originalPos.y - baseHeight) / (maxHeight - baseHeight), 0.0, 1.0);
        float sway = sin(originalPos.x * windFrequency + u_time * windSpeed) * windAmplitude * influence;

        vec4 displacedPos = originalPos;
        displacedPos.x += sway;

        gl_Position = data_in[i].projection * displacedPos;

        Normal = data_in[i].Normal;
        color = data_in[i].color;
        texCoord = data_in[i].texCoord;
        crntPos = vec3(displacedPos);

        EmitVertex();
    }

    EndPrimitive();
}