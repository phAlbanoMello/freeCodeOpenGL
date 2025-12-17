#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/epsilon.hpp>

#include <glm/gtx/matrix_decompose.hpp>
#include <glm/gtx/quaternion.hpp>

#include "Model.h"
#include "Main.h"
#include "Scene.h"

const unsigned int mWidth = 1024;
const unsigned int mHeight = 1024;

float mNearPlane = 0.1f;
float mFarPlane = 100.0f;

float mDeltaTime;
float mLastFrameTime;

std::shared_ptr<Model> mStatueModel;
std::shared_ptr<Model> mGroundModel;
std::shared_ptr<Model> mLightModel;
std::shared_ptr<Model> mCrowModel;

std::shared_ptr<Shader> mMainShader;
std::shared_ptr<Shader> mLightShader;
std::shared_ptr<Shader> mShadowMapShader;
std::shared_ptr<Shader> mColoredLightsShader;

std::shared_ptr<Camera> mCamera;
std::shared_ptr<Scene> mScene;

std::vector<std::shared_ptr<Model>> mModels;

//---------- ImGUI static variables
static char selectedObjectName[64] = "\0";
static float selectedObjectPosition[3] = { 0.0f, 0.0f, 0.0f };
static float selectedObjectRotation[3] = { 0.0f, 0.0f, 0.0f };
static float selectedObjectScale[3] = { 1.0f, 1.0f, 1.0f };

int main() {
	// Initialize GLFW
	glfwInit();

	// Set OpenGL version to 3.3 Core Profile
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	// Create window
	GLFWwindow* window = glfwCreateWindow(mWidth, mHeight, "OpenGL study Main", NULL, NULL);
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

	//Setup ImGui context
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& ImGuiInput = ImGui::GetIO();
	ImGuiInput.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;// Enable Keyboard Controls

	// Setup Platform/Renderer backends
	ImGui_ImplGlfw_InitForOpenGL(window, true);// Second param install_callback=true will install GLFW callbacks and chain to existing ones.
	ImGui_ImplOpenGL3_Init();

	// Set viewport from bottom-left (0,0) to top-right (width, height)
	glViewport(0, 0, mWidth, mHeight);

	// Load shaders - Stored as shared pointers to facilitate access across functions
	mMainShader = std::make_shared<Shader>("default.vert", "default.frag");
	mShadowMapShader = std::make_shared<Shader>("shadowMap.vert", "shadowMap.frag");
	mLightShader = std::make_shared<Shader>("light.vert", "light.frag");
	mColoredLightsShader = std::make_shared<Shader>("colorLight.vert", "colorLight.frag");

	// Enable depth testing
	glEnable(GL_DEPTH_TEST);
	//Face culling
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);
	glFrontFace(GL_CCW);

	// Initialize camera
	CameraData camData = Camera::LoadCameraState();
	mCamera = std::make_shared<Camera>(mWidth, mHeight, camData.position, camData.orientation);
	
	// Load models
	mStatueModel = std::make_shared<Model>("Models/statue/scene.gltf", true, "Statue");
	mGroundModel = std::make_shared<Model>("Models/ground/scene.gltf", true, "Ground");
	mLightModel = std::make_shared<Model>("Models/sphere/scene.gltf", true, "Light");
	mCrowModel = std::make_shared<Model>("Models/crow/scene.gltf", true, "Crow");

	mModels.push_back(mStatueModel);
	mModels.push_back(mGroundModel);
	mModels.push_back(mLightModel);
	mModels.push_back(mCrowModel);

	//Adding objects to the collection (unordered map) of Object data, used as source of truth for objects transformation.
	mScene = std::make_shared<Scene>();
	mScene->AddObjectsToScene(mModels);

	//Depth framebuffer
	unsigned int depthMapFBO;
	glGenFramebuffers(1, &depthMapFBO);

	//Depth buffer texture for framebuffer
	const unsigned int SHADOW_WIDTH = mWidth * 4, SHADOW_HEIGHT = mHeight * 4;
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
	glDrawBuffer(GL_NONE); //Not using colors since we're using only for depth component
	glReadBuffer(GL_NONE);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	//Associate the uniform to the texture slots
	mMainShader->Activate();
	mMainShader->SetInt("diffuse0", 0);
	mMainShader->SetInt("shadowMap", 1);

	//Models initial transform values
	glm::vec3 lightStartPosition(0.0f, 5.0f, 5.0f);

	glm::vec3 statuePos(-1.5f, 0.9f, 0.0f);
	glm::vec3 statueRot(0.0f, 52.5f, 0.0f);
	glm::vec3 statueSca(2.0f, 2.0f, 2.0f);
	
	glm::vec3 crowPos(1.9f, 0.7f, 0.0f);
	glm::vec3 crowRot(0.0f, -45.0f, 0.0f);
	glm::vec3 crowSca(0.04f, 0.04f, 0.04f);

	glm::vec3 groundPos(0.0f, 0.3f, 0.0f);
	glm::vec3 groundSca(0.1f, 0.01f, 0.1f);

	//Updating the matrices at the object map with the initial setup.
	mScene->SetObjectMatrix(mLightModel, lightStartPosition, glm::vec3(0.0f), glm::vec3(0.2f));
	mScene->SetObjectMatrix(mStatueModel, statuePos, statueRot, statueSca);
	mScene->SetObjectMatrix(mCrowModel, crowPos, crowRot, crowSca);
	mScene->SetObjectMatrix(mGroundModel, groundPos, glm::vec3(0.0f), groundSca);

	//Calculate object bounds after changing their matrices (used for object picking)
	mScene->UpdateAllObjectsBounds();

	// Main render loop
	while (!glfwWindowShouldClose(window)) {
		glfwPollEvents();

		//ImGui frame
		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();

		DrawObjectEditor();
		
		float currentTime = glfwGetTime();
		mDeltaTime = currentTime - mLastFrameTime;
		mLastFrameTime = currentTime;

		if (!(ImGuiInput.WantCaptureMouse || ImGuiInput.WantCaptureKeyboard)) {
			mCamera->Inputs(window, mDeltaTime);
		}
		
		glClearColor(0.1f, 0.1f, 0.1f, 1.0f);

		glm::vec3 currentLightPos = mScene->GetObjectPosition(mLightModel);
		glm::mat4 lightSpaceMatrix = Main::GenerateLightSpaceMatrix(currentLightPos, mNearPlane, mFarPlane);

		ShadowMapRenderPass(lightSpaceMatrix, SHADOW_WIDTH, SHADOW_HEIGHT, depthMapFBO);

		mCamera->updateMatrix(45.f, mNearPlane, mFarPlane);
		
		mMainShader->Activate();
		Main::SetupMainShaderUniforms(mMainShader, currentLightPos, lightSpaceMatrix);

		//Bind the texture to the shadowMap uniform at the appropriate texture slot, associated before the loop
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, depthMap);

		Main::DrawSceneWithShader(mMainShader, *mCamera, false);
		Main::DrawLight(mLightShader, *mCamera, glm::vec4(1.0f), mScene->GetObjectPosition(mLightModel), false);

		HandleObjectSelection(window, ImGuiInput, *mCamera); //Sets mSelectedObject
	
		UpdateUI();//Updates data at ImGui window according to selection

		//ImGui Render
		ImGui::Render();
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

		// Swap buffers and poll events
		glfwSwapBuffers(window);
	}
	mCamera->SaveCameraState();

	// Cleanup
	mMainShader->Delete();
	mLightShader->Delete();
	mShadowMapShader->Delete();

	//Shutdown
	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();
	glfwDestroyWindow(window);
	glfwTerminate();
	return 0;
}

void Main::SetupMainShaderUniforms(std::shared_ptr<Shader>& shader, glm::vec3& currentLightPos, glm::mat4& lightSpaceMatrix)
{
	shader->SetMatrix4("projection", mCamera->GetProjectionMatrix());
	shader->SetMatrix4("view", mCamera->GetViewMatrix());

	shader->SetVec("viewPos", mCamera->Position);
	shader->SetVec("lightPos", glm::vec3(currentLightPos.x, currentLightPos.y, currentLightPos.z));

	shader->SetMatrix4("lightSpaceMatrix", lightSpaceMatrix);
}

void ShadowMapRenderPass(glm::mat4& lightSpaceMatrix, const unsigned int SHADOW_WIDTH, const unsigned int SHADOW_HEIGHT, unsigned int depthMapFBO)
{
	mShadowMapShader->Activate();
	mShadowMapShader->SetMatrix4("lightSpaceMatrix", lightSpaceMatrix);

	glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);
	glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
	glClear(GL_DEPTH_BUFFER_BIT);

	Main::DrawSceneWithShader(mShadowMapShader, *mCamera, false);

	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	glViewport(0, 0, mWidth, mHeight);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

glm::mat4 Main::GenerateLightSpaceMatrix(glm::vec3 currentLightPosition, float nearPlane, float farPlane) {
	glm::mat4 lightProjection;
	glm::mat4 lightView;
	glm::mat4 lightSpaceMatrix;

	glm::vec3 lightPos = mScene->GetObjectPosition(mLightModel);//To sync light model position with actual light matrix

	lightProjection = glm::ortho(-10.0f, 10.0f, -10.0f, 10.0f, nearPlane, farPlane);
	lightView = glm::lookAt(lightPos, glm::vec3(0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
	lightSpaceMatrix = lightProjection * lightView;

	return lightSpaceMatrix;
}

void HandleObjectSelection(GLFWwindow* window, ImGuiIO& io, const Camera& camera)
{
	static bool wasMousePressedLastFrame = false;

	bool isMousePressedNow = glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS;
	if (isMousePressedNow && !wasMousePressedLastFrame) {
		if (!io.WantCaptureMouse) {
			mScene->mSelectedObject = Main::GetSelectedObject(camera, window);
		}
	}

	wasMousePressedLastFrame = isMousePressedNow;
}

void UpdateUI() {
	Object* selectedObject = mScene->mSelectedObject;

	if (selectedObject) {
		strncpy_s(selectedObjectName, selectedObject->name.c_str(), sizeof(selectedObjectName));
		selectedObjectName[sizeof(selectedObjectName) - 1] = '\0';
		selectedObjectPosition[0] = selectedObject->position.x;
		selectedObjectPosition[1] = selectedObject->position.y;
		selectedObjectPosition[2] = selectedObject->position.z;

		selectedObjectRotation[0] = selectedObject->rotation.x;
		selectedObjectRotation[1] = selectedObject->rotation.y;
		selectedObjectRotation[2] = selectedObject->rotation.z;

		selectedObjectScale[0] = selectedObject->scale.x;
		selectedObjectScale[1] = selectedObject->scale.y;
		selectedObjectScale[2] = selectedObject->scale.z;
	}
	else {
		strncpy_s(selectedObjectName, "", sizeof(selectedObjectName));
		selectedObjectPosition[0] = 0.f;
		selectedObjectPosition[1] = 0.f;
		selectedObjectPosition[2] = 0.f;

		selectedObjectRotation[0] = 0.f;
		selectedObjectRotation[1] = 0.f;
		selectedObjectRotation[2] = 0.f;

		selectedObjectScale[0] = 1.0f;
		selectedObjectScale[1] = 1.0f;
		selectedObjectScale[2] = 1.0f;
	}
}

void UpdateSelectedObjectPosition() {
	Object* selectedObject = mScene->mSelectedObject;

	if (!selectedObject) {
		return;
	}

	glm::vec3 position(
		selectedObjectPosition[0],
		selectedObjectPosition[1],
		selectedObjectPosition[2]
	);

	glm::vec3 rotation(
		selectedObjectRotation[0],
		selectedObjectRotation[1],
		selectedObjectRotation[2]
	);

	glm::vec3 scale(
		selectedObjectScale[0],
		selectedObjectScale[1],
		selectedObjectScale[2]
	);

	glm::mat4 newMatrix = glm::translate(glm::mat4(1.0f), position);
	newMatrix = glm::rotate(newMatrix, glm::radians(rotation.x), glm::vec3(1.0f, 0.0f, 0.0f));
	newMatrix = glm::rotate(newMatrix, glm::radians(rotation.y), glm::vec3(0.0f, 1.0f, 0.0f));
	newMatrix = glm::rotate(newMatrix, glm::radians(rotation.z), glm::vec3(0.0f, 0.0f, 1.0f));
	newMatrix = glm::scale(newMatrix, scale);

	selectedObject->position = position;
	selectedObject->rotation = rotation;
	selectedObject->scale = scale;
	selectedObject->matrix = newMatrix;

	selectedObject->CalculateObjectBounds();
}

Object* Main::GetSelectedObject(const Camera& camera, GLFWwindow* window) {
	double mouseX, mouseY;
	glfwGetCursorPos(window, &mouseX, &mouseY);

	glm::mat4 view = camera.GetViewMatrix();
	glm::mat4 proj = camera.GetProjectionMatrix();

	glm::vec3 rayEnd = glm::unProject(
		glm::vec3(mouseX, mHeight - mouseY, 1.0f),
		view,
		proj,
		glm::vec4(0, 0, mWidth, mHeight)
	);

	glm::vec3 rayOrigin = camera.Position;
	glm::vec3 rayDir = glm::normalize(rayEnd - rayOrigin);

	Object* selected = nullptr;
	float closestHit = std::numeric_limits<float>::max();

	for (auto& [name, obj] : mScene->mObjects) {
		float tHit;
		if (Main::RayIntersectsAABB(rayOrigin, rayDir, obj, tHit)) {
			if (tHit > 0.0f && tHit < closestHit) {
				closestHit = tHit;
				selected = &obj;
			}
		}
	}

	return selected;
}

void Main::DrawSceneWithShader(std::shared_ptr<Shader>& shader, Camera camera, bool debugBounds) {

	for (size_t i = 0; i < mModels.size(); i++)
	{
		if (mModels[i]->name == mLightModel->name) //Don't draw light with regular shaders (maybe make this in a appropriate manner later)
		{
			continue;
		}
		glm::mat4 matrix = mScene->GetObjectMatrix(mModels[i]);

		mModels[i]->Draw(*shader, camera, matrix);

		if (debugBounds || mScene->mSelectedObject && mModels[i]->name == mScene->mSelectedObject->name)
		{
			DebugModelCenterScreenSpace(camera, matrix, IM_COL32(255, 255, 255, 255));
			DebugModelBoundsScreenSpace(camera, matrix, mModels[i]->localInitialBoundsMin, mModels[i]->localInitialBoundsMax);
		}
	}
}

void Main::DrawLight(std::shared_ptr<Shader> shader, Camera camera, glm::vec4 color, glm::vec3 position, bool debugBounds) {
	shader->Activate();
	shader->SetVec("lightColor", glm::vec4(color.x, color.y, color.z, color.w));

	glm::vec3 lightScaleFactors(0.07f);
	glm::mat4 lightMatrix = glm::translate(glm::mat4(1.0f), position);
	lightMatrix = glm::scale(lightMatrix, lightScaleFactors);

	mLightModel->Draw(*shader, camera, lightMatrix);

	if (debugBounds || mScene->mSelectedObject && mLightModel->name == mScene->mSelectedObject->name)
	{
		DebugModelCenterScreenSpace(camera, lightMatrix, IM_COL32(255, 0, 0, 255));
		DebugModelBoundsScreenSpace(camera, lightMatrix, mLightModel->localInitialBoundsMin, mLightModel->localInitialBoundsMax);
	}
}

bool Main::RayIntersectsAABB(const glm::vec3& rayOrigin, const glm::vec3& rayDir, const Object& obj, float& tHit)
{
	float tMin = -std::numeric_limits<float>::infinity();
	float tMax = std::numeric_limits<float>::infinity();

	for (int axis = 0; axis < 3; ++axis)
	{
		if (std::abs(rayDir[axis]) < FLT_EPSILON)
		{
			if (rayOrigin[axis] < obj.minBounds[axis] ||
				rayOrigin[axis] > obj.maxBounds[axis])
			{
				return false;
			}
		}
		else
		{
			float invDir = 1.0f / rayDir[axis];

			float tNear = (obj.minBounds[axis] - rayOrigin[axis]) * invDir;
			float tFar = (obj.maxBounds[axis] - rayOrigin[axis]) * invDir;

			if (tNear > tFar) std::swap(tNear, tFar);

			tMin = std::max(tMin, tNear);
			tMax = std::min(tMax, tFar);

			if (tMin > tMax) return false;
		}
	}

	tHit = tMin;
	return true;
}

void DrawObjectEditor()
{
	ImGui::Begin("Object Editor");

	if (mScene->mSelectedObject) {

		ImGui::Text("Name    ");
		ImGui::SameLine();
		ImGui::InputText("##name", selectedObjectName, IM_ARRAYSIZE(selectedObjectName));

		DrawText("Transform", 0.5f, 0.3f);

		ImGui::Text("Position");
		ImGui::SameLine();
		ImGui::DragFloat3("##pos", selectedObjectPosition, 0.1f);

		ImGui::Text("Rotation");
		ImGui::SameLine();
		ImGui::DragFloat3("##rot", selectedObjectRotation, 0.1f);

		ImGui::Text("Scale   ");
		ImGui::SameLine();
		ImGui::DragFloat3("##sca", selectedObjectScale, 0.01f, 0.01f, 10.0f);

		UpdateSelectedObjectPosition();

		mScene->mSelectedObject->CalculateObjectBounds();
	}
	else {
		ImGui::Text("No object selected");
	}

	ImGui::End();
}

void DrawText(const char* text, float xOffset, float yOffset)
{
	ImVec2 windowSize = ImGui::GetWindowSize();
	ImVec2 textSize = ImGui::CalcTextSize(text);
	float textPosX = (windowSize.x - textSize.x) * xOffset;
	float textPosY = (windowSize.y - textSize.y) * yOffset;
	ImGui::SetCursorPos(ImVec2(textPosX, textPosY));

	ImGui::Text(text);
}

void Main::DebugModelCenterScreenSpace(Camera camera, glm::mat4 modelMatrix, ImU32 color) {
	// Extract the world position from the modelMatrix (translation part)
	glm::vec3 worldPos = glm::vec3(modelMatrix[3]);

	// Project to clip space
	glm::vec4 clipSpacePos = camera.GetProjectionMatrix() * camera.GetViewMatrix() * glm::vec4(worldPos, 1.0f);
	if (clipSpacePos.w <= 0) return; // behind camera. The w component of the homogeneous clipspace coordinates represents the
	// depth scaling factor, or the perspective divisor.

	// Normalize to NDC
	glm::vec3 normalizedDeviceCoordinates = glm::vec3(clipSpacePos) / clipSpacePos.w;

	// Convert to screen space
	float screenSpaceWidth = (normalizedDeviceCoordinates.x * 0.5f + 0.5f) * mWidth;
	float screenSpaceHeight = (1.0f - (normalizedDeviceCoordinates.y * 0.5f + 0.5f)) * mHeight;
	ImVec2 screen(screenSpaceWidth, screenSpaceHeight);

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

	float minX = mWidth, minY = mHeight;
	float maxX = 0, maxY = 0;

	glm::mat4 viewProj = camera.GetProjectionMatrix() * camera.GetViewMatrix();

	for (auto& corner : localCorners) {
		glm::vec4 worldPos = modelMatrix * glm::vec4(corner, 1.0f);

		glm::vec4 clip = viewProj * worldPos;
		if (clip.w <= 0) continue;

		glm::vec3 normalizedDeviceCoordinates = glm::vec3(clip) / clip.w;

		ImVec2 screen(
			(normalizedDeviceCoordinates.x * 0.5f + 0.5f) * mWidth,
			(1.0f - (normalizedDeviceCoordinates.y * 0.5f + 0.5f)) * mHeight
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