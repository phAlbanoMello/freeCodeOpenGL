#version 330 core
out vec4 FragColor;

in vec3 color;
//Imports the texture coordinates from the Vertex Shader
in vec2 texCoord;
//Imports the normal from the Vertex Shader
in vec3 Normal;
//Imports the current position from vertex shader
in vec3 crntPos;

//Get the texture unit from Main.cpp
uniform sampler2D tex0;
uniform sampler2D tex1;
//Gets the color of the light from main
uniform vec4 lightColor;
//Gets the position of the light from main
uniform vec3 lightPos;
//Gets the position of the Camera from main
uniform vec3 camPos;
// Uniform for scaling the texture coordinates
uniform float texScale;

vec4 scaledTextCoord(sampler2D tex){
	return texture(tex, texCoord * texScale);
}


vec4 pointLight()
{
	float ambient = 0.15f;
	vec3 normal = normalize(Normal);
	vec3 lightDirection = normalize(lightPos - crntPos);
	float diffuse = max(dot(normal, lightDirection), 0.f);
	
	float specularLight = 0.5f;
	vec3 viewDirection = normalize(camPos - crntPos);
	vec3 reflectionDirection = reflect(-lightDirection, normal);
	float specAmount = pow(max(dot(viewDirection, reflectionDirection), 0.f), 16);
	float specular = specAmount * specularLight;

	return scaledTextCoord(tex0) * (diffuse + ambient) + scaledTextCoord(tex1).r * specular * lightColor;
}

void main()
{
  FragColor = pointLight();
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