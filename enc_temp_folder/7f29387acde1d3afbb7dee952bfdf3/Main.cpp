#include "Model.h"
#include "Main.h"

const unsigned int width = 1024;
const unsigned int height = 1024;

bool debugFPS = false;

const unsigned int MSAASamples = 1;

// Variables for periodic FPS display
double prevTime = 0.0f;
double currTime = 0.0f;
double timeDiff;
unsigned int counter = 0;

int main() {
	// Initialize GLFW
	glfwInit();

	// Set OpenGL version to 3.3 Core Profile
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_SAMPLES, MSAASamples);

	// Create window
	GLFWwindow* window = glfwCreateWindow(width, height, "OpenGL study Main", NULL, NULL);
	if (window == NULL) {
		std::cerr << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(window);

	// Load OpenGL functions using GLAD
	if (!gladLoadGL()) {
		std::cerr << "Failed to initialize GLAD" << std::endl;
		return -1;
	}

	// Set viewport from bottom-left (0,0) to top-right (width, height)
	glViewport(0, 0, width, height);

	// Load shaders
	Shader shaderProgram("default.vert", "default.frag");

	// Set up lighting
	glm::vec4 lightColor = glm::vec4(1.0f);
	glm::vec3 lightPos = glm::vec3(0.5f);
	glm::mat4 lightModel = glm::translate(glm::mat4(1.0f), lightPos);

	shaderProgram.Activate();
	glUniform4f(glGetUniformLocation(shaderProgram.ID, "lightColor"), lightColor.x, lightColor.y, lightColor.z, lightColor.w);
	glUniform3f(glGetUniformLocation(shaderProgram.ID, "lightPos"), lightPos.x, lightPos.y, lightPos.z);

	// Enable depth testing, face culling, and stencil buffer
	glEnable(GL_DEPTH_TEST);

	glEnable(GL_MULTISAMPLE);

	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);
	glFrontFace(GL_CCW);

	// Initialize camera
	Camera camera(width, height, glm::vec3(-6.42748, 11.1339, 32.1955));

	// Load models
	Model model = Main::GenerateModel("Models/crow/scene.gltf", true);

	// Main render loop
	while (!glfwWindowShouldClose(window)) {
		currTime = glfwGetTime();
		timeDiff = currTime - prevTime;
		counter++;

		if (timeDiff >= 1.0 / 30.0) {
			if (debugFPS) {
				std::string FPS = std::to_string((1.0 / timeDiff) * counter);
				std::string TimeBetweenFramesInMS = std::to_string((timeDiff / counter) * 1000);
				std::string newTitle = "OpenGLStudy - Face Culling and FPS - " + FPS + "FPS / " + TimeBetweenFramesInMS + "ms";
				glfwSetWindowTitle(window, newTitle.c_str());
			}
			prevTime = currTime;
			counter = 0;
			camera.Inputs(window);
		}

		// Clear buffers
		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

		// Update camera matrix
		camera.updateMatrix(45.f, 0.1f, 100.f);

		model.Draw(shaderProgram, camera);

		// Swap buffers and poll events
		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	// Cleanup
	shaderProgram.Delete();

	glfwDestroyWindow(window);
	glfwTerminate();
	return 0;
}

Model Main::GenerateModel(std::string path, bool flipUVY)
{
	std::string modelPath = path;
	Model model(modelPath.c_str(), flipUVY);
	return model;
}