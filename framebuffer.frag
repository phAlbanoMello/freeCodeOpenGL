//-------------- Radar/Border detection -----------------------

//#version 330 core
//
//out vec4 FragColor;
//in vec2 texCoords;
//
//uniform sampler2D screenTexture;
//
//const float offset_x = 1.0f / 1024.0f;  
//const float offset_y = 1.0f / 1024.0f;  
//
//vec2 offsets[9] = vec2[]
//(
//    vec2(-offset_x,  offset_y), vec2( 0.0f,    offset_y), vec2( offset_x,  offset_y),
//    vec2(-offset_x,  0.0f),     vec2( 0.0f,    0.0f),     vec2( offset_x,  0.0f),
//    vec2(-offset_x, -offset_y), vec2( 0.0f,   -offset_y), vec2( offset_x, -offset_y) 
//);
//
//float kernel[9] = float[]
//(
//    1,  1, 1,
//    1, -8, 1,
//    1,  1, 1
//);
//
//void main()
//{
//    vec3 color = vec3(0.0f);
//    for(int i = 0; i < 9; i++)
//        color += vec3(texture(screenTexture, texCoords.st + offsets[i])) * kernel[i];
//    FragColor = vec4(color, 1.0f);
//}

//-------------- PIXELIZATION -----------------------
#version 330 core

out vec4 FragColor;
in vec2 texCoords;

uniform sampler2D screenTexture;
uniform float pixelSize; // Tamanho do "bloco" de pixel

void main()
{
    // Arredonda as coordenadas para o centro de um bloco
    vec2 pixelatedCoords = vec2(
        floor(texCoords.x / pixelSize) * pixelSize,
        floor(texCoords.y / pixelSize) * pixelSize
    );

    vec3 color = texture(screenTexture, pixelatedCoords).rgb;
    FragColor = vec4(color, 1.0);
}

//------------ GAUSSIAN BLUR ------------------
//#version 330 core
//
//out vec4 FragColor;
//in vec2 texCoords;
//
//uniform sampler2D screenTexture;
//uniform float blurSize;
//
//void main()
//{
//    vec3 result = vec3(0.0);
//    
//    //Gaussian weights
//    float weights[5] = float[](0.227027, 0.1945946, 0.1216216, 0.054054, 0.016216);
//
//    for (int i = -4; i <= 4; ++i)
//    {
//        float offset = float(i) * blurSize;
//        result += texture(screenTexture, texCoords + vec2(offset, 0.0)).rgb * weights[abs(i)];
//    }
//
//    FragColor = vec4(result, 1.0);
//}

