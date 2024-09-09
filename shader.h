// shader.h
#ifndef SHADER_H
#define SHADER_H

#include <GL/glew.h>      // Must be included before other OpenGL headers
#include <GLFW/glfw3.h>   // GLFW should come after GLEW

// Vertex Shader source code
extern const char* vertexShaderSource;

// Fragment Shader source code
extern const char* fragmentShaderSource;

// Function to compile a shader
GLuint createShader(GLenum type, const char* source);

// Function to create and link shader program
GLuint createShaderProgram(const char* vertexSource, const char* fragmentSource);

#endif // SHADER_H
