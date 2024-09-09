// model.h
#ifndef MODEL_H
#define MODEL_H

#include <vector>
#include <string>
#include <glm/glm.hpp>
#include <assimp/scene.h>
#include <GL/glew.h>      // Must be included before other OpenGL headers
#include <GLFW/glfw3.h>   // GLFW should come after GLEW
#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>


struct Mesh {
    std::vector<float> vertices;      // Vertex positions and normals
    std::vector<unsigned int> indices; // Indices for vertex elements
    GLuint VAO = 0;                   // Initialize to 0 to avoid uninitialized variable warnings
    GLuint VBO = 0;                   // Initialize to 0 to avoid uninitialized variable warnings
    GLuint EBO = 0;                   // Initialize to 0 to avoid uninitialized variable warnings
};



// Load a model from file
void loadModel(const std::string& path, std::vector<Mesh>& meshes);

// Process a node in the Assimp scene graph
void processNode(aiNode* node, const aiScene* scene, std::vector<Mesh>& meshes);

// Process an individual mesh in the Assimp scene
Mesh processMesh(aiMesh* mesh, const aiScene* scene);

// Compute the bounding box for a model
void computeBoundingBox(const Mesh& mesh, glm::vec3& min, glm::vec3& max);
void positionModelOnGrid(std::vector<Mesh>& meshes);

#endif //MODEL_H