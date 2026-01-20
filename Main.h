#include "Model.h"
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include "Object.h"

class Main {
public:
	//Rendering
	static void DrawSceneWithShader(std::shared_ptr<Shader>& shader, Camera camera, bool debugBounds);
	static std::vector<glm::mat4>  GenerateLightSpaceMatrices(glm::vec3 currentLightPosition, float nearPlane, float farPlane);
	static void SetupMainShaderUniforms(std::shared_ptr<Shader>& shader, glm::vec3& currentLightPos);
	static void DrawLight(std::shared_ptr<Shader> shader, Camera camera, glm::vec4 color, glm::vec3 position, bool debugBounds);
	
	//Picking
	static Object* GetSelectedObject(const Camera& camera, GLFWwindow* window);
	static bool RayIntersectsAABB(const glm::vec3& rayOrigin, const glm::vec3& rayDir, const Object& obj, float& tHit);
	
	//Debug
	static void DebugModelCenterScreenSpace(Camera camera, glm::mat4 modelMatrix, ImU32 color);
	static void DebugModelBoundsScreenSpace(Camera camera, glm::mat4 modelMatrix, const glm::vec3& boundsMin, const glm::vec3& boundsMax);
};

//ImGUI
void DrawObjectEditor();
void DrawText(const char* text, float xOffset, float yOffset);
void HandleObjectSelection(GLFWwindow* window, ImGuiIO& io, const Camera& camera);
void UpdateUI();
void UpdateSelectedObjectPosition();

//Shadow Renderer
void ShadowMapRenderPass(std::vector<glm::mat4>& lightSpaceMatrices, const unsigned int SHADOW_WIDTH, const unsigned int SHADOW_HEIGHT, unsigned int depthMapFBO);
