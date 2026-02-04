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
uniform sampler2D shadowMap;
uniform sampler2D normalMap;
uniform samplerCube depthMap;

// Lighting and camera uniforms
uniform vec4 lightColor;
//Gets the position of the light from main
uniform vec3 lightPos;
uniform vec3 lightForward;
//Gets the position of the Camera from main
uniform vec3 camPos;
//Used by the cubemap shadowmap
uniform float far_plane;

uniform float linear;
uniform float quadratic;
uniform float textureTilling;
uniform int isGround;

// Ambient light intensity
uniform float ambient;

float cubeMapShadow(vec3 fragPos){
	vec3 fragToLight = fragPos - lightPos;
	float currentDepth = length(fragToLight);

	float closestDepth = texture(depthMap, normalize(fragToLight)).r;
	closestDepth *= far_plane;

	float bias = max(0.05 * (1.0 - dot(normalize(Normal), normalize(lightPos - fragPos))), 0.001);

	float shadow = currentDepth - bias > closestDepth ? 1.0 : 0.0;

	return shadow;
}

float pcfShadows(vec4 fragPosLightSpace){
	vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w; 
	projCoords = projCoords * 0.5 + 0.5;

	if (projCoords.z > 1.0)return 0.0;

    float bias = max(0.003 * (1.0 - dot(normalize(Normal), normalize(lightPos - crntPos))), 0.001);

    float shadow = 0.0;

	//convert pixel offsets into normalized texture coordinate offsets, so we can sample neighboring texels correctly
    vec2 texelSize = 1.0 / textureSize(shadowMap, 0);

    for(int x = -1; x <= 1; ++x)
    {
        for(int y = -1; y <= 1; ++y)
        {
			//sampling neighboor texels by adding offsets that alternate between -1 and 1, and multiplying by texel normalized coordinate offset
            float pcfDepth = texture(shadowMap, projCoords.xy + vec2(x, y) * texelSize).r;
            shadow += projCoords.z - bias > pcfDepth ? 1.0 : 0.0;
        }
    }

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


vec4 pointLight()
{
	vec3 lightVec = lightPos - crntPos;
	
	float dist = length(lightVec);
	float a = 3.f;
	float b = 0.7f;
	float intensity = 1.f / (linear * dist * dist + quadratic * dist + 1.f);

	vec3 normal = normalize(Normal);
	vec3 lightDirection = normalize(lightVec);
	float diffuseLight = max(dot(normal, lightDirection), 0.f) * 0.9f; //Trying to soften diffuse
	float specular = 0.0f;
	//Blinn-Phong lighting
	if(diffuseLight != 0.f){
		float specularLight = 0.2f;
		vec3 viewDirection = normalize(camPos - crntPos);
		vec3 halfWayVec = normalize(viewDirection + lightDirection);
		float specAmount = pow(max(dot(normal, halfWayVec), 0.f), 32);
		specular = specAmount * specularLight;
	}

	float shadow = cubeMapShadow(crntPos);
    // Final color composition
    vec4 diffuseColor = texture(diffuse0, texCoord);
	float specVal = texture(specular0, texCoord).r;

	vec3 ambientComposition = ambient * diffuseColor.rgb;
	vec3 diffuseComposition = diffuseColor.rgb * diffuseLight;
	vec3 specularComposition = specVal * specular * lightColor.rgb;

	vec3 lightComposition = diffuseComposition + specularComposition;

	vec3 finalComposition = ambientComposition + lightComposition * intensity * (1.0 - shadow);

	return vec4(finalComposition, 1.);
}

vec4 spotLight()
{
    float outerCone = cos(radians(25.0));  // Light fades out beyond this angle (cosine)
    float innerCone = cos(radians(15.0)); // Full brightness within this angle (cosine)

	vec2 finalTexCoords = texCoord;

	if(isGround == 1){
		finalTexCoords = texCoord * textureTilling;
	}
	
	//vec3 normal = normalize(Normal);
	vec3 normal = texture(normalMap, finalTexCoords).rgb;
	normal = normalize(normal * 2.0 - 1.0);

	vec3 lightDirection = normalize(lightPos - crntPos);
	float diffuseLight = max(dot(normal, lightDirection), 0.0);
	
	float specularLight = 0.2f;
	vec3 viewDirection = normalize(camPos - crntPos);
	vec3 reflectionDirection = reflect(-lightDirection, normal);
	float specAmount = pow(max(dot(viewDirection, reflectionDirection), 0.f), 32.);
	float specular = specAmount * specularLight;

    // Spotlight intensity based on angle
    float angle = dot(normalize(lightForward), -lightDirection);
    float intensity = clamp((angle - outerCone) / (innerCone - outerCone), 0.0, 1.0);
	
	float shadow = cubeMapShadow(crntPos);

    vec4 diffuseColor = texture(diffuse0, finalTexCoords);
    float specVal = texture(specular0, finalTexCoords).r;

	vec3 ambientComposition = diffuseColor.rgb * ambient;
	vec3 diffuseComposition = diffuseColor.rgb * diffuseLight;
	vec3 specularComposition = specVal * specular * lightColor.rgb;

	vec3 lightComposition = diffuseComposition + specularComposition;

	vec3 finalComposition = ambientComposition + lightComposition * intensity * (1-shadow);


    return vec4(finalComposition, 1.);
}

void main()
{
	FragColor =  spotLight();
}

