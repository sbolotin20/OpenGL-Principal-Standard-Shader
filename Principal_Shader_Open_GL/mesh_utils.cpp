// mesh_utils.cpp
#include "mesh_utils.h"
#include <glad/glad.h>

#include "mesh_utils.h"


Mesh createTriangle() {
    float vertices[] = {
    // positions // texture coords
    -0.5f, -0.5f, 0.0f, 0.0f, 0.0f,
    0.5f, -0.5f, 0.0f, 1.0f, 0.0f,
    0.0f, 0.5f, 0.0f, 0.5f, 1.0f
    };

    Mesh mesh;
    mesh.vertexCount = 3;

    glGenVertexArrays(1, &mesh.VAO); // generate 1 VAO
    glGenBuffers(1, &mesh.VBO); // create 1 buffer ID

    glBindVertexArray(mesh.VAO); // bind it (make it active)
    glBindBuffer(GL_ARRAY_BUFFER, mesh.VBO); // bind the buffer (target = array buffer)
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    // position attribute (location = 0)
    glVertexAttribPointer(
        0,                // index (matches "layout (location = 0)" in shader)
        3,                // size (vec3 = 3 floats)
        GL_FLOAT,         // type
        GL_FALSE,         // normalize?
        5 * sizeof(float),// stride (distance between sets of attributes - there are 5 floats per vertex)
        (void*)0          // offset (start at beginning of array)
    );
    glEnableVertexAttribArray(0); // enable that vertex attribute

    // texture coordinate attribute (location = 1)
    glVertexAttribPointer(
        1,                // index (matches "layout (location = 1)" in shader)
        2,                // size (vec2 = 2 floats)
        GL_FLOAT,         // type
        GL_FALSE,         // normalize?
        5 * sizeof(float),// stride (distance between sets of attributes)
        (void*)(3 * sizeof(float))          // offset (skips past the position attributes)
    );
    glEnableVertexAttribArray(1);  // enable that vertex attribute

    return mesh;
}
