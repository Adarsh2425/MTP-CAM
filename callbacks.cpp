#include <iostream>
#include "callbacks.h"
#include "shader.h"
#include "model.h"
#include "camera.h"
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

// Global variables for mouse handling
static float lastX = 640.0f, lastY = 360.0f;
static bool firstMouse = true;
bool isDragging = false;
bool isAxisConstrained = false;  // New flag for axis-constrained dragging
glm::vec3 dragStartPosition;
glm::vec3 selectedObjectPosition;
glm::vec3 dragAxis;
extern Camera camera;

void mouse_button_callback(GLFWwindow* window, int button, int action, int mods) {
    if (button == GLFW_MOUSE_BUTTON_RIGHT) {
        if (action == GLFW_PRESS) {
            // Initiate dragging
            isDragging = true;
            dragStartPosition = GetWorldCoordinatesAtMousePosition(window);
            selectedObjectPosition = dragStartPosition; // Initialize selected object's position

            // Check for axis constraint modifiers (e.g., Shift or Ctrl)
            if (mods & GLFW_MOD_SHIFT) {
                // Axis-constrained dragging (example: along X-axis)
                isAxisConstrained = true;
                dragAxis = glm::vec3(1.0f, 0.0f, 0.0f);  // Constrain to X-axis
            }
            else if (mods & GLFW_MOD_CONTROL) {
                // Axis-constrained dragging (example: along Y-axis)
                isAxisConstrained = true;
                dragAxis = glm::vec3(0.0f, 1.0f, 0.0f);  // Constrain to Y-axis
            }
            else {
                isAxisConstrained = false;
            }
        }
        else if (action == GLFW_RELEASE) {
            // End dragging
            isDragging = false;
            isAxisConstrained = false;
        }
    }
}

void mouse_callback(GLFWwindow* window, double xpos, double ypos) {
    static float lastX = xpos;
    static float lastY = ypos;

    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos;  // Reversed since y-coordinates range from bottom to top
    lastX = xpos;
    lastY = ypos;

    if (isDragging) {
        glm::vec3 currentMousePosition = GetWorldCoordinatesAtMousePosition(window);
        glm::vec3 offset = currentMousePosition - dragStartPosition;

        // Apply axis constraint if active
        if (isAxisConstrained) {
            offset = dragAxis * glm::dot(offset, dragAxis);
        }

        selectedObjectPosition += offset;
        dragStartPosition = currentMousePosition;

        // Update the position of the object you're dragging
        // Example:
        // myModel.SetPosition(selectedObjectPosition);
    }
    else if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_MIDDLE) == GLFW_PRESS) {
        // Middle-click: Pan camera
        camera.PanCamera(xoffset, yoffset);
    }
    else if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS) {
        // Right-click: Orbit camera
        camera.ProcessMouseMovement(xoffset, yoffset);
    }
}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset) {
    Camera* camera = static_cast<Camera*>(glfwGetWindowUserPointer(window));
    if (!camera) return;

    camera->ProcessMouseScroll(static_cast<float>(yoffset));
}

glm::vec3 GetWorldCoordinatesAtMousePosition(GLFWwindow* window) {
    Camera* camera = static_cast<Camera*>(glfwGetWindowUserPointer(window));
    if (!camera) return glm::vec3(0.0f);

    // Get the current mouse position
    double xpos, ypos;
    glfwGetCursorPos(window, &xpos, &ypos);

    int width, height;
    glfwGetWindowSize(window, &width, &height);

    // Retrieve the depth buffer value at the mouse position
    float depth;
    glReadPixels(static_cast<int>(xpos), height - static_cast<int>(ypos), 1, 1, GL_DEPTH_COMPONENT, GL_FLOAT, &depth);

    // Create a 3D point in screen space
    glm::vec3 screenPos(static_cast<float>(xpos), static_cast<float>(height - ypos), depth);

    // Convert the screen space position to world space using glm::unProject
    glm::mat4 projectionMatrix = glm::perspective(glm::radians(camera->fov), static_cast<float>(width) / static_cast<float>(height), 0.1f, 100.0f);
    glm::mat4 viewMatrix = camera->GetViewMatrix();
    glm::vec4 viewport(0.0f, 0.0f, static_cast<float>(width), static_cast<float>(height));

    return glm::unProject(screenPos, viewMatrix, projectionMatrix, viewport);
}

void renderScene(GLFWwindow* window, GLuint shaderProgram, const std::vector<Mesh>& meshes, Camera& camera,
    float lightIntensity, glm::vec3 lightColor, glm::vec3 lightPos, glm::vec3 objectColor) {
    // Clear the screen and set up shader program
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glUseProgram(shaderProgram);

    // Update the view and projection matrices
    glm::mat4 view = camera.GetViewMatrix();
    glm::mat4 projection = glm::perspective(glm::radians(camera.fov), 1280.0f / 720.0f, 0.1f, 100.0f);

    // Set the shader uniforms
    glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "view"), 1, GL_FALSE, glm::value_ptr(view));
    glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
    glUniform3fv(glGetUniformLocation(shaderProgram, "lightPos"), 1, glm::value_ptr(lightPos));
    glUniform3fv(glGetUniformLocation(shaderProgram, "lightColor"), 1, glm::value_ptr(lightColor));
    glUniform1f(glGetUniformLocation(shaderProgram, "lightIntensity"), lightIntensity);
    glUniform3f(glGetUniformLocation(shaderProgram, "viewPos"), camera.position.x, camera.position.y, camera.position.z);
    glUniform3fv(glGetUniformLocation(shaderProgram, "objectColor"), 1, glm::value_ptr(objectColor));

    // Iterate over each mesh and render
    for (const Mesh& mesh : meshes) {
        glm::mat4 model = glm::translate(glm::mat4(1.0f), selectedObjectPosition);
        glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));

        glBindVertexArray(mesh.VAO);
        glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(mesh.indices.size()), GL_UNSIGNED_INT, 0);
        glBindVertexArray(0);
    }
}
