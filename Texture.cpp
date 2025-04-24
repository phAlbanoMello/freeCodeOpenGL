#include "Texture.h"

Texture::Texture(const char* image, const char* texType, GLuint slot)
{
	//Assign the type if the texture to the texture object
	type = texType;

	//Stores texture dimensions and color channels
	int widthImg, heightImg, numColCh;
	//Flips the image vertically (stbi and openGL read the image towards different directions)
	stbi_set_flip_vertically_on_load(true);
	//Reads the image from a file and stores it in bytes
	unsigned char* bytes = stbi_load(image, &widthImg, &heightImg, &numColCh, 0);

	if (!bytes) {
		std::cerr << "FAILED TO LOAD: " << image << std::endl;
	}

	//Generates the OpenGL texture object
	glGenTextures(1, &ID);
	//Assigning the texture to a texture unit
	glActiveTexture(GL_TEXTURE0 + slot);
	unit = slot;
	glBindTexture(GL_TEXTURE_2D, ID);

	//Configures the type of algorithm used to make the image smaller or bigger (solving borders)
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	//Configures how the texture repeats if the shape is bigger then the texture dimensions
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	//In case GL_CLAMP_TO_BORDER is used we have to declare a flat color and set the param
	/*GLfloat flatColor[] = { 1.0f, 1.0f, 1.0f, 1.0f }; 
	glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, flatColor);*/

	//Check type of color channels and load it accordingly
	if (numColCh == 4)
	{
		glTexImage2D(
			GL_TEXTURE_2D,
			0,
			GL_RGBA,
			widthImg,
			heightImg,
			0,
			GL_RGBA,
			GL_UNSIGNED_BYTE,
			bytes
		);
	}
	else if (numColCh == 3)
	{
		glTexImage2D(
			GL_TEXTURE_2D,
			0,
			GL_RGB,
			widthImg,
			heightImg,
			0,
			GL_RGB,
			GL_UNSIGNED_BYTE,
			bytes
		);
	}
	else if (numColCh == 1)
	{
		glTexImage2D(
			GL_TEXTURE_2D,
			0,
			GL_RED,
			widthImg,
			heightImg,
			0,
			GL_RED,
			GL_UNSIGNED_BYTE,
			bytes
		);
	}
	else {
		throw std::invalid_argument("Automatic Texture type recognition failed");
	}

	//Generate Mipmaps
	glGenerateMipmap(GL_TEXTURE_2D);

	//Deletes the image data as it is already in the OpenGL Texture object
	stbi_image_free(bytes);

	//Unbinds the OpenGL Texture object so that it can't accidentally be modified
	glBindTexture(GL_TEXTURE_2D,0);
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
	glBindTexture(GL_TEXTURE_2D, ID);
}

void Texture::Unbind()
{
	glBindTexture(GL_TEXTURE_2D, 0);
}

void Texture::Delete()
{
	glDeleteTextures(1, &ID);
}
