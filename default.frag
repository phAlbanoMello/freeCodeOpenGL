#version 330 core

// Final output color of the fragment
out vec4 FragColor;

// Inputs from the vertex shader
in vec3 crntPos;
in vec3 Normal;
in vec3 color;
in vec2 texCoord;

// Texture samplers
uniform sampler2D diffuse0;
uniform sampler2D specular0;

// Lighting and camera uniforms
uniform vec4 lightColor;
uniform vec3 lightPos;
uniform vec3 camPos;

// Ambient light intensity
const float ambient = 0.5;

vec4 pointLight()
{
    vec3 lightVec = lightPos - crntPos;
    float dist = length(lightVec);

    // Attenuation
    float a = 3.0;
    float b = 0.7;
    float intensity = 1.0 / (a * dist * dist + b * dist + 1.0);

    // Diffuse lighting
    vec3 normal = normalize(Normal);
    vec3 lightDir = normalize(lightVec);
    float diffuse = max(dot(normal, lightDir), 0.0) * 0.9;

    // Specular lighting
    float specularStrength = 0.2;
    vec3 viewDir = normalize(camPos - crntPos);
    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32.0) * specularStrength;

    // Final color composition
    vec4 diff = texture(diffuse0, texCoord);
    float specVal = texture(specular0, texCoord).r;
    return (diff * (diffuse * intensity + ambient) + specVal * spec * intensity) * lightColor;
}

vec4 directLight()
{
    vec3 normal = normalize(Normal);
    vec3 lightDir = normalize(vec3(1.0, 1.0, 0.0));
    float diffuse = max(dot(normal, lightDir), 0.0) * 0.9;

    float specularStrength = 0.2;
    vec3 viewDir = normalize(camPos - crntPos);
    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32.0) * specularStrength;

    vec4 diff = texture(diffuse0, texCoord);
    float specVal = texture(specular0, texCoord).r;
    return (diff * (diffuse + ambient) + specVal * spec) * lightColor;
}

vec4 spotLight()
{
    float outerCone = 0.8;  // Light fades out beyond this angle (cosine)
    float innerCone = 0.95; // Full brightness within this angle (cosine)

    vec3 normal = normalize(Normal);
    vec3 lightDir = normalize(lightPos - crntPos);
    float diffuse = max(dot(normal, lightDir), 0.0) * 0.9;

    float specularStrength = 0.2;
    vec3 viewDir = normalize(camPos - crntPos);
    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32.0) * specularStrength;

    // Spotlight intensity based on angle
    float angle = dot(vec3(0.0, -1.0, 0.0), -lightDir);
    float intensity = clamp((angle - outerCone) / (innerCone - outerCone), 0.0, 1.0);

    vec4 diff = texture(diffuse0, texCoord);
    float specVal = texture(specular0, texCoord).r;
    return (diff * (diffuse * intensity + ambient) + specVal * spec * intensity) * lightColor;
}

//Depth functions
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
    // float depth = logisticDepth(gl_FragCoord.z, 0.2, 5.0);
    FragColor = directLight(); // Try pointLight() or spotLight() for different effects
    // FragColor = directLight() * (1.0 - depth) + vec4(depth * vec3(0.85, 0.85, 0.90), 1.0);
}