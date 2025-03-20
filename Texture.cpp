#include "Texture.h"

Texture::Texture(const char* image, GLenum texType, GLenum slot, GLenum format, GLenum pixelType)
{
	//Assign the type if the texture to the texture object
	type = texType;

	//Stores texture dimensions and color channels
	int widthImg, heightImg, numColCh;
	//Flips the image vertically (stbi and openGL read the image towards different directions)
	stbi_set_flip_vertically_on_load(true);
	//Reads the image from a file and stores it in bytes
	unsigned char* bytes = stbi_load(image, &widthImg, &heightImg, &numColCh, 0);

	//Generates the OpenGL texture object
	glGenTextures(1, &ID);
	//Assigning the texture to a texture unit
	glActiveTexture(GL_TEXTURE0 + slot);
	unit = slot;
	glBindTexture(texType, ID);

	//Configures the type of algorithm used to make the image smaller or bigger (solving borders)
	glTexParameteri(texType, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_LINEAR);
	glTexParameteri(texType, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	//Configures how the texture repeats if the shape is bigger then the texture dimensions
	glTexParameteri(texType, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(texType, GL_TEXTURE_WRAP_T, GL_REPEAT);

	//In case GL_CLAMP_TO_BORDER is used we have to declare a flat color and set the param
	/*GLfloat flatColor[] = { 1.0f, 1.0f, 1.0f, 1.0f }; 
	glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, flatColor);*/

	//Assigns the image to the OpenGL Texture Object
	glTexImage2D(texType, 0, GL_RGBA, widthImg, heightImg, 0, format, pixelType, bytes);
	//Generate Mipmaps
	glGenerateMipmap(texType);

	//Deletes the image data as it is already in the OpenGL Texture object
	stbi_image_free(bytes);

	//Unbinds the OpenGL Texture object so that it can't accidentally be modified
	glBindTexture(texType,0);
}

void Texture::texUnit(Shader& shader, const char* uniform, GLuint unit)
{
	// Gets the location of the uniform
	GLuint texUni = glGetUniformLocation(shader.ID, uniform);
	// Shader needs to be activated before changing the value of a uniform
	shader.Activate();
	// Sets the value of the uniform
	glUniform1i(texUni, unit);
}

void Texture::Bind()
{
	glActiveTexture(GL_TEXTURE0 + unit);
	glBindTexture(type, ID);
}

void Texture::Unbind()
{
	glBindTexture(type, 0);
}

void Texture::Delete()
{
	glDeleteTextures(1, &ID);
}
