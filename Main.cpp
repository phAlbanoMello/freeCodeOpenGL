#include "Model.h"

const unsigned int width = 1024;
const unsigned int height = 1024;

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
	GLFWwindow* window = glfwCreateWindow(width, height, "FreeOpenGL", NULL, NULL);

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
	glViewport(0,0,width,height);

	//Create shader program object and get its reference
	Shader shaderProgram("default.vert", "default.frag");

	//----------------------------------------------------------- Setting up Shaders -------------------------------------------------------
	glm::vec4 lightColor = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
	glm::vec3 lightPos = glm::vec3(0.5f, 0.5f, 0.5f);
	glm::mat4 lightModel = glm::mat4(1.0f);
	lightModel = glm::translate(lightModel, lightPos);
	
	shaderProgram.Activate();
	
	glUniform4f(glGetUniformLocation(shaderProgram.ID, "lightColor"), lightColor.x, lightColor.y, lightColor.z, lightColor.w);
	glUniform3f(glGetUniformLocation(shaderProgram.ID, "lightPos"), lightPos.x, lightPos.y, lightPos.z);
	//----------------------------------------------------------------------------------------------------------------------------------------
	
	//Enables depth buffer
	glEnable(GL_DEPTH_TEST);
	
	// Enables Cull Facing
	glEnable(GL_CULL_FACE);
	// Keeps front faces
	glCullFace(GL_FRONT);
	// Uses clock-wise standard
	glFrontFace(GL_CW);

	Camera camera(width, height, glm::vec3(0.f, 0.f, 2.f));

	//Load Model------------------------------------------------------------------
	std::string statueModelPath = "Models/statue/scene.gltf";
	Model statueModel(statueModelPath.c_str(), true);
	//----------------------------------------------------------------------------
	
	// Variables to create periodic event for FPS displaying
	double prevTime = 0.0f;
	double currTime = 0.0f;
	double timeDiff;
	// Keeps track of the amount of frames in timeDiff
	unsigned int counter = 0;

	//Main while loop
	while (!glfwWindowShouldClose(window)) {

		currTime = glfwGetTime();
		timeDiff = currTime - prevTime;
		counter++;

		if (timeDiff >= 1.0 / 30.0)
		{
			// Creates new title
			std::string FPS = std::to_string((1.0 / timeDiff) * counter);
			std::string TimeBetweenFramesInMS = std::to_string((timeDiff / counter) * 1000);
			std::string newTitle = "OpenGLStudy - Face Culling and FPS - " + FPS + "FPS / " + TimeBetweenFramesInMS + "ms";
			glfwSetWindowTitle(window, newTitle.c_str());

			// Resets times and counter
			prevTime = currTime;
			counter = 0;

			//Moved here so that inputs are processed in sync with time so that inputs response do not vary with FPS with VSync disabled.
			camera.Inputs(window); 
		}

		// Specify the color of the background
		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

		// Clean the back buffer and assign the new color to it
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		camera.updateMatrix(45.f, 0.1f, 100.f);

		statueModel.Draw(shaderProgram, camera);

		// Swap the back buffer with the front buffer
		glfwSwapBuffers(window);
		//Take care of all GLFW events
		glfwPollEvents();
	}
	// Delete all the objects we've created
	shaderProgram.Delete(); 
	//Delete GLFWwindow object after closing it
	glfwDestroyWindow(window);
	//Terminates GLFW before ending the program
	glfwTerminate();
	return 0;
}