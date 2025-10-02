#include "Main.h"
const unsigned int width = 1024;
const unsigned int height = 1024;

bool debugFPS = false;

float skyboxVertices[] = {
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

unsigned int skyboxIndices[] = {
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

// Variables for periodic FPS display
double prevTime = 0.0f;
double currTime = 0.0f;
double timeDiff;
unsigned int counter = 0;

const unsigned int CUBEMAP_FACE_COUNT = 6;

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
	Shader skyboxShader("skybox.vert", "skybox.frag");

	// Set up lighting
	glm::vec4 lightColor = glm::vec4(1.0f);
	glm::vec3 lightPos = glm::vec3(0.5f);

	shaderProgram.Activate();
	glUniform4f(glGetUniformLocation(shaderProgram.ID, "lightColor"), lightColor.x, lightColor.y, lightColor.z, lightColor.w);
	glUniform3f(glGetUniformLocation(shaderProgram.ID, "lightPos"), lightPos.x, lightPos.y, lightPos.z);
	skyboxShader.Activate();
	glUniform1i(glGetUniformLocation(skyboxShader.ID, "skybox"), 0);

	// Enable depth testing, face culling, and stencil buffer
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
	
	glCullFace(GL_BACK);
	glFrontFace(GL_CCW);

	// Initialize camera
	Camera camera(width, height, glm::vec3(0.f, 0.f, 2.f));

	// Load models
	std::string airPlaneModelPath = "Models/airplane/scene.gltf";
	Model airPlaneModel = Main::GenerateModel("Models/airplane/scene.gltf", true);
	
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

	/* Quick notes to better remind myself of each structure 
		VAO (VertexArrayObject) : Like a folder that holds the settings of how to draw
		VBO (VertexBufferObject): Like a sheet of paper with the vertex data
		EBO (ElementBufferObject): Like a map that says how to connect the points/indexes
		
		so if we're drawing a cube, VBO will have the vertex data,
		EBO says to snap vertex 0 with 1 and 2 to shape a triangle,
		the VAO holds all this up for us so we don't have to set it up again.
	*/

	std::string cubemapFaces[CUBEMAP_FACE_COUNT] =
	{
		"Models/skybox/right.jpg",
		"Models/skybox/left.jpg",
		"Models/skybox/top.jpg",
		"Models/skybox/bottom.jpg",
		"Models/skybox/front.jpg",
		"Models/skybox/back.jpg"
	};

	unsigned int cubemapTexture;
	glGenTextures(1, &cubemapTexture);
	glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTexture);

	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

	//This will repeat the last pixel color after the texture's border, preventing seams.
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	
	//This will smoothly interpolate between cubemap faces, reducing even more the chances of having seams
	//specially when we're using filtered cubemaps and reflections.
	//glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);

	Texture::BindTexturesToCubeMap(cubemapFaces, CUBEMAP_FACE_COUNT);

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
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// Update camera matrix
		camera.updateMatrix(45.f, 0.1f, 100.f);

		airPlaneModel.Draw(shaderProgram, camera);
		
		glDisable(GL_CULL_FACE);

		glDepthFunc(GL_LEQUAL);

		skyboxShader.Activate();

		glm::mat4 view = glm::mat4(1.0f);
		glm::mat4 projection = glm::mat4(1.0f);
		// We make the mat4 into a mat3 and then a mat4 again in order to get rid of the last row and column
		// The last row and column affect the translation of the skybox (which we don't want to affect)
		view = glm::mat4(glm::mat3(glm::lookAt(camera.Position, camera.Position + camera.Orientation, camera.Up)));
		projection = glm::perspective(glm::radians(45.0f), (float)width / height, 0.1f, 100.0f);
		glUniformMatrix4fv(glGetUniformLocation(skyboxShader.ID, "view"), 1, GL_FALSE, glm::value_ptr(view));
		glUniformMatrix4fv(glGetUniformLocation(skyboxShader.ID, "projection"), 1, GL_FALSE, glm::value_ptr(projection));

		// Draws the cubemap as the last object so we can save a bit of performance by discarding all fragments
		// where an object is present (a depth of 1.0f will always fail against any object's depth value)
		glBindVertexArray(skyboxVAO);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTexture);
		glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);
		glBindVertexArray(0);

		// Switch back to the normal depth function
		glDepthFunc(GL_LESS);

		// Swap buffers and poll events
		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	// Cleanup
	shaderProgram.Delete();
	skyboxShader.Delete();
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
