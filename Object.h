#pragma once
#include <string>
#include <glm/fwd.hpp>
#include <glm/vec3.hpp>
#include <glm/mat4x4.hpp>
#include <memory>
#include "Model.h"

class Object
{
public:
	std::string name;

    glm::vec3 position{ 0.0f };
    glm::vec3 rotation{ 0.0f };
    glm::vec3 scale{ 1.0f };
    glm::mat4 matrix{ 1.0f };
    glm::vec3 minBounds{ 0.0f };
    glm::vec3 maxBounds{ 0.0f };
    glm::vec3 center{ 0.0f };

	std::shared_ptr<Model> mModel = nullptr;

	void CalculateObjectBounds();
    static Object MakeObjectFromModelMatrix(const std::shared_ptr<Model>& model, const glm::mat4& modelMatrix, const std::string& name);
};

