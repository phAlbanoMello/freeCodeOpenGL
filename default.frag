#version 330 core

out vec4 FragColor;

// Inputs interpolated from the geometry shader
in vec3 crntPos;
in vec3 Normal;
in vec3 color;     
in vec2 texCoord;   

uniform sampler2D diffuse0;
uniform sampler2D specular0;

// Lighting and camera uniforms
uniform vec4 lightColor;
uniform vec3 lightPos;
uniform vec3 camPos;

const float ambient = 0.5;

vec4 pointLight()
{
    vec3 lightVec = lightPos - crntPos;
    float dist = length(lightVec);

    float a = 3.0;
    float b = 0.7;
    float intensity = 1.0 / (a * dist * dist + b * dist + 1.0);

    // Diffuse lighting (Lambertian)
    vec3 normal = normalize(Normal);
    vec3 lightDir = normalize(lightVec);
    float diffuse = max(dot(normal, lightDir), 0.0) * 0.9;

    // Specular lighting (Phong)
    float specularStrength = 0.2;
    vec3 viewDir = normalize(camPos - crntPos);
    vec3 reflectDir = reflect(-lightDir, Normal);
    float specAmount = pow(max(dot(viewDir, reflectDir), 0.0), 32.0);
    float specular = specAmount * specularStrength;

    // Combine textures and lighting
    vec4 diff = texture(diffuse0, texCoord);
    float specVal = texture(specular0, texCoord).r;
    return (diff * (diffuse * intensity + ambient) + specVal * specular * intensity) * lightColor;
}

vec4 directLight()
{
    vec3 normal = normalize(Normal);
    vec3 lightDir = normalize(vec3(1.0, 1.0, 0.0));
    float diffuse = max(dot(normal, lightDir), 0.0) * 0.9;

    float specularStrength = 0.2;
    vec3 viewDir = normalize(camPos - crntPos);
    vec3 reflectDir = reflect(-lightDir, normal);
    float specAmount = pow(max(dot(viewDir, reflectDir), 0.0), 32.0);
    float specular = specAmount * specularStrength;

    vec4 diff = texture(diffuse0, texCoord);
    float specVal = texture(specular0, texCoord).r;
    return (diff * (diffuse + ambient) + specVal * specular) * lightColor;
}

vec4 spotLight()
{
    float outerCone = 0.8;  // Cos of outer cone angle
    float innerCone = 0.95; // Cos of inner cone angle

    vec3 normal = normalize(Normal);
    vec3 lightDir = normalize(lightPos - crntPos);
    float diffuse = max(dot(normal, lightDir), 0.0) * 0.9;

    float specularStrength = 0.2;
    vec3 viewDir = normalize(camPos - crntPos);
    vec3 reflectDir = reflect(-lightDir, normal);
    float specAmount = pow(max(dot(viewDir, reflectDir), 0.0), 32.0);
    float specular = specAmount * specularStrength;

    // Spotlight intensity based on angle
    float angle = dot(vec3(0.0, -1.0, 0.0), -lightDir);
    float intensity = clamp((angle - outerCone) / (innerCone - outerCone), 0.0, 1.0);

    vec4 diff = texture(diffuse0, texCoord);
    float specVal = texture(specular0, texCoord).r;
    return (diff * (diffuse * intensity + ambient) + specVal * specular * intensity) * lightColor;
}

float near = 0.1;
float far = 100.0;

float linearizeDepth(float depth)
{
    return (2.0 * near * far) / (far + near - (depth * 2.0 - 1.0) * (far - near));
}

float logisticDepth(float depth, float steepness, float offset)
{
    float zVal = linearizeDepth(depth);
    return 1.0 / (1.0 + exp(-steepness * (zVal - offset)));
}

void main()
{
    if(directLight().a < 0.1){
    discard;
    }
    FragColor = directLight();
}