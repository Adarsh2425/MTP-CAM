#include "camera.h"
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/rotate_vector.hpp>
#include <glm/gtx/vector_angle.hpp>

// Constructor
Camera::Camera(glm::vec3 startPosition, glm::vec3 startTarget) :
    position(startPosition),
    target(startTarget),
    up(glm::vec3(0.0f, 1.0f, 0.0f)),
    yaw(-45.0f),
    pitch(-35.0f),
    roll(0.0f),
    distance(glm::distance(startPosition, startTarget)),
    fov(45.0f),
    nearPlane(0.1f),
    farPlane(1000.0f),
    orbitMode(false),
    movementSpeed(2.5f),
    mouseSensitivity(0.1f),
    zoomSensitivity(1.0f)
{
    updateCameraVectors();
}

// Returns the view matrix calculated using LookAt Matrix
glm::mat4 Camera::GetViewMatrix() const {
    return glm::lookAt(position, target, up);
}

// Returns the projection matrix with FOV, aspect ratio, near and far planes
glm::mat4 Camera::GetProjectionMatrix(float aspectRatio) const {
    return glm::perspective(glm::radians(fov), aspectRatio, nearPlane, farPlane);
}

// Resets the camera to the given position and target
void Camera::Reset(glm::vec3 position, glm::vec3 target) {
    this->position = position;
    this->target = target;
    this->yaw = -45.0f;
    this->pitch = -35.0f;
    this->roll = 0.0f;
    this->distance = glm::distance(position, target);
    this->fov = 45.0f;
    this->nearPlane = 0.1f;
    this->farPlane = 1000.0f;
    this->orbitMode = false;
    updateCameraVectors();
}

// Updates the camera's vectors based on the updated Euler angles
void Camera::updateCameraVectors() {
    // Calculate the front vector from the camera's Euler Angles
    glm::vec3 front;
    front.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
    front.y = sin(glm::radians(pitch));
    front.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
    this->front = glm::normalize(front);

    // Calculate the right and up vector
    this->right = glm::normalize(glm::cross(this->front, glm::vec3(0.0f, 1.0f, 0.0f)));
    this->up = glm::normalize(glm::cross(this->right, this->front));

    // Apply roll if needed
    if (roll != 0.0f) {
        this->right = glm::rotate(this->right, glm::radians(roll), this->front);
        this->up = glm::rotate(this->up, glm::radians(roll), this->front);
    }

    // Update position if in orbit mode
    if (orbitMode) {
        position.x = target.x + distance * cos(glm::radians(pitch)) * cos(glm::radians(yaw));
        position.y = target.y + distance * sin(glm::radians(pitch));
        position.z = target.z + distance * cos(glm::radians(pitch)) * sin(glm::radians(yaw));
    }
}

// Processes input received from a mouse input system
void Camera::ProcessMouseMovement(float xoffset, float yoffset, bool constrainPitch) {
    xoffset *= mouseSensitivity;
    yoffset *= mouseSensitivity;

    yaw += xoffset;
    pitch += yoffset;

    // Make sure that when pitch is out of bounds, screen doesn't get flipped
    if (constrainPitch) {
        if (pitch > 89.0f)
            pitch = 89.0f;
        if (pitch < -89.0f)
            pitch = -89.0f;
    }

    // Update front, right and up Vectors using the updated Euler angles
    updateCameraVectors();
}

// Processes input received from a mouse scroll-wheel event
void Camera::ProcessMouseScroll(float yoffset) {
    distance -= yoffset * zoomSensitivity;

    // Clamp the zoom distance
    if (distance < 1.0f) distance = 1.0f;
    if (distance > 100.0f) distance = 100.0f;

    updateCameraVectors();
}

// Pans the camera by a certain offset
void Camera::PanCamera(float xoffset, float yoffset) {
    float panSensitivity = 0.01f;
    glm::vec3 offset = right * xoffset * panSensitivity + up * yoffset * panSensitivity;

    position += offset;
    target += offset;

    updateCameraVectors();
}

// Adjusts the camera's field of view
void Camera::AdjustFOV(float fovOffset) {
    fov -= fovOffset;
    if (fov < 1.0f) fov = 1.0f;
    if (fov > 120.0f) fov = 120.0f;
}



Camera::CameraState Camera::SaveState() const {
    CameraState state = {
        position, target, up,
        yaw, pitch, roll, distance, fov,
        nearPlane, farPlane, orbitMode,
        movementSpeed, mouseSensitivity, zoomSensitivity
    };
    return state;
}

void Camera::LoadState(const CameraState& state) {
    position = state.position;
    target = state.target;
    up = state.up;
    yaw = state.yaw;
    pitch = state.pitch;
    roll = state.roll;
    distance = state.distance;
    fov = state.fov;
    nearPlane = state.nearPlane;
    farPlane = state.farPlane;
    orbitMode = state.orbitMode;
    movementSpeed = state.movementSpeed;
    mouseSensitivity = state.mouseSensitivity;
    zoomSensitivity = state.zoomSensitivity;

    updateCameraVectors();
}
