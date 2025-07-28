#include "Model.h"

const unsigned int width = 1024;
const unsigned int height = 1024;

bool debugFPS = false;

// Variables for periodic FPS display
double prevTime = 0.0f;
double currTime = 0.0f;
double timeDiff;
unsigned int counter = 0;

const unsigned int numWindows = 100;
glm::vec3 positionsWin[numWindows];
float rotationsWin[numWindows];

unsigned int orderDraw[numWindows];
float distanceCamera[numWindows];

int compare(const void* a, const void* b) {
	double diff = distanceCamera[*(int*)b] - distanceCamera[*(int*)a];
	return (0 < diff) - (diff < 0);
} //ToDo : I need to understand this!!!!

int main() {
	// Initialize GLFW
	glfwInit();

	// Set OpenGL version to 3.3 Core Profile
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

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
	Shader grassProgram("default.vert", "grass.frag");
	Shader winProgram("default.vert", "windows.frag");

	// Set up lighting
	glm::vec4 lightColor = glm::vec4(1.0f);
	glm::vec3 lightPos = glm::vec3(0.5f);
	glm::mat4 lightModel = glm::translate(glm::mat4(1.0f), lightPos);

	shaderProgram.Activate();
	glUniform4f(glGetUniformLocation(shaderProgram.ID, "lightColor"), lightColor.x, lightColor.y, lightColor.z, lightColor.w);
	glUniform3f(glGetUniformLocation(shaderProgram.ID, "lightPos"), lightPos.x, lightPos.y, lightPos.z);
	grassProgram.Activate();
	glUniform4f(glGetUniformLocation(grassProgram.ID, "lightColor"), lightColor.x, lightColor.y, lightColor.z, lightColor.w);
	glUniform3f(glGetUniformLocation(grassProgram.ID, "lightPos"), lightPos.x, lightPos.y, lightPos.z);

	// Enables the Depth Buffer
	glEnable(GL_DEPTH_TEST);
	// Enables Cull Facing
	glEnable(GL_CULL_FACE);
	// Keeps front faces
	glCullFace(GL_FRONT);
	// Uses counter clock-wise standard
	glFrontFace(GL_CW);
	// Configures the blending function
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	// Initialize camera
	Camera camera(width, height, glm::vec3(0.f, 0.f, 2.f));

	// Load models
	std::string groundModelPath = "Models/t_ground/scene.gltf";
	Model groundModel(groundModelPath.c_str(), true);

	std::string grassModelPath = "Models/grass/scene.gltf";
	Model grassModel(grassModelPath.c_str(), true);

	std::string windowsModelPath = "Models/windows/scene.gltf";
	Model windowsModels(windowsModelPath.c_str(), true);

	//Generating windows
	for (unsigned int i = 0; i < numWindows; i++)
	{
		positionsWin[i] = glm::vec3(
			-15.0f + static_cast <float>(rand()) / (static_cast <float>(RAND_MAX / (15.0f - (-15.0f)))),
			1.0f + static_cast <float>(rand()) / (static_cast <float>(RAND_MAX / (4.0f - 1.0f))),
			-15.0f + static_cast <float>(rand()) / (static_cast <float>(RAND_MAX / (15.0f - (-15.0f))))
		);

		rotationsWin[i] = static_cast <float>(rand()) / (static_cast <float>(RAND_MAX / 1.0f));
		orderDraw[i] = i;
	}

	// Main render loop
	while (!glfwWindowShouldClose(window)) {
		currTime = glfwGetTime();
		timeDiff = currTime - prevTime;
		counter++;
		float deltaTime = (timeDiff / counter) * 1000;

		if (timeDiff >= 1.0 / 30.0) {
			if (debugFPS) {
				std::string FPS = std::to_string((1.0 / timeDiff) * counter);
				std::string TimeBetweenFramesInMS = std::to_string(deltaTime);
				std::string newTitle = "OpenGLStudy - Face Culling and FPS - " + FPS + "FPS / " + TimeBetweenFramesInMS + "ms";
				glfwSetWindowTitle(window, newTitle.c_str());
			}
			prevTime = currTime;
			counter = 0;
			camera.Inputs(window, deltaTime);
		}

		// Clear buffers
		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

		// Update camera matrix
		camera.updateMatrix(45.f, 0.1f, 100.f);

		groundModel.Draw(shaderProgram, camera);

		// Disable cull face so that grass and windows have both faces
		glDisable(GL_CULL_FACE);
		grassModel.Draw(grassProgram, camera);
		glEnable(GL_BLEND);

		// Get distance from each window to the camera
		for (unsigned int i = 0; i < numWindows; i++)
		{
			distanceCamera[i] = glm::length(camera.Position - positionsWin[i]);
		}

		// Sort windows by distance from camera
		qsort(orderDraw, numWindows, sizeof(unsigned int), compare);

		// Draw windows
		for (unsigned int i = 0; i < numWindows; i++)
		{
			windowsModels.Draw(winProgram, camera, positionsWin[orderDraw[i]], glm::quat(1.0f, 0.0f, rotationsWin[orderDraw[i]], 0.0f));
		}
		glDisable(GL_BLEND);
		glEnable(GL_CULL_FACE);

		// Swap buffers and poll events
		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	// Cleanup
	shaderProgram.Delete();
	grassProgram.Delete();
	winProgram.Delete();

	glfwDestroyWindow(window);
	glfwTerminate();
	return 0;
}