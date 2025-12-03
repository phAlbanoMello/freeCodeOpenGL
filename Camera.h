#ifndef CAMERA_CLASS_H
#define CAMERA_CLASS_H

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include<glm/glm.hpp>
#include<glm/gtc/matrix_transform.hpp>
#include<glm/gtc/type_ptr.hpp>
#include<glm/gtx/rotate_vector.hpp>
#include<glm/gtx/vector_angle.hpp>

#include"Shader.h"

class Camera {
public:
	glm::vec3 Position;
	glm::vec3 Orientation = glm::vec3(0.66125, -0.710963, 0.239399);
	glm::vec3 Up = glm::vec3(0.f, 1.f, 0.f);
	glm::mat4 cameraMatrix = glm::mat4(1.f);

	bool firstClick = true;

	glm::vec3 lastPosition;
	glm::vec3 lastOrientation;
	glm::vec3 startPosition;
	glm::vec3 startOrientation;

	int	width;
	int height;

	float speedIncreaseRate = 0.15f;
	float initialSpeed = 0.5f;
	float speed = 0.5f;
	float sensitivity = 100.f;

	const float epsilon = 0.0001f;

	Camera(int width, int height, glm::vec3 position, glm::vec3 orientation);
	
	//Updates and exports the camera matrix to the Vertex Shader
	void updateMatrix(float FOVdeg, float nearPlane, float farPlane);
	void Matrix(Shader& shader, const char* uniform);
	void Inputs(GLFWwindow* window, float deltaTime);
	void LogState();

	glm::mat4 GetViewMatrix() const;
	glm::mat4 GetProjectionMatrix() const;

private:
	glm::mat4 viewMatrix;
	glm::mat4 projectionMatrix;

	void LogCameraIfChanged();
	bool PositionChanged();
	bool OrientationChanged();
	void ClearLog();
};

#endif
