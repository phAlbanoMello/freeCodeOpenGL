#include "Object.h"
#include <glm/gtx/matrix_decompose.hpp>

void Object::CalculateObjectBounds() {
	std::vector<glm::vec3> bounds;

	glm::vec3 minLocal = mModel->localInitialBoundsMin;
	glm::vec3 maxLocal = mModel->localInitialBoundsMax;

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

	for (auto& corner : localCorners) {
		glm::vec4 worldPos = matrix * glm::vec4(corner, 1.0f);
		minBounds = glm::min(minBounds, glm::vec3(worldPos));
		maxBounds = glm::max(maxBounds, glm::vec3(worldPos));
	}

	this->minBounds = minBounds;
	this->maxBounds = maxBounds;
	this->center = (this->minBounds + this->maxBounds) * 0.5f;
}

Object Object::MakeObjectFromModelMatrix(const std::shared_ptr<Model>& model, const glm::mat4& modelMatrix, const std::string& name) {

	glm::vec3 scale;
	glm::vec3 translation;
	glm::vec3 skew;
	glm::vec4 perspective;
	glm::quat rotation;
	glm::mat4 localMatrix(modelMatrix);

	glm::decompose(localMatrix, scale, rotation, translation, skew, perspective);

	Object obj;
	obj.name = name;
	obj.minBounds = model->localInitialBoundsMin;
	obj.maxBounds = model->localInitialBoundsMax;
	obj.matrix = modelMatrix;
	obj.position = translation;
	obj.rotation = glm::eulerAngles(rotation);
	obj.scale = scale;
	obj.mModel = model;

	return obj;
}