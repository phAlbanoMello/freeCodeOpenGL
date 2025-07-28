#version 330 core

// Final output color of the fragment
out vec4 FragColor;

// Inputs from the vertex shader
in vec3 crntPos;
//Imports the normal from the Vertex Shader
in vec3 Normal;
//Imports the color from the Vertex Shader
in vec3 color;
//Imports the texture coordinates from the Vertex Shader
in vec2 texCoord;

// Texture samplers
uniform sampler2D diffuse0;
uniform sampler2D specular0;

// Lighting and camera uniforms
uniform vec4 lightColor;
//Gets the position of the light from main
uniform vec3 lightPos;
//Gets the position of the Camera from main
uniform vec3 camPos;

// Ambient light intensity
const float ambient = 0.5;

vec4 pointLight()
{
	vec3 lightVec = lightPos - crntPos;
	
	float dist = length(lightVec);
	float a = 3.f;
	float b = 0.7f;
	float intensity = 1.f / (a * dist * dist + b * dist + 1.f);

	vec3 normal = normalize(Normal);
	vec3 lightDirection = normalize(lightVec);
	float diffuse = max(dot(normal, lightDirection), 0.f) * 0.9f; //Trying to soften diffuse
	
	float specularLight = 0.2f;
	vec3 viewDirection = normalize(camPos - crntPos);
	vec3 reflectionDirection = reflect(-lightDirection, normal);
	float specAmount = pow(max(dot(viewDirection, reflectionDirection), 0.f), 32);
	float specular = specAmount * specularLight;

    // Final color composition
    vec4 diff = texture(diffuse0, texCoord);
    float specVal = texture(specular0, texCoord).r;
    return (diff * (diffuse * intensity + ambient) + specVal * specular * intensity) * lightColor;
}

vec4 directLight(){

	vec3 normal = normalize(Normal);
	vec3 lightDirection = normalize(vec3(1.f, 1.f, 0.f));
	float diffuse = max(abs(dot(normal, lightDirection)), 0.f) * 0.9f; //Absolute helps grass shading (???)
	
	float specularLight = 0.2f;
	vec3 viewDirection = normalize(camPos - crntPos);
	vec3 reflectionDirection = reflect(-lightDirection, normal);
	float specAmount = pow(max(dot(viewDirection, reflectionDirection), 0.f), 32);
	float specular = specAmount * specularLight;

	if(texture(diffuse0, texCoord).a < 0.1)//If alpha is less then 0.1, discard it (for transparency)
		discard;

    vec4 diff = texture(diffuse0, texCoord);
    float specVal = texture(specular0, texCoord).r;
    return (diff * (diffuse + ambient) + specVal * specular) * lightColor;
}

vec4 spotLight()
{
    float outerCone = 0.8;  // Light fades out beyond this angle (cosine)
    float innerCone = 0.95; // Full brightness within this angle (cosine)

	vec3 normal = normalize(Normal);
	vec3 lightDirection = normalize(lightPos - crntPos);
	float diffuse = max(dot(normal, lightDirection), 0.f) * 0.9f;
	
	float specularLight = 0.2f;
	vec3 viewDirection = normalize(camPos - crntPos);
	vec3 reflectionDirection = reflect(-lightDirection, normal);
	float specAmount = pow(max(dot(viewDirection, reflectionDirection), 0.f), 32);
	float specular = specAmount * specularLight;

    // Spotlight intensity based on angle
    float angle = dot(vec3(0.0, -1.0, 0.0), -lightDirection);
    float intensity = clamp((angle - outerCone) / (innerCone - outerCone), 0.0, 1.0);

    vec4 diff = texture(diffuse0, texCoord);
    float specVal = texture(specular0, texCoord).r;
    return (diff * (diffuse * intensity + ambient) + specVal * specular * intensity) * lightColor;
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
	
	FragColor = directLight();
}

/* Notes on Diffuse lighting!
	This Diffuse lighting is based on the -Lambertian reflectance model-, 
	which states that the brightness of a surface depends on the angle between 
	the light source and the surface normal.
	The closer the angle is to 0� (i.e., the more directly the light hits the surface), 
	the brighter the surface appears.

	There are many lighting models that can be used to achieve a variety of effects, such as :

	
	Lambertian - Simple, cosine-based diffuse lighting - General-purpose, smooth surfaces

	Oren-Nayar - Accounts for surface roughness	- Rough surfaces (e.g., cloth)

	Disney Diffuse - Physically based, - energy conservation - PBR workflows

	Subsurface Scattering -	Simulates light scattering inside materials	- Translucent materials (e.g., skin)

	Minnaert -	Darkens at grazing angles -	Moon-like surfaces, velvet

	Burley Diffuse - Modern, physically based - PBR workflows

	Phong Diffuse -	Artistic control over falloff - Rarely used today

	Toon/Cel Shading -	Stylized, discrete lighting bands -	Cartoon-like rendering
*/