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
        // Ondulação senoidal no eixo Y
        float waveAmplitude = 0.0025;
        float waveFrequency = 25.0;
        float waveSpeed = 8.0;

    for(int i = 0; i < 3; i++){
        vec4 originalPos = gl_in[i].gl_Position;

        float wave = sin(originalPos.x * waveFrequency + u_time * waveSpeed) * waveAmplitude;

        // Aplica a ondulação
        vec4 displacedPos = originalPos;
        displacedPos.x += wave;

        gl_Position = data_in[i].projection * displacedPos;
        
        Normal = data_in[i].Normal;
        color = data_in[i].color;
        texCoord = data_in[i].texCoord;
        crntPos = vec3(displacedPos); 

        EmitVertex();
    }
    EndPrimitive();
}