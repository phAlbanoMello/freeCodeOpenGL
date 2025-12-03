#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "Model.h"
#include "Main.h"

const unsigned int width = 1024;
const unsigned int height = 1024;

float deltaTime;
float lastFrame;

std::shared_ptr<Model> mStatueModel;
std::shared_ptr<Model> mGroundModel;
std::shared_ptr<Model> mLightModel;
std::shared_ptr<Model> mCrowModel;

std::vector<Object> objects;
static char selectedObjectName[64] = "";

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

	// Setup Dear ImGui context
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO();
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

	// Setup Platform/Renderer backends
	ImGui_ImplGlfw_InitForOpenGL(window, true);          // Second param install_callback=true will install GLFW callbacks and chain to existing ones.
	ImGui_ImplOpenGL3_Init();

	// Set viewport from bottom-left (0,0) to top-right (width, height)
	glViewport(0, 0, width, height);

	// Load shaders
	Shader mainShader("default.vert", "default.frag");
	Shader lightShader("light.vert", "light.frag");
	Shader shadowMapShader("shadowMap.vert", "shadowMap.frag");

	// Enable depth testing, face culling, and stencil buffer
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);
	glFrontFace(GL_CCW);

	CameraData camData = Main::LoadCameraState();
	// Initialize camera
	Camera camera(width, height, camData.position, camData.orientation);
	
	// Load models
	mStatueModel = std::make_shared<Model>(Main::GenerateModel("Models/statue/scene.gltf", true));
	mGroundModel = std::make_shared<Model>(Main::GenerateModel("Models/ground/scene.gltf", true));
	mLightModel = std::make_shared<Model>(Main::GenerateModel("Models/sphere/scene.gltf", true));
	mCrowModel = std::make_shared<Model>(Main::GenerateModel("Models/crow/scene.gltf", true));
	//Depth framebuffer
	unsigned int depthMapFBO;
	glGenFramebuffers(1, &depthMapFBO);

	//Depth buffer texture for framebuffer
	const unsigned int SHADOW_WIDTH = 2048 * 4, SHADOW_HEIGHT = 2048 * 4;
	unsigned int depthMap;
	glGenTextures(1, &depthMap);
	glBindTexture(GL_TEXTURE_2D, depthMap);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT,
		SHADOW_WIDTH, SHADOW_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
	float borderColor[] = { 1.0, 1.0, 1.0, 1.0 };
	glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);

	//Attach as framebuffer's depth buffer
	glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
	
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthMap, 0);
	glDrawBuffer(GL_NONE);
	glReadBuffer(GL_NONE);
	
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	//Associate the uniform to the texture slots
	mainShader.Activate();
	glUniform1i(glGetUniformLocation(mainShader.ID, "diffuse0"), 0);
	glUniform1i(glGetUniformLocation(mainShader.ID, "shadowMap"), 1);

	glm::vec3 lightPos(0.0f, 5.0f, 5.0f);

	// Main render loop
	while (!glfwWindowShouldClose(window)) {
		glfwPollEvents();

		//ImGui frame
		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();

		DrawObjectEditor();

		float currentFrame = glfwGetTime();
		deltaTime = currentFrame - lastFrame;
		lastFrame = currentFrame;

		if (!(io.WantCaptureMouse || io.WantCaptureKeyboard)) {
			camera.Inputs(window, deltaTime);
		}
		

		glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
		// 1. render depth of scene to texture (from light's perspective)
		// --------------------------------------------------------------
		glm::mat4 lightProjection, lightView;
		glm::mat4 lightSpaceMatrix;

		float near_plane = 0.1f, far_plane = 50.0f;
		
		lightProjection = glm::ortho(-10.0f, 10.0f, -10.0f, 10.0f, near_plane, far_plane);
		lightView = glm::lookAt(lightPos, glm::vec3(0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
		lightSpaceMatrix = lightProjection * lightView;

		shadowMapShader.Activate();
		glUniformMatrix4fv(glGetUniformLocation(shadowMapShader.ID, "lightSpaceMatrix"),1, GL_FALSE, glm::value_ptr(lightSpaceMatrix));
	
		glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);
		glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
		glClear(GL_DEPTH_BUFFER_BIT);

		Main::DrawSceneWithShader(shadowMapShader, camera, false);

		glBindFramebuffer(GL_FRAMEBUFFER, 0);

		glViewport(0, 0, width, height);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		// 2. render scene as normal using the generated depth/shadow map  
		// --------------------------------------------------------------
		mainShader.Activate();
		camera.updateMatrix(45.f, near_plane, far_plane);
		glUniformMatrix4fv(glGetUniformLocation(mainShader.ID, "projection"), 1, GL_FALSE, glm::value_ptr(camera.GetProjectionMatrix()));
		glUniformMatrix4fv(glGetUniformLocation(mainShader.ID, "view"), 1, GL_FALSE, glm::value_ptr(camera.GetViewMatrix()));

		glUniform3f(glGetUniformLocation(mainShader.ID, "viewPos"), camera.Position.x, camera.Position.y, camera.Position.z);
		glUniform3f(glGetUniformLocation(mainShader.ID, "lightPos"), lightPos.x, lightPos.y, lightPos.z);
		glUniformMatrix4fv(glGetUniformLocation(mainShader.ID, "lightSpaceMatrix"), 1, GL_FALSE, glm::value_ptr(lightSpaceMatrix));

		//Bind the texture to the shadowMap uniform at the appropriate texture slot, associated before the loop
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, depthMap);

		Main::DrawSceneWithShader(mainShader, camera, true);
		Main::DrawLight(lightShader, camera, glm::vec4(1.0f), lightPos, true);


		bool wasMousePressedLastFrame = false;

		bool isMousePressedNow = glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS;
		if (isMousePressedNow && !wasMousePressedLastFrame) {
			double mouseX, mouseY;
			glfwGetCursorPos(window, &mouseX, &mouseY);

			if (!io.WantCaptureMouse) {
				glm::vec3 rayStart = glm::unProject(
					glm::vec3(mouseX, height - mouseY, 0.0f),
					camera.GetViewMatrix(),
					camera.GetProjectionMatrix(),
					glm::vec4(0, 0, width, height)
				);

				glm::vec3 rayEnd = glm::unProject(
					glm::vec3(mouseX, height - mouseY, 1.0f),
					camera.GetViewMatrix(),
					camera.GetProjectionMatrix(),
					glm::vec4(0, 0, width, height)
				);

				if (glm::any(glm::isnan(rayStart)) || glm::any(glm::isnan(rayEnd))) {
					// Handle error - unProject failed
					return 0;
				}

				glm::vec3 rayDir = glm::normalize(rayEnd - rayStart);

				float closestHit = std::numeric_limits<float>::max();
				Object* selected = nullptr;

				for (auto& obj : objects) {
					float tHit;
					if (Main::RayIntersectsAABB(rayStart, rayDir, obj, tHit)) {
						if (tHit > 0.0f && tHit < closestHit) {
							closestHit = tHit;
							selected = &obj;
						}
					}
				}

				if (selected) {
					strncpy_s(selectedObjectName, selected->name.c_str(), sizeof(selectedObjectName));
					selectedObjectName[sizeof(selectedObjectName) - 1] = '\0';
				}
				else {
					strncpy_s(selectedObjectName, "", sizeof(selectedObjectName));
				}
			}
		}

		wasMousePressedLastFrame = isMousePressedNow;




		//ImGui Render
		ImGui::Render();
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

		// Swap buffers and poll events
		glfwSwapBuffers(window);
	}
	Main::SaveCameraState(camera);
	// Cleanup
	mainShader.Delete();
	shadowMapShader.Delete();

	//Shutdown
	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();
	glfwDestroyWindow(window);
	glfwTerminate();
	return 0;
}

void DrawObjectEditor()
{
	ImGui::Begin("Object Editor");

	ImGui::Text("Name");
	ImGui::Separator();
	ImGui::InputText("##", selectedObjectName, IM_ARRAYSIZE(selectedObjectName));

	ImGui::Text("Transform");
	ImGui::Separator();

	static float position[3] = { 0.0f, 0.0f, 0.0f };
	static float rotation[3] = { 0.0f, 0.0f, 0.0f };
	static float scale[3] = { 1.0f, 1.0f, 1.0f };

	ImGui::DragFloat3("Position", position, 0.1f);
	ImGui::DragFloat3("Rotation", rotation, 0.1f);
	ImGui::DragFloat3("Scale", scale, 0.1f);

	ImGui::End();
}

Model Main::GenerateModel(std::string path, bool flipUVY)
{
	Model model(path.c_str(), flipUVY);
	return model;
}

void Main::DrawSceneWithShader(Shader shader, Camera camera, bool debugBounds) {
	glm::vec3 statuePos = glm::vec3(0.f, -0.2f, 0.0f);
	glm::mat4 statueMatrix = glm::translate(glm::mat4(1.0f), statuePos);
	mStatueModel->Draw(shader, camera, statueMatrix);

	if (debugBounds)
	{
		DebugModelCenterScreenSpace(camera, statueMatrix, width, height, IM_COL32(255, 255, 0, 255));
		DebugModelBoundsScreenSpace(camera, statueMatrix, mStatueModel->boundsMin, mStatueModel->boundsMax);
	}

	glm::vec3 groundPos = glm::vec3(0.f, -0.7f, 0.0f);
	glm::vec3 groundScaleFactor(0.5f, 0.1f, 0.5f);
	glm::mat4 groundMatrix = glm::translate(glm::mat4(1.0f), groundPos);
	groundMatrix = glm::scale(groundMatrix, groundScaleFactor);
	mGroundModel->Draw(shader, camera, groundMatrix);

	
	glm::vec3 crowPos = glm::vec3(2.f, -0.045f, 0.f);
	glm::vec3 crowScaleFactor(0.05f, 0.05f, 0.05f);
	glm::mat4 crowMatrix = glm::translate(glm::mat4(1.0f), crowPos);
	crowMatrix = glm::rotate(crowMatrix, glm::radians(-90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
	crowMatrix = glm::scale(crowMatrix, crowScaleFactor);
	mCrowModel->Draw(shader, camera, crowMatrix);

	if (debugBounds)
	{
		DebugModelCenterScreenSpace(camera, crowMatrix, width, height, IM_COL32(0, 0, 255, 255));
		DebugModelBoundsScreenSpace(camera, crowMatrix, mCrowModel->boundsMin, mCrowModel->boundsMax);
	}

	AddObjectToPickablesCollection(mStatueModel, statueMatrix, "Statue");
	AddObjectToPickablesCollection(mCrowModel, crowMatrix, "Crow");
}

void Main::AddObjectToPickablesCollection(const std::shared_ptr<Model>& model, const glm::mat4& modelMatrix, const std::string& name) {
	auto exists = std::find_if(objects.begin(), objects.end(),
		[&](const Object& obj) { return obj.name == name; });

	if (exists == objects.end()) {
		objects.push_back(MakeObjectFromModelMatrix(model, modelMatrix, name));
	}
}

Object Main::MakeObjectFromModelMatrix(const std::shared_ptr<Model>& model,const glm::mat4& modelMatrix,const std::string& name) {

	glm::vec3 minLocal = model->boundsMin;
	glm::vec3 maxLocal = model->boundsMax;

	std::vector<glm::vec3> localCorners = {
		{minLocal.x, minLocal.y, minLocal.z},
		{maxLocal.x, minLocal.y, minLocal.z},
		{minLocal.x, maxLocal.y, minLocal.z},
		{maxLocal.x, maxLocal.y, minLocal.z},
		{minLocal.x, minLocal.y, maxLocal.z},
		{maxLocal.x, minLocal.y, maxLocal.z},
		{minLocal.x, maxLocal.y, maxLocal.z},
		{maxLocal.x, maxLocal.y, maxLocal.z}
	};

	glm::vec3 minBounds(FLT_MAX);
	glm::vec3 maxBounds(-FLT_MAX);

	for (auto& c : localCorners) {
		glm::vec4 worldPos = modelMatrix * glm::vec4(c, 1.0f);
		minBounds = glm::min(minBounds, glm::vec3(worldPos));
		maxBounds = glm::max(maxBounds, glm::vec3(worldPos));
	}

	Object obj;
	obj.name = name;
	obj.minBounds = minBounds;
	obj.maxBounds = maxBounds;
	obj.matrix = modelMatrix;
	return obj;
}

void Main::DebugModelCenterScreenSpace(Camera camera, glm::mat4 modelMatrix, int width, int height, ImU32 color) {
	// Extract the world position from the modelMatrix (translation part)
	glm::vec3 worldPos = glm::vec3(modelMatrix[3]);

	// Project to clip space
	glm::vec4 clip = camera.GetProjectionMatrix() * camera.GetViewMatrix() * glm::vec4(worldPos, 1.0f);
	if (clip.w <= 0) return; // behind camera

	// Normalize to NDC
	glm::vec3 ndc = glm::vec3(clip) / clip.w;

	// Convert to screen space
	ImVec2 screen(
		(ndc.x * 0.5f + 0.5f) * width,
		(1.0f - (ndc.y * 0.5f + 0.5f)) * height
	);

	// Draw circle overlay
	ImDrawList* draw_list = ImGui::GetForegroundDrawList();
	draw_list->AddCircle(screen, 6.0f, color, 16, 2.0f);
}


void Main::DebugModelBoundsScreenSpace(Camera camera, glm::mat4 modelMatrix, const glm::vec3& boundsMin, const glm::vec3& boundsMax) {
	// Build 8 corners from the model-level bounds
	std::vector<glm::vec3> localCorners = {
		{boundsMin.x, boundsMin.y, boundsMin.z},
		{boundsMax.x, boundsMin.y, boundsMin.z},
		{boundsMin.x, boundsMax.y, boundsMin.z},
		{boundsMax.x, boundsMax.y, boundsMin.z},
		{boundsMin.x, boundsMin.y, boundsMax.z},
		{boundsMax.x, boundsMin.y, boundsMax.z},
		{boundsMin.x, boundsMax.y, boundsMax.z},
		{boundsMax.x, boundsMax.y, boundsMax.z}
	};

	float minX = width, minY = height;
	float maxX = 0, maxY = 0;

	glm::mat4 viewProj = camera.GetProjectionMatrix() * camera.GetViewMatrix();

	for (auto& corner : localCorners) {
		// Transform to world space
		glm::vec4 worldPos = modelMatrix * glm::vec4(corner, 1.0f);

		// Project to clip space
		glm::vec4 clip = viewProj * worldPos;
		if (clip.w <= 0) continue; // behind camera

		// Normalize to NDC
		glm::vec3 ndc = glm::vec3(clip) / clip.w;

		// Convert to screen space
		ImVec2 screen(
			(ndc.x * 0.5f + 0.5f) * width,
			(1.0f - (ndc.y * 0.5f + 0.5f)) * height
		);

		minX = std::min(minX, screen.x);
		minY = std::min(minY, screen.y);
		maxX = std::max(maxX, screen.x);
		maxY = std::max(maxY, screen.y);
	}

	// Draw debug rectangle
	ImDrawList* draw_list = ImGui::GetForegroundDrawList();
	draw_list->AddRect(ImVec2(minX, minY), ImVec2(maxX, maxY), IM_COL32(255, 0, 0, 255));
}


void Main::DrawLight(Shader shader, Camera camera, glm::vec4 color, glm::vec3 position, bool debugBounds) {
	shader.Activate();
	glUniform4f(glGetUniformLocation(shader.ID, "lightColor"), color.x, color.y, color.z, color.w);

	glm::vec3 lightScaleFactors(0.07f);
	glm::mat4 lightMatrix = glm::translate(glm::mat4(1.0f), position);
	lightMatrix = glm::scale(lightMatrix, lightScaleFactors);

	mLightModel->Draw(shader, camera, lightMatrix);

	if (debugBounds)
	{
		DebugModelCenterScreenSpace(camera, lightMatrix, width, height, IM_COL32(255, 0, 0, 255));
		DebugModelBoundsScreenSpace(camera, lightMatrix, mLightModel->boundsMin, mLightModel->boundsMax);
	}

	AddObjectToPickablesCollection(mLightModel, lightMatrix, "Light");
}

void Main::SaveCameraState(Camera& cam) {
	std::cout << "Saving Camera State" << std::endl;
	cam.LogState();

	nlohmann::json cameraStateData;
	cameraStateData["position"] = { cam.Position.x, cam.Position.y, cam.Position.z };
	cameraStateData["orientation"] = { cam.Orientation.x, cam.Orientation.y, cam.Orientation.z };

	std::ofstream file("camera_state.json");
	if (file.is_open()) {
		file << cameraStateData.dump(4);
		file.close();
	}
}

CameraData Main::LoadCameraState() {
	CameraData cam;
	std::ifstream file("camera_state.json");
	if (file.is_open()) {
		nlohmann::json j;
		file >> j;

		auto pos = j["position"];
		auto ori = j["orientation"];

		cam.position = glm::vec3(pos[0], pos[1], pos[2]);
		cam.orientation = glm::vec3(ori[0], ori[1], ori[2]);

		file.close();
	}
	else {
		cam.position = glm::vec3(-5.0f, 10.0f, 0.0f);
		cam.orientation = glm::vec3(0.66125, -0.710963, 0.239399);
	}

	return cam;
}

bool Main::RayIntersectsAABB(const glm::vec3& rayOrigin,
	const glm::vec3& rayDir,
	const Object& obj,
	float& tHit)
{
	// obj.minBounds e obj.maxBounds devem estar em world space
	float tMin = -std::numeric_limits<float>::infinity();
	float tMax = std::numeric_limits<float>::infinity();

	for (int i = 0; i < 3; i++) {
		if (fabs(rayDir[i]) < 1e-8f) {
			// Raio paralelo ao eixo
			if (rayOrigin[i] < obj.minBounds[i] || rayOrigin[i] > obj.maxBounds[i])
				return false; // fora do intervalo
		}
		else {
			float ood = 1.0f / rayDir[i];
			float t1 = (obj.minBounds[i] - rayOrigin[i]) * ood;
			float t2 = (obj.maxBounds[i] - rayOrigin[i]) * ood;

			if (t1 > t2) std::swap(t1, t2);

			tMin = std::max(tMin, t1);
			tMax = std::min(tMax, t2);

			if (tMin > tMax) return false; // sem interseção
		}
	}

	tHit = tMin;
	return true;
}


