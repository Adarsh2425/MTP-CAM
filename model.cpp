// model.cpp
#include "model.h"
#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <iostream>

// Load a 3D model from a file
void loadModel(const std::string& path, std::vector<Mesh>& meshes) {
    // Create an Assimp Importer object
    Assimp::Importer importer;

    // Read the model file and process it
    const aiScene* scene = importer.ReadFile(path, aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_JoinIdenticalVertices);

    // Check for errors during the import process
    if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
        std::cerr << "Error loading model: " << importer.GetErrorString() << std::endl;
        return;
    }

    // Process the root node recursively
    processNode(scene->mRootNode, scene, meshes);
}

// Process a node in the Assimp scene graph recursively
void processNode(aiNode* node, const aiScene* scene, std::vector<Mesh>& meshes) {
    // Process each mesh at the current node
    for (unsigned int i = 0; i < node->mNumMeshes; i++) {
        aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
        meshes.push_back(processMesh(mesh, scene));
    }

    // Recursively process each child node
    for (unsigned int i = 0; i < node->mNumChildren; i++) {
        processNode(node->mChildren[i], scene, meshes);
    }
}

// Process an individual mesh to extract vertex data and indices
Mesh processMesh(aiMesh* mesh, const aiScene* scene) {
    Mesh resultMesh; // Mesh object to hold vertex data and OpenGL buffers
    std::vector<float> vertices;
    std::vector<unsigned int> indices;

    // Iterate through each vertex in the mesh
    for (unsigned int i = 0; i < mesh->mNumVertices; i++) {
        // Store the position of the vertex
        vertices.push_back(mesh->mVertices[i].x);
        vertices.push_back(mesh->mVertices[i].y);
        vertices.push_back(mesh->mVertices[i].z);

        // Store the normal of the vertex if it exists, otherwise use a default value
        if (mesh->mNormals) {
            vertices.push_back(mesh->mNormals[i].x);
            vertices.push_back(mesh->mNormals[i].y);
            vertices.push_back(mesh->mNormals[i].z);
        }
        else {
            vertices.push_back(0.0f);
            vertices.push_back(0.0f);
            vertices.push_back(1.0f);
        }
    }

    // Iterate through each face in the mesh and store the indices
    for (unsigned int i = 0; i < mesh->mNumFaces; i++) {
        aiFace face = mesh->mFaces[i];
        for (unsigned int j = 0; j < face.mNumIndices; j++) {
            indices.push_back(face.mIndices[j]);
        }
    }

    // Generate OpenGL buffers and arrays for the mesh
    glGenVertexArrays(1, &resultMesh.VAO);
    glGenBuffers(1, &resultMesh.VBO);
    glGenBuffers(1, &resultMesh.EBO);

    // Bind the vertex array object (VAO)
    glBindVertexArray(resultMesh.VAO);

    // Bind and set vertex buffer data
    glBindBuffer(GL_ARRAY_BUFFER, resultMesh.VBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), &vertices[0], GL_STATIC_DRAW);

    // Bind and set element buffer data (indices)
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, resultMesh.EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), &indices[0], GL_STATIC_DRAW);

    // Set vertex attribute pointers (positions and normals)
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    // Unbind the VAO for now
    glBindVertexArray(0);

    // Store the vertex and index data in the Mesh object
    resultMesh.vertices = vertices;
    resultMesh.indices = indices;

    return resultMesh;
}
void computeBoundingBox(const Mesh& mesh, glm::vec3& min, glm::vec3& max) {
    const auto& vertices = mesh.vertices;
    min = glm::vec3(std::numeric_limits<float>::max());
    max = glm::vec3(-std::numeric_limits<float>::max());

    for (size_t i = 0; i < vertices.size(); i += 6) { // Assuming 6 floats per vertex (position + normal)
        glm::vec3 vertex(
            vertices[i],
            vertices[i + 1],
            vertices[i + 2]
        );
        min = glm::min(min, vertex);
        max = glm::max(max, vertex);
    }
}
void positionModelOnGrid(std::vector<Mesh>& meshes) {
    glm::vec3 min, max, overallMin, overallMax;
    overallMin = glm::vec3(std::numeric_limits<float>::max());
    overallMax = glm::vec3(-std::numeric_limits<float>::max());

    // Compute the overall bounding box of all meshes
    for (const auto& mesh : meshes) {
        computeBoundingBox(mesh, min, max);
        overallMin = glm::min(overallMin, min);
        overallMax = glm::max(overallMax, max);
    }

    // Compute the translation needed to align the model's lowest point with the grid (y = 0)
    float translateY = -overallMin.y;
    std::cout << "Translate Y: " << translateY << std::endl;
    std::cout << "Overall Min Y: " << overallMin.y << std::endl;

    // Adjust each mesh so that the model's lowest point is on the grid
    for (auto& mesh : meshes) {
        for (size_t i = 0; i < mesh.vertices.size(); i += 6) {
            mesh.vertices[i + 1] += translateY; // Move the y-coordinate of each vertex
        }

        // Update the mesh's vertex buffer
        glBindBuffer(GL_ARRAY_BUFFER, mesh.VBO);
        glBufferData(GL_ARRAY_BUFFER, mesh.vertices.size() * sizeof(float), mesh.vertices.data(), GL_STATIC_DRAW);
    }
}



