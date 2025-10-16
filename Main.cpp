#include "Model.h"
#include "Main.h"

const unsigned int width = 1024;
const unsigned int height = 1024;

bool debugFPS = false;

// Variables for periodic FPS display
double prevTime = 0.0f;
double currTime = 0.0f;
double timeDiff;

float skyboxVertices[] =
{
	//   Coordinates
	-1.0f, -1.0f,  1.0f,//        7--------6
	 1.0f, -1.0f,  1.0f,//       /|       /|
	 1.0f, -1.0f, -1.0f,//      4--------5 |
	-1.0f, -1.0f, -1.0f,//      | |      | |
	-1.0f,  1.0f,  1.0f,//      | 3------|-2
	 1.0f,  1.0f,  1.0f,//      |/       |/
	 1.0f,  1.0f, -1.0f,//      0--------1
	-1.0f,  1.0f, -1.0f
};

unsigned int skyboxIndices[] =
{
	// Right
	1, 2, 6,
	6, 5, 1,
	// Left
	0, 4, 7,
	7, 3, 0,
	// Top
	4, 5, 6,
	6, 7, 4,
	// Bottom
	0, 3, 2,
	2, 1, 0,
	// Back
	0, 1, 5,
	5, 4, 0,
	// Front
	3, 7, 6,
	6, 2, 3
};

float randf()
{
	return -1.0f + (rand() / (RAND_MAX / 2.0f));
}

int main() {
//------------------ Open GL Initialization ------------------
	glfwInit();

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	GLFWwindow* window = glfwCreateWindow(width, height, "OpenGL study Main", NULL, NULL);
	if (window == NULL) {
		std::cerr << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(window);

	if (!gladLoadGL()) {
		std::cerr << "Failed to initialize GLAD" << std::endl;
		return -1;
	}

	glViewport(0, 0, width, height);
//---------------- Shaders ----------------------------------------
	// Load shaders
	Shader shaderProgram("default.vert", "default.frag");
	Shader skyboxShader("skybox.vert", "skybox.frag");
	Shader asteroidShader("asteroid.vert", "default.frag");

	// Set up lighting
	glm::vec4 lightColor = glm::vec4(1.0f);
	glm::vec3 lightPos = glm::vec3(0.5f);
	glm::mat4 lightModel = glm::translate(glm::mat4(1.0f), lightPos);

	shaderProgram.Activate();
	glUniform4f(glGetUniformLocation(shaderProgram.ID, "lightColor"), lightColor.x, lightColor.y, lightColor.z, lightColor.w);
	glUniform3f(glGetUniformLocation(shaderProgram.ID, "lightPos"), lightPos.x, lightPos.y, lightPos.z);

	skyboxShader.Activate();
	glUniform1i(glGetUniformLocation(skyboxShader.ID, "skybox"), 0);

	asteroidShader.Activate();
	glUniform4f(glGetUniformLocation(asteroidShader.ID, "lightColor"), lightColor.x, lightColor.y, lightColor.z, lightColor.w);
	glUniform3f(glGetUniformLocation(asteroidShader.ID, "lightPos"), lightPos.x, lightPos.y, lightPos.z);

//--------------- Render state initial setup ---------------------
	// Enable depth testing, face culling
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);
	glFrontFace(GL_CCW);

	// Initialize camera
	glm::vec3 cameraStartPos = glm::vec3(-10.09498, 2.80734, 85.0286);

	Camera camera(width, height, cameraStartPos);

	// Load models
	Model jupiterModel = Main::GenerateModel("Models/jupiter/scene.gltf", true);

	//----------------------------- Skybox -----------------------------------------//
	unsigned int skyboxVAO, skyboxVBO, skyboxEBO;
	// Generate VAO, VBO and EBO and store its IDs
	glGenVertexArrays(1, &skyboxVAO);
	glGenBuffers(1, &skyboxVBO);
	glGenBuffers(1, &skyboxEBO);

	// Bind the VAO to configure it
	glBindVertexArray(skyboxVAO);

	// Bind the VBO to the array buffer target
	glBindBuffer(GL_ARRAY_BUFFER, skyboxVBO);
	// Upload vertex data to the VBO
	glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVertices), &skyboxVertices, GL_STATIC_DRAW);
	// Bind the EBO to the element array buffer target
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, skyboxEBO);
	// Upload index data to the EBO
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(skyboxIndices), &skyboxIndices, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	// All the faces of the cubemap (make sure they are in this exact order)
	std::string facesCubemap[6] =
	{
		"Models/skybox/right.png",
		"Models/skybox/left.png",
		"Models/skybox/top.png",
		"Models/skybox/bottom.png",
		"Models/skybox/front.png",
		"Models/skybox/back.png",
	};

	unsigned int cubemapTexture;
	glGenTextures(1, &cubemapTexture);
	glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTexture);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

	// Cycles through all the textures and attaches them to the cubemap object
	Texture::BindTexturesToCubeMap(facesCubemap, 6);

	// The number of asteroids to be created
	const unsigned int number = 5000;
	// Radius of circle around which asteroids orbit
	float radius = 100.0f;
	// How much ateroids deviate from the radius
	float radiusDeviation = 25.0f;

	// Holds all transformations for the asteroids
	std::vector <glm::mat4> instanceMatrix = GenerateInstanceMatrix(number, radius, radiusDeviation);

	// Create the asteroid model with instancing enabled
	Model asteroidModel = Main::GenerateModel("Models/asteroid/scene.gltf", number, instanceMatrix, true);
//------------------ Main render loop --------------------------
	while (!glfwWindowShouldClose(window)) {
//-----------------FPS Debug and input rate control---------------
		currTime = glfwGetTime();
		timeDiff = currTime - prevTime;
		
		camera.Inputs(window);
//-------------------------------------------------------------------
		// Clear buffers
		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// Update camera matrix
		camera.updateMatrix(45.f, 0.1f, 1000.f);

		float time = glfwGetTime();
		glm::mat4 rotation = glm::rotate(glm::mat4(1.0f), time * glm::radians(10.0f), glm::vec3(0.0f, 1.0f, 0.0f));

		glEnable(GL_CULL_FACE);
		glCullFace(GL_BACK);
		jupiterModel.Draw(shaderProgram, camera, glm::vec3(0.0f), glm::quat_cast(rotation), glm::vec3(1.0f));

		asteroidModel.Draw(asteroidShader, camera);

		//--------------- Rendering Skybox -----------------------//
		glDisable(GL_CULL_FACE);
		glDepthFunc(GL_LEQUAL);
		skyboxShader.Activate();

		glm::mat4 view = glm::mat4(1.0f);
		glm::mat4 projection = glm::mat4(1.0f);

		// We make the mat4 into a mat3 and then a mat4 again in order to get rid of the last row and column
		// The last row and column affect the translation of the skybox (which we don't want to affect)
		view = glm::mat4(glm::mat3(glm::lookAt(camera.Position, camera.Position + camera.Orientation, camera.Up)));
		projection = glm::perspective(glm::radians(45.0f), (float)width / height, 0.1f, 500.f);
		glUniformMatrix4fv(glGetUniformLocation(skyboxShader.ID, "view"), 1, GL_FALSE, glm::value_ptr(view));
		glUniformMatrix4fv(glGetUniformLocation(skyboxShader.ID, "projection"), 1, GL_FALSE, glm::value_ptr(projection));

		// Draws the cubemap as the last object so we can save a bit of performance by discarding all fragments
		// where an object is present (a depth of 1.0f will always fail against any object's depth value)
		glBindVertexArray(skyboxVAO);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTexture);
		glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);
		glBindVertexArray(0);

		glDepthFunc(GL_LESS);
		//---------------------------------------------------------//
		

		// Swap buffers and poll events
		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	// Cleanup
	shaderProgram.Delete();
	skyboxShader.Delete();
	asteroidShader.Delete();

	glfwDestroyWindow(window);
	glfwTerminate();
	return 0;
}

std::vector <glm::mat4> GenerateInstanceMatrix(float number, float radius, float radiusDeviation) {
	std::vector <glm::mat4> instanceMatrix;
	for (unsigned int i = 0; i < number; i++)
	{
		// Generates x and y for the function x^2 + y^2 = radius^2 which is a circle
		float x = randf();
		float finalRadius = radius + randf() * radiusDeviation;
		float y = ((rand() % 2) * 2 - 1) * sqrt(1.0f - x * x);

		// Holds transformations before multiplying them
		glm::vec3 tempTranslation;
		glm::quat tempRotation;
		glm::vec3 tempScale;

		// Makes the random distribution more even
		if (randf() > 0.5f)
		{
			// Generates a translation near a circle of radius "radius"
			tempTranslation = glm::vec3(y * finalRadius, randf(), x * finalRadius);
		}
		else
		{
			// Generates a translation near a circle of radius "radius"
			tempTranslation = glm::vec3(x * finalRadius, randf(), y * finalRadius);
		}
		//random rotations
		float angle = randf() * glm::pi<float>();
		glm::vec3 axis = glm::normalize(glm::vec3(randf(), randf(), randf()));
		tempRotation = glm::angleAxis(angle, axis);

		//random scales
		tempScale = 0.1f * glm::vec3(0.5f + randf() * 0.5f);

		// Initialize matrices
		glm::mat4 trans = glm::mat4(1.0f);
		glm::mat4 rot = glm::mat4(1.0f);
		glm::mat4 sca = glm::mat4(1.0f);

		// Transform the matrices to their correct form
		trans = glm::translate(trans, tempTranslation);
		rot = glm::mat4_cast(tempRotation);
		sca = glm::scale(sca, tempScale);

		// Push matrix transformation
		instanceMatrix.push_back(trans * rot * sca);
	}
	return instanceMatrix;
}

Model Main::GenerateModel(const char* path, unsigned int count, std::vector<glm::mat4> instanceMatrix, bool flipUVY)
{
	Model model(path, count, instanceMatrix, flipUVY);
	return model;
}