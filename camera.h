#ifndef CAMERA_H
#define CAMERA_H

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/component_wise.hpp>
#include <glm/gtx/norm.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/gtx/rotate_vector.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtx/vector_angle.hpp>

class Camera {
public:
    struct CameraState {
        glm::vec3 position, target, up;
        float yaw, pitch, roll, distance, fov;
        float nearPlane, farPlane;
        bool orbitMode;
        float movementSpeed, mouseSensitivity, zoomSensitivity;
    };

    glm::vec3 position, target, up, front, right;
    float yaw, pitch, roll, distance, fov;
    float nearPlane, farPlane;
    bool orbitMode;
    float movementSpeed, mouseSensitivity, zoomSensitivity;

    Camera(glm::vec3 startPosition, glm::vec3 startTarget);

    glm::mat4 GetViewMatrix() const;
    glm::mat4 GetProjectionMatrix(float aspectRatio) const;

    void ProcessMouseMovement(float xoffset, float yoffset, bool constrainPitch = true);
    void ProcessMouseScroll(float yoffset);
    void PanCamera(float xoffset, float yoffset);
    void AdjustFOV(float fovOffset);

    void Reset(glm::vec3 position, glm::vec3 target);
    void updateCameraVectors();

    CameraState SaveState() const;
    void LoadState(const CameraState& state);
};

#endif
