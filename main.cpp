#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <tinyfiledialogs.h>
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include <glm/gtc/type_ptr.hpp>
#include <vector>

#include "shader.h"
#include "model.h"
#include "camera.h"
#include "callbacks.h"

// Global variables for camera and model
Camera camera(glm::vec3(5.0f, 5.0f, 5.0f), glm::vec3(0.0f, 0.0f, 0.0f));
Camera::CameraState savedState;
std::vector<Mesh> meshes;

GLuint framebuffer, textureColorbuffer, rbo;
GLuint gridVAO, gridVBO;

// Variables for ImGui controls
glm::vec3 clearColor = glm::vec3(0.2f, 0.3f, 0.3f);
bool wireframeMode = false;
bool keys[1024];
float cameraSpeed = 2.5f;
float fov = 45.0f;
glm::vec3 modelRotation = glm::vec3(0.0f, 0.0f, 0.0f);
bool showGrid = true;
int gridSize = 10;
glm::vec3 lightPos(1.2f, 1.0f, 2.0f);
glm::vec3 lightColor(1.0f, 1.0f, 1.0f);
glm::vec3 objectColor(1.0f, 0.5f, 0.2f);
float lightIntensity = 1.0f;
glm::vec3 overallMin(std::numeric_limits<float>::max());
glm::vec3 overallMax(-std::numeric_limits<float>::max());

float lastFrame = 0.0f;
float calculateDeltaTime() {
    float currentFrame = glfwGetTime();
    float deltaTime = currentFrame - lastFrame;
    lastFrame = currentFrame;
    return deltaTime;
}

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    Camera* camera = static_cast<Camera*>(glfwGetWindowUserPointer(window));
    if (!camera) return;

    if (action == GLFW_PRESS) {
        keys[key] = true;
    }
    else if (action == GLFW_RELEASE) {
        keys[key] = false;
    }
}

void setupFramebuffer(int width, int height) {
    glGenFramebuffers(1, &framebuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);

    glGenTextures(1, &textureColorbuffer);
    glBindTexture(GL_TEXTURE_2D, textureColorbuffer);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, textureColorbuffer, 0);

    glGenRenderbuffers(1, &rbo);
    glBindRenderbuffer(GL_RENDERBUFFER, rbo);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, width, height);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rbo);

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        std::cerr << "ERROR::FRAMEBUFFER:: Framebuffer is not complete!" << std::endl;

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    glViewport(0, 0, width, height);
    setupFramebuffer(width, height);
}
void createGrid(int gridSize) {
    std::vector<float> vertices;
    float halfSize = gridSize / 2.0f;

    for (int i = -gridSize; i <= gridSize; i++) {
        vertices.push_back(i);
        vertices.push_back(0.0f);
        vertices.push_back(-halfSize);

        vertices.push_back(i);
        vertices.push_back(0.0f);
        vertices.push_back(halfSize);

        vertices.push_back(-halfSize);
        vertices.push_back(0.0f);
        vertices.push_back(i);

        vertices.push_back(halfSize);
        vertices.push_back(0.0f);
        vertices.push_back(i);
    }

    glGenVertexArrays(1, &gridVAO);
    glGenBuffers(1, &gridVBO);

    glBindVertexArray(gridVAO);

    glBindBuffer(GL_ARRAY_BUFFER, gridVBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), &vertices[0], GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glBindVertexArray(0);
}

void renderGrid() {
    glBindVertexArray(gridVAO);
    glDrawArrays(GL_LINES, 0, gridSize * 4 * 2);
    glBindVertexArray(0);
}

int main() {
    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW" << std::endl;
        return -1;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window = glfwCreateWindow(640, 360, "3D Model Viewer", NULL, NULL);
    if (!window) {
        std::cerr << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);

    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetKeyCallback(window, key_callback);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetScrollCallback(window, scroll_callback);
    glfwSetWindowUserPointer(window, &camera);
    glfwSetMouseButtonCallback(window, mouse_button_callback);


    glewExperimental = GL_TRUE;
    GLenum err = glewInit();
    if (err != GLEW_OK) {
        std::cerr << "Failed to initialize GLEW: " << glewGetErrorString(err) << std::endl;
        return -1;
    }

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;

    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 330");

    ImGui::StyleColorsDark();

    glEnable(GL_DEPTH_TEST);
    setupFramebuffer(640, 360);

    GLuint shaderProgram = createShaderProgram(vertexShaderSource, fragmentShaderSource);

    GLint modelLoc = glGetUniformLocation(shaderProgram, "model");
    GLint viewLoc = glGetUniformLocation(shaderProgram, "view");
    GLint projLoc = glGetUniformLocation(shaderProgram, "projection");
    GLint lightPosLoc = glGetUniformLocation(shaderProgram, "lightPos");
    GLint lightColorLoc = glGetUniformLocation(shaderProgram, "lightColor");
    GLint objectColorLoc = glGetUniformLocation(shaderProgram, "objectColor");
    GLint lightIntensityLoc = glGetUniformLocation(shaderProgram, "lightIntensity");

    createGrid(gridSize);

    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();
        float deltaTime = calculateDeltaTime();

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        // Main menu bar
        if (ImGui::BeginMainMenuBar()) {
            if (ImGui::BeginMenu("File")) {
                if (ImGui::MenuItem("Open", "Ctrl+O")) {
                    const char* filters[] = { "*.obj", "*.stl" };
                    const char* newPath = tinyfd_openFileDialog("Open 3D Model", "", 2, filters, "3D Files", 0);
                    if (newPath) {
                        loadModel(newPath, meshes);

                        positionModelOnGrid(meshes); // Center the model on the grid

                        glm::vec3 center = (overallMin + overallMax) / 2.0f;
                        glm::vec3 size = overallMax - overallMin;
                        float distance = glm::length(size) * 1.5f;

                        camera.target = center;
                        camera.position = center + glm::vec3(distance, distance, distance);
                        camera.updateCameraVectors();
                    }
                }
                if (ImGui::MenuItem("Save Screenshot", "Ctrl+S")) {
                    // Add screenshot functionality
                }
                if (ImGui::MenuItem("Exit", "Ctrl+Q")) {
                    glfwSetWindowShouldClose(window, true);
                }
                ImGui::EndMenu();
            }

            if (ImGui::BeginMenu("View")) {
                ImGui::MenuItem("Show Grid", NULL, &showGrid);
                ImGui::MenuItem("Wireframe Mode", NULL, &wireframeMode);
                ImGui::EndMenu();
            }

            if (ImGui::BeginMenu("Help")) {
                if (ImGui::MenuItem("About")) {
                    ImGui::OpenPopup("AboutPopup");
                }
                ImGui::EndMenu();
            }
            ImGui::EndMainMenuBar();
        }

        // About Popup
        if (ImGui::BeginPopupModal("AboutPopup", NULL, ImGuiWindowFlags_AlwaysAutoResize)) {
            ImGui::Text("3D Model Viewer\n");
            ImGui::Text("Version 1.0\n");
            ImGui::Text("Created by: Adarsh \n");
            ImGui::Separator();
            ImGui::Text("Use this application to view 3D models with various options like wireframe mode, lighting, and more.");
            ImGui::Separator();
            if (ImGui::Button("Close")) {
                ImGui::CloseCurrentPopup();
            }
            ImGui::EndPopup();
        }

        // Render to framebuffer
        glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
        glClearColor(clearColor.r, clearColor.g, clearColor.b, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        GLenum currentPolygonMode = wireframeMode ? GL_LINE : GL_FILL;
        static GLenum lastPolygonMode = GL_FILL;
        if (lastPolygonMode != currentPolygonMode) {
            glPolygonMode(GL_FRONT_AND_BACK, currentPolygonMode);
            lastPolygonMode = currentPolygonMode;
        }

        renderScene(window, shaderProgram, meshes, camera, lightIntensity, lightColor, lightPos, objectColor);

        if (showGrid) {
            renderGrid();
        }

        glBindFramebuffer(GL_FRAMEBUFFER, 0);
       
            ImGui::Begin("3D Model Viewer Controls");

            // Existing controls...
            ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
            ImGui::ColorEdit3("Background Color", (float*)&clearColor);
            ImGui::Checkbox("Wireframe Mode", &wireframeMode);
            ImGui::SliderFloat("Camera Speed", &cameraSpeed, 0.1f, 10.0f);
            ImGui::SliderFloat("Field of View", &fov, 30.0f, 90.0f);
            ImGui::SliderFloat3("Model Rotation", (float*)&modelRotation, -180.0f, 180.0f);
            ImGui::ColorEdit3("Object Color", glm::value_ptr(objectColor));
            ImGui::Checkbox("Show Grid", &showGrid);

            // Reset button
            if (ImGui::Button("Reset Camera")) {
                camera.Reset(glm::vec3(0.0f, 0.0f, 10.0f), glm::vec3(0.0f, 0.0f, 0.0f));
            }

            // Camera Controls
            ImGui::Text("Camera Controls");

            // Slider and input field combined
            ImGui::SliderFloat3("Camera Position", glm::value_ptr(camera.position), -10.0f, 10.0f);
            ImGui::InputFloat3("Camera Position Input", glm::value_ptr(camera.position));

            ImGui::SliderFloat3("Camera Target", glm::value_ptr(camera.target), -10.0f, 10.0f);
            ImGui::InputFloat3("Camera Target Input", glm::value_ptr(camera.target));

            ImGui::SliderFloat3("Camera Up Vector", glm::value_ptr(camera.up), -1.0f, 1.0f);
            ImGui::InputFloat3("Camera Up Vector Input", glm::value_ptr(camera.up));

            ImGui::SliderFloat("Yaw", &camera.yaw, -180.0f, 180.0f);
            ImGui::InputFloat("Yaw Input", &camera.yaw);

            ImGui::SliderFloat("Pitch", &camera.pitch, -89.0f, 89.0f);
            ImGui::InputFloat("Pitch Input", &camera.pitch);

            ImGui::SliderFloat("Zoom", &camera.distance, 1.0f, 100.0f);
            ImGui::InputFloat("Zoom Input", &camera.distance);

            ImGui::SliderFloat("Field of View", &camera.fov, 1.0f, 120.0f);
            ImGui::InputFloat("Field of View Input", &camera.fov);

            ImGui::SliderFloat("Roll", &camera.roll, -180.0f, 180.0f);
            ImGui::InputFloat("Roll Input", &camera.roll);

            ImGui::SliderFloat("Near Plane", &camera.nearPlane, 0.1f, 10.0f);
            ImGui::InputFloat("Near Plane Input", &camera.nearPlane);

            ImGui::SliderFloat("Far Plane", &camera.farPlane, 10.0f, 1000.0f);
            ImGui::InputFloat("Far Plane Input", &camera.farPlane);

            // Save and Load Buttons
   // Save and Load Buttons
            if (ImGui::Button("Save Camera State")) {
                savedState = camera.SaveState();
            }

            if (ImGui::Button("Load Camera State")) {
                camera.LoadState(savedState);
            }

            // Orbit Mode
            static bool orbitMode = false;
            ImGui::Checkbox("Orbit Mode", &orbitMode);
            camera.orbitMode = orbitMode;

        
        

        ImGui::Text("Lighting Controls");
        ImGui::SliderFloat3("Light Position", glm::value_ptr(lightPos), -10.0f, 10.0f);
        ImGui::ColorEdit3("Light Color", glm::value_ptr(lightColor));
        ImGui::SliderFloat("Light Intensity", &lightIntensity, 0.0f, 2.0f);

        camera.updateCameraVectors();  // Update camera after changes

        glUseProgram(shaderProgram);
        glUniform3fv(lightPosLoc, 1, glm::value_ptr(lightPos));
        glUniform3fv(lightColorLoc, 1, glm::value_ptr(lightColor));
        glUniform1f(lightIntensityLoc, lightIntensity);
        glUniform3fv(objectColorLoc, 1, glm::value_ptr(objectColor)); // Add this line to update object color

        ImGui::Text("Scene Info");
        ImGui::Text("Camera Position: (%.2f, %.2f, %.2f)", camera.position.x, camera.position.y, camera.position.z);
        ImGui::Text("Camera Yaw: %.2f, Pitch: %.2f", camera.yaw, camera.pitch);
        ImGui::Text("FOV: %.2f, Zoom: %.2f", fov, camera.distance);
        ImGui::Text("Light Position: (%.2f, %.2f, %.2f)", lightPos.x, lightPos.y, lightPos.z);

        ImGui::End();

        // Display framebuffer texture in ImGui
        ImGui::Begin("3D Model Viewer");
        ImGui::Image((void*)(intptr_t)textureColorbuffer, ImVec2(640, 360));
        ImGui::End();

        ImGui::Render();
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        glfwSwapBuffers(window);
    }

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    glDeleteVertexArrays(1, &gridVAO);
    glDeleteBuffers(1, &gridVBO);

    glfwTerminate();
    return 0;
}

