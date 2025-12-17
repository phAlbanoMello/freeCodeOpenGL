#include "Camera.h"
#include <glm/gtc/epsilon.hpp>

//Class Responsibility :
//Process perspective transformations and process movement related inputs

Camera::Camera(int width, int height, glm::vec3 position, glm::vec3 orientation) {
	Camera::width = width;
	Camera::height = height;

	Position = position;
    lastPosition = position;
    startPosition = position;

    Orientation = orientation;
    startOrientation = orientation;
    lastOrientation = orientation;

    viewMatrix = glm::mat4(1.0);
    projectionMatrix = glm::mat4(1.0);
}

void Camera::updateMatrix(float FOVdeg, float nearPlane, float farPlane)
{
    viewMatrix = glm::lookAt(Position, Position + Orientation, Up);
    projectionMatrix = glm::perspective(glm::radians(FOVdeg), (float)(width / height), nearPlane, farPlane);

    cameraMatrix = projectionMatrix * viewMatrix;
}

void Camera::Matrix(Shader& shader, const char* uniform)
{
    glUniformMatrix4fv(glGetUniformLocation(shader.ID, uniform), 1, GL_FALSE, glm::value_ptr(cameraMatrix));
}

void Camera::Inputs(GLFWwindow* window, float deltaTime) {
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
        Position += (speed * Orientation) * deltaTime;
    }
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
        Position += (speed * -Orientation) * deltaTime;
    }
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
        Position += (speed * -glm::normalize(glm::cross(Orientation, Up))) * deltaTime;
    }
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
        Position += (speed * glm::normalize(glm::cross(Orientation, Up))) * deltaTime;
    }
    if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS) {
        Position += (speed * Up) * deltaTime;
    }
    if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_RIGHT_CONTROL) == GLFW_PRESS) {
        Position += (speed * -Up) * deltaTime;
    }

 
    if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS && Camera::PositionChanged()) {
        speed += speedIncreaseRate;
    }

    if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_RELEASE) {
        speed = initialSpeed;
    }

    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
    {
        Position = startPosition;
        Orientation = startOrientation;
    }
    // Mouse Control
    if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS) {
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);

        if (firstClick) {
            glfwSetCursorPos(window, (width / 2), (height / 2));
            firstClick = false;
        }

        double mouseX, mouseY;
        glfwGetCursorPos(window, &mouseX, &mouseY);

        float rotX = sensitivity * (float)(mouseY - (height / 2)) / height;
        float rotY = sensitivity * (float)(mouseX - (width / 2)) / width;

        glm::vec3 newOrientation = glm::rotate(
            Orientation,
            glm::radians(-rotX),
            glm::normalize(glm::cross(Orientation, Up))
        );

        if (abs(glm::angle(newOrientation, Up) - glm::radians(90.0f)) <= glm::radians(85.f)) {
            Orientation = newOrientation;
        }

        Orientation = glm::rotate(Orientation, glm::radians(-rotY), Up);

        glfwSetCursorPos(window, (width / 2), (height / 2));
    }
    else if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_RELEASE) {
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
        firstClick = true;
    }

    if (Camera::PositionChanged() || Camera::OrientationChanged()) {
        ClearLog();
        LogState();
    }
    lastPosition = Position;
    lastOrientation = Orientation;
}

void Camera::LogState()
{
    std::cout << "Camera Position: ("
        << Position.x << ", " << Position.y << ", " << Position.z << ")\n";
    std::cout << "Camera Orientation: ("
        << Orientation.x << ", " << Orientation.y << ", " << Orientation.z << ")\n";
}

bool Camera::PositionChanged() {
    bool positionChanged = glm::epsilonNotEqual(Position.x, lastPosition.x, epsilon) ||
        glm::epsilonNotEqual(Position.y, lastPosition.y, epsilon) ||
        glm::epsilonNotEqual(Position.z, lastPosition.z, epsilon);

    return positionChanged;
}
bool Camera::OrientationChanged() {
    bool orientationChanged = glm::epsilonNotEqual(Orientation.x, lastOrientation.x, epsilon) ||
        glm::epsilonNotEqual(Orientation.y, lastOrientation.y, epsilon) ||
        glm::epsilonNotEqual(Orientation.z, lastOrientation.z, epsilon);

    return orientationChanged;
}

void Camera::ClearLog()
{
    std::cout << "\033[2J\033[1;1H";
}

glm::mat4 Camera::GetViewMatrix() const {
    return viewMatrix;
}

glm::mat4 Camera::GetProjectionMatrix() const {
    return projectionMatrix;
}

void Camera::SaveCameraState() {
    std::cout << "Saving Camera State" << std::endl;
    LogState();

    nlohmann::json cameraStateData;
    cameraStateData["position"] = { Position.x, Position.y, Position.z };
    cameraStateData["orientation"] = { Orientation.x, Orientation.y, Orientation.z };

    std::ofstream file("camera_state.json");
    if (file.is_open()) {
        file << cameraStateData.dump(4);
        file.close();
    }
}

CameraData Camera::LoadCameraState() {
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