#include "Model.h"

class Main {
public:
	static Model GenerateModel(const char* path = "", unsigned int count = 1, std::vector<glm::mat4> instanceMatrix = {}, bool flipUVY = true);
};

void UpdateAsteroidsRotationAtInstanceMatrix(const unsigned int number, float time, float radius, float radiusDeviation, std::vector<glm::mat4>& instanceMatrix);
std::vector <glm::mat4> GenerateInstanceMatrix(float number, float radius, float radiusDeviation);