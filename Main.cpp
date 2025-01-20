#include<iostream>
#include<glad/glad.h>
#include<GLFW/glfw3.h>
#include<stb/stb_image.h>

#include "Shader.h"
#include "VAO.h"
#include "EBO.h"
#include "Texture.h"

//Vertices coordinates for a triforce
//GLfloat vertices[] = 
//{
//	//            Coordinates            /        Colors            //
//   -0.5f, -0.5f * float(sqrt(3)) / 3,     0.2f, 0.2f, .5f,  2.f, //Lower left corner
//	0.5f, -0.5f * float(sqrt(3)) / 3,     0.2f, 0.2f, .5f,  2.f, //Lower right corner
//	0.0f,  0.5f * float(sqrt(3)) * 2 / 3, 0.2f, 0.2f, .8f,  2.f, //Upper right corner
//   -0.25f, 0.5f * float(sqrt(3)) / 6,     0.2f, 0.2f, .8f, 2.f, //Inner Left
//	0.25f, 0.5f * float(sqrt(3)) / 6,     0.2f, 0.2f, .8f, 2.f, //Inner Right
//	0.0f, -0.5f * float(sqrt(3)) / 3,     0.2f, 0.2f, .5f,  2.f //Inner down
//};

//Vertices for a square
GLfloat vertices[] = {
	//Coordinates           //Colors        /TexCoord
   -0.5f, -0.5f,0.0f,      1.0f,0.0f,0.0f,  0.0f, 0.0f,   //Lower left corner
   -0.5f,  0.5f,0.0f,      0.0f,1.0f,0.0f,  0.0f, 1.0f,  //Upper left corner
	0.5f,  0.5f,0.0f,      0.0f,0.0f,1.0f,  1.0f, 1.0f,  //Upper right corner
	0.5f, -0.5f,0.0f,      1.0f,1.0f,1.0f,  1.0f, 0.0f  //Lower left corner
};

//Indices for vertices order in a triforce
//GLuint indices[] = {
//	0,3,5, //Lower left triangles
//	3,2,4, //Lower right triangle
//	5,4,1 //Upper triangle
//};

GLuint indices[] = {
	0,2,1, //Upper triangle
	0,3,2, //Lower triangle
};


int main() {
	//Initialize GLFW
	glfwInit();

	//Tell GLFW what version of OpenGL we are using
	//In this case 3.3 (thats why Major and Minor are 3)
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	//Tell GLFW we are using the CORE profile
	//So that means we only have the modern functions
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	//Create a GLFWwindow object with given dimensions, the name and wether it should be fullscreen.
	GLFWwindow* window = glfwCreateWindow(800, 800, "FreeOpenGL", NULL, NULL);

	//Error check if windows failed to be created
	if (window == NULL)
	{
		std::cout << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return  -1;
	}
	//Introduce window to the current context
	glfwMakeContextCurrent(window);

	//Load GLAD so it configures OpenGL
	gladLoadGL();

	//Specify the viewport of OpenGL in the window
	//In this case going from x and y = 0 to x and y = 800 (top left for bottom right)
	glViewport(0,0,800,800);

	//Create shader program object and get its reference
	Shader shaderProgram("default.vert", "default.frag");

	VAO VAO1;
	VAO1.Bind();

	//Generating Vertex Buffer Object and linking it to the vertices
	VBO VBO1(vertices, sizeof(vertices));
	//Generates the Element Buffer Object and links it to the indices
	EBO EBO1(indices, sizeof(indices));
	
	//Links VBO to VAO
	VAO1.LinkAttrib(VBO1, 0, 3, GL_FLOAT, 8 * sizeof(float), (void*)0);
	VAO1.LinkAttrib(VBO1, 1, 3, GL_FLOAT, 8 * sizeof(float), (void*)(3*sizeof(float)));
	VAO1.LinkAttrib(VBO1, 2, 2, GL_FLOAT, 8 * sizeof(float), (void*)(6 * sizeof(float)));

	//Unbind everything
	VAO1.Unbind();
	VBO1.Unbind();
	EBO1.Unbind();

	GLuint uniID = glGetUniformLocation(shaderProgram.ID, "scale");

	//Texture
	Texture waterFall("res.png", GL_TEXTURE_2D, GL_TEXTURE0, GL_RGBA, GL_UNSIGNED_BYTE);
	waterFall.texUnit(shaderProgram, "tex0", 0);

	//Main while loop
	while (!glfwWindowShouldClose(window)) {
		
		// Specify the color of the background
		glClearColor(0.07f, 0.13f, 0.17f, 1.0f);
		// Clean the back buffer and assign the new color to it
		glClear(GL_COLOR_BUFFER_BIT);
		// Tell OpenGL which Shader Program we want to use
		shaderProgram.Activate();
		glUniform1f(uniID, .5f);
		//Binding the texture
		waterFall.Bind();
		// Bind the VAO so OpenGL knows to use it
		VAO1.Bind();
		// Draw the triangle using the GL_TRIANGLES primitive
		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
		// Swap the back buffer with the front buffer
		glfwSwapBuffers(window);
		//Take care of all GLFW events
		glfwPollEvents();
	}

	// Delete all the objects we've created
	VAO1.Delete();
	VBO1.Delete();
	EBO1.Delete();
	waterFall.Delete();
	shaderProgram.Delete(); 

	//Delete GLFWwindow object after closing it
	glfwDestroyWindow(window);
	//Terminates GLFW before ending the program
	glfwTerminate();
	return 0;
}