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
// Imports the fragment position of the light
in vec4 fragPosLight;

// Texture samplers
uniform sampler2D diffuse0;
uniform sampler2D specular0;
uniform sampler2D shadowMap;

// Lighting and camera uniforms
uniform vec4 lightColor;
//Gets the position of the light from main
uniform vec3 lightPos;
//Gets the position of the Camera from main
uniform vec3 camPos;

// Ambient light intensity
const float ambient = 0.12;

float pcfShadows(vec4 fragPosLightSpace){
	vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w; 
	projCoords = projCoords * 0.5 + 0.5;

	if (projCoords.z > 1.0)return 0.0;

    float bias = max(0.003 * (1.0 - dot(normalize(Normal), normalize(lightPos - crntPos))), 0.001);

    float shadow = 0.0;
    vec2 texelSize = 1.0 / textureSize(shadowMap, 0);

    for(int x = -1; x <= 1; ++x)
    {
        for(int y = -1; y <= 1; ++y)
        {
            float pcfDepth = texture(shadowMap, projCoords.xy + vec2(x, y) * texelSize).r;
            shadow += projCoords.z - bias > pcfDepth ? 1.0 : 0.0;
        }
    }

	//I could also calculate the kernel size but I have to check if this is okay or if it's unnescessary heavy
	/*

	float radius = 1; //1 = 3x3, 2 = 5x5, 3 = 7x7

	for(int x = -radius; x <= radius; ++x)
    {
        for(int y = -radius; y <= radius; ++y)
        {
	............................................

	float kernelSize = float(pow(2 * radius + 1, 2));
    shadow /= kernelSize;

	*/

    shadow /= 9.0;

	return shadow;
}

float basicShadow(vec4 fragPosLightSpace)
{
	vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w; // returns the fragment's light-space position in the range [-1,1]
	projCoords = projCoords * 0.5 + 0.5;

	float closestDepth = texture(shadowMap, projCoords.xy).r;
	float currentDepth = projCoords.z;
	float bias = max(0.005 * (1.0 - dot(normalize(Normal), normalize(lightPos - crntPos))), 0.001);
	float shadow = currentDepth - bias > closestDepth ? 1.0 : 0.0;

	return shadow;
}


vec4 directLight(){

	vec3 color = texture(diffuse0, texCoord).rgb;
	vec3 normal = normalize(Normal);
	vec3 lightColor = vec3(1.0);
	vec3 ambient = ambient * lightColor;
	vec3 lightDir = normalize(lightPos - crntPos);
	float diff = max(dot(lightDir, normal), .0);
	vec3 diffuse = diff * lightColor;
	vec3 viewDir = normalize(camPos - crntPos);
	float spec = 0.0;

	vec3 halfwayDir = normalize(lightDir + viewDir);

	vec3 specular = spec * lightColor;

	float shadow = pcfShadows(fragPosLight);

	vec3 lighting = (ambient + (1.0 - shadow) * (diffuse + specular)) * color;

	float debugNormalsPB = dot(lightDir, normal);
	vec4 debugNormalsHue = vec4(normal * 0.5 + 0.5, 1.0);
	vec4 debugShadow= vec4(vec3(1.0 - shadow), 1.0);


	return vec4(lighting, 1.0);
}

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
	float specular = 0.25f;
	//Blinn-Phong lighting
	if(diffuse != 0.f){
		float specularLight = 0.2f;
		vec3 viewDirection = normalize(camPos - crntPos);
		vec3 reflectionDirection = reflect(-lightDirection, normal);
		vec3 halfWayVec = normalize(viewDirection + lightDirection);
		float specAmount = pow(max(dot(normal, halfWayVec), 0.f), 32);
		specular = specAmount * specularLight;
	}

    // Final color composition
    vec4 diff = texture(diffuse0, texCoord);
    float specVal = texture(specular0, texCoord).r;
    return (diff * (diffuse * intensity + ambient) + specVal * specular * intensity) * lightColor;
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

void main()
{
	FragColor =  directLight(); //directLight();
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