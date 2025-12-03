#include "Model.h"
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

struct CameraData {
	glm::vec3 position;
	glm::vec3 orientation;
};
struct Object {
	std::string name;

	// Transform parameters
	glm::vec3 position{ 0.0f };
	glm::vec3 rotation{ 0.0f }; // em graus (Euler)
	glm::vec3 scale{ 1.0f };

	// Derived data
	glm::mat4 matrix{ 1.0f };
	glm::vec3 minBounds;
	glm::vec3 maxBounds;
};

class Main {
public:
	static Model GenerateModel(std::string path, bool flipUVY);
	static void DrawSceneWithShader(Shader shader, Camera camera, bool debugBounds);
	static void AddObjectToPickablesCollection(const std::shared_ptr<Model>& model, const glm::mat4& modelMatrix, const std::string& name);
	static Object MakeObjectFromModelMatrix(const std::shared_ptr<Model>& model,
		const glm::mat4& modelMatrix,
		const std::string& name);
	static void DebugModelCenterScreenSpace(Camera camera, glm::mat4 modelMatrix, int width, int height, ImU32 color);
	static void DebugModelBoundsScreenSpace(Camera camera, glm::mat4 modelMatrix, const glm::vec3& boundsMin, const glm::vec3& boundsMax);
	static void DrawLight(Shader shader, Camera camera, glm::vec4 color, glm::vec3 position, bool debugBounds);
	static void SaveCameraState(Camera& cam);
	static CameraData LoadCameraState();
	//static bool RayIntersectsSphere(const glm::vec3& rayOrigin, const glm::vec3& rayDir, const Object& obj, float& tHit);
	static bool RayIntersectsAABB(const glm::vec3& rayOrigin, const glm::vec3& rayDir, const Object& obj, float& tHit);
};

void DrawObjectEditor();
