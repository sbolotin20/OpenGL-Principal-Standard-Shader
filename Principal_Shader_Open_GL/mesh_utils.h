#pragma once
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <string>
#include <fstream>
#include <sstream>
#include <string>
#include <iostream>
#include <cstddef>
#include <filesystem>
// ─────────────────────────────────────────────
// Vertex struct: holds per-vertex data
// ─────
// Needed to send position, normal, UV, and tangent to the GPU.
struct Vertex {
    glm::vec3 position;
    glm::vec3 normal;
    glm::vec2 texCoord;
    glm::vec3 tangent = glm::vec3(0.0);
};

// ─────────────────────────────────────────────
// Mesh struct: holds GPU handle info and helpers
// ─────
struct Mesh {
    GLuint VAO; // Vertex Array Object: blueprint of how OpenGL should handle vertex data later in rendering
    GLuint VBO; // Vertex Buffer Object: holds actual vertex data (like triangle positions)
    GLuint EBO;
    int vertexCount;
    int indexCount;

    void draw() const {
        glBindVertexArray(VAO);
        glDrawElements(GL_TRIANGLES, indexCount, GL_UNSIGNED_INT, 0);
    }

    void cleanup() const {
        glDeleteVertexArrays(1, &VAO);
        glDeleteBuffers(1, &VBO);
        glDeleteBuffers(1, &EBO);
    }
};

void ComputeTangents(std::vector<Vertex>& vertices, const std::vector<unsigned int>& indices); // calculate tangent vectors for each vertex to support nomal mapping
Mesh createQuad();
Mesh createMesh(); // generic function for any obj passed in 
Mesh createCube();
Mesh loadObjModel(const std::string& path);
void renderCube();


