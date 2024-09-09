// callbacks.h
#ifndef CALLBACKS_H
#define CALLBACKS_H

#include <GL/glew.h>      // Must be included before other OpenGL headers
#include <GLFW/glfw3.h>   // GLFW should come after GLEW
#include <vector>
#include "camera.h"
#include "model.h"

// Mouse movement callback function
void mouse_callback(GLFWwindow* window, double xpos, double ypos);

// Scroll callback function to handle zoom
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);

// Keyboard callback function to handle camera reset
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);
void mouse_button_callback(GLFWwindow* window, int button, int action, int mods);

// Render the scene (includes shader, model, and camera updates)
void renderScene(GLFWwindow* window, GLuint shaderProgram, const std::vector<Mesh>& meshes, Camera& camera,float lightIntensity, glm::vec3 lightColor, glm::vec3 lightPos, glm::vec3 objectColor);

glm::vec3 GetWorldCoordinatesAtMousePosition(GLFWwindow* window);
#endif // CALLBACKS_H
