#include "Scene.h"

void Scene::AddModelDataToObjectsCollection(const std::shared_ptr<Model>& model, const glm::mat4& modelMatrix, const std::string& name)
{
	if (mObjects.find(name) == mObjects.end()) {
		mObjects.insert({ name, Object::MakeObjectFromModelMatrix(model, modelMatrix, name) });
	}
}

void Scene::UpdateAllObjectsBounds() {
	for (auto& [name, obj] : mObjects) {
		obj.CalculateObjectBounds();
	}
}

glm::vec3 Scene::GetObjectPosition(std::shared_ptr<Model> model) {
	auto it = mObjects.find(model->name);
	if (it == mObjects.end()) {
		std::cout << "Object not found in collection : " << model->name << std::endl;
		return glm::vec3(0.f);
	}

	return it->second.position;
}

glm::vec3 Scene::GetObjectRotation(std::shared_ptr<Model> model) {
	auto it = mObjects.find(model->name);
	if (it == mObjects.end()) {
		std::cout << "Object not found in collection : " << model->name << std::endl;
		return glm::vec3(0.f);
	}

	return it->second.rotation;
}

glm::vec3 Scene::GetObjectScale(std::shared_ptr<Model> model) {
	auto it = mObjects.find(model->name);
	if (it == mObjects.end()) {
		std::cout << "Object not found in collection : " << model->name << std::endl;
		return glm::vec3(0.f);
	}

	return it->second.scale;
}

glm::mat4 Scene::GetObjectMatrix(std::shared_ptr<Model> model) {
	auto it = mObjects.find(model->name);
	if (it == mObjects.end()) {
		std::cout << "Object not found in collection : " << model->name << std::endl;
		return glm::mat4(1.f);
	}

	return it->second.matrix;
}

void Scene::SetObjectMatrix(std::shared_ptr<Model>& model, glm::vec3 position, glm::vec3 rotation, glm::vec3 scale) {
	glm::mat4 matrix = translate(glm::mat4(1.0f), position);

	matrix = glm::rotate(matrix, glm::radians(rotation.x), glm::vec3(1.0f, 0.0f, 0.0f));
	matrix = glm::rotate(matrix, glm::radians(rotation.y), glm::vec3(0.0f, 1.0f, 0.0f));
	matrix = glm::rotate(matrix, glm::radians(rotation.z), glm::vec3(0.0f, 0.0f, 1.0f));

	matrix = glm::scale(matrix, scale);

	auto it = mObjects.find(model->name);
	if (it != mObjects.end()) {
		Object& obj = it->second;
		obj.matrix = matrix;
		obj.position = position;
		obj.rotation = rotation;
		obj.scale = scale;

		std::cout << "Set Object Matrix : " << obj.name << std::endl;
	}
}

void Scene::AddObjectsToScene(const std::vector<std::shared_ptr<Model>>& models) {
	for (size_t i = 0; i < models.size(); i++)
	{
		glm::vec3 position = glm::vec3(0.0f, 0.0f, 0.0f);
		glm::mat4 matrix = glm::translate(glm::mat4(1.0f), position);

		AddModelDataToObjectsCollection(models[i], matrix, models[i]->name);
	}
}

Object& Scene::GetObject(const char* name) {
	auto it = mObjects.find(name);
	if (it != mObjects.end()) {
		Object& obj = it->second;

		return obj;
	}
}