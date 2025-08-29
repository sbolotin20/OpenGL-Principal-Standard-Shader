#pragma once
#include <glad/glad.h>


struct Mesh {
    GLuint VAO; // Vertex Array Object: blueprint of how OpenGL should handle vertex data later in rendering
    GLuint VBO; // Vertex Buffer Object: holds actual vertex data (like triangle positions)
    int vertexCount;


    void draw() const {
        glBindVertexArray(VAO);
        glDrawArrays(GL_TRIANGLES, 0, vertexCount);
    }


    void cleanup() const {
        glDeleteVertexArrays(1, &VAO);
        glDeleteBuffers(1, &VBO);
    }
};


Mesh createTriangle();