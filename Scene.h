#pragma once
#include <unordered_map>
#include "Object.h"

class Scene
{
public:
	std::unordered_map<std::string, Object> mObjects;
	Object* mSelectedObject = nullptr;

	void AddObjectsToScene(const std::vector<std::shared_ptr<Model>>& models);
	Object& GetObject(const char* name);
	void AddModelDataToObjectsCollection(const std::shared_ptr<Model>& model, const glm::mat4& modelMatrix, const std::string& name);
	void UpdateAllObjectsBounds();
	glm::vec3 GetObjectPosition(std::shared_ptr<Model> model);
	glm::vec3 GetObjectRotation(std::shared_ptr<Model> model);
	glm::quat GetObjectOrientationQuat(std::shared_ptr<Model> model);
	glm::vec3 GetObjectScale(std::shared_ptr<Model> model);
	glm::mat4 GetObjectMatrix(std::shared_ptr<Model> model);
	void SetObjectMatrix(std::shared_ptr<Model>& model, glm::vec3 position, glm::vec3 rotation, glm::vec3 scale);
};

