// mesh_utils.cpp
#include "mesh_utils.h"
#include <glad/glad.h>
#include <cstddef>


Mesh createMesh(const std::vector<Vertex>& vertices, const std::vector<unsigned int>& indices) {
    Mesh mesh;
    mesh.vertexCount = vertices.size();
    mesh.indexCount = indices.size();
    
    glGenVertexArrays(1, &mesh.VAO); // generate 1 VAO
    glGenBuffers(1, &mesh.VBO); // create 1 buffer ID
    glGenBuffers(1, &mesh.EBO); 


    // VAO
    glBindVertexArray(mesh.VAO); // bind it (make it active)
    
    // VBO
    glBindBuffer(GL_ARRAY_BUFFER, mesh.VBO); // bind the buffer (target = array buffer)
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), vertices.data(), GL_STATIC_DRAW);

    // EBO
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh.EBO); // bind the buffer (target = array buffer)
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(Vertex), indices.data(), GL_STATIC_DRAW);

    // position attribute (location = 0)
    glVertexAttribPointer(
        0,                                  // index (matches "layout (location = 0)" in shader)
        3,                                  // size 
        GL_FLOAT,                           // type
        GL_FALSE,                           // normalize?
        sizeof(Vertex),                     // stride (size of 1 Vertex)
        (void*)offsetof(Vertex, position)   // offset (start at beginning of array)
    );
    glEnableVertexAttribArray(0); // enable that vertex attribute

    // normal attribute (location = 1)
    glVertexAttribPointer(
        1,                                  // index (matches "layout (location = 1)" in shader)
        3,                                  // size 
        GL_FLOAT,                           // type
        GL_FALSE,                           // normalize?
        sizeof(Vertex),                     // stride (size of 1 Vertex)
        (void*)offsetof(Vertex, normal)   // offset (start at beginning of array)
    );
    glEnableVertexAttribArray(1); // enable that vertex attribute

    // textCoord attribute (location = 2)
    glVertexAttribPointer(
        2,                                  // index (matches "layout (location = 2)" in shader)
        2,                                  // size 
        GL_FLOAT,                           // type
        GL_FALSE,                           // normalize?
        sizeof(Vertex),                     // stride (size of 1 Vertex)
        (void*)offsetof(Vertex, texCoord)   // offset (start at beginning of array)
    );
    glEnableVertexAttribArray(2); // enable that vertex attribute

    // tangent attribute (location = 3)
    glVertexAttribPointer(
        3,                                  // index (matches "layout (location = 3)" in shader)
        3,                                  // size 
        GL_FLOAT,                           // type
        GL_FALSE,                           // normalize?
        sizeof(Vertex),                     // stride (size of 1 Vertex)
        (void*)offsetof(Vertex, tangent)   // offset (start at beginning of array)
    );
    glEnableVertexAttribArray(3); // enable that vertex attribute

    glBindVertexArray(0); // unbinds VAO to prevent accidntal modification elswhere
    return mesh;
}

void ComputeTangents(std::vector<Vertex>& vertices, const std::vector<unsigned int>& indices) {
    // Loop through each triangle (3 indices at a time)
    for (size_t i = 0; i < indices.size(); i += 3) {
        // fetch triangle vertex data
        Vertex& v0 = vertices[indices[i]];
        Vertex& v1 = vertices[indices[i + 1]];
        Vertex& v2 = vertices[indices[i + 2]];

        // World Positions
        glm::vec3 pos0 = v0.position;
        glm::vec3 pos1 = v1.position;
        glm::vec3 pos2 = v2.position;

        // Texture coordinates
        glm::vec2 uv0 = v0.texCoord;
        glm::vec2 uv1 = v1.texCoord;
        glm::vec2 uv2 = v2.texCoord;

        // Vector Edges of the triangle (in model space)
        glm::vec3 edge1 = pos1 - pos0;
        glm::vec3 edge2 = pos2 - pos0;

        // UV deltas (in texture space) - UV space edges
        glm::vec2 deltaUV1 = uv1 - uv0;
        glm::vec2 deltaUV2 = uv2 - uv0;

        float f = 1.0f / (deltaUV1.x * deltaUV2.y - deltaUV2.x * deltaUV1.y);

        glm::vec3 tangent;
        tangent.x = f * (deltaUV2.y * edge1.x - deltaUV1.y * edge2.x);
        tangent.y = f * (deltaUV2.y * edge1.y - deltaUV1.y * edge2.y);
        tangent.z = f * (deltaUV2.y * edge1.z - deltaUV1.y * edge2.z);
        tangent = glm::normalize(tangent);

        // Accumulate tangent per vertex (in case of sharing)
        v0.tangent += tangent;
        v1.tangent += tangent;
        v2.tangent += tangent;
    }

    // Normalize the accumulated tangents
    for (Vertex& v : vertices) {
        v.tangent = glm::normalize(v.tangent);
    }
}

Mesh createQuad() {
    //each Vertex has vec of position, normal, texCoord and tangent 
    std::vector<Vertex> vertices = {
        // Bottom-left
        { glm::vec3(-1.0f, -1.0f, 0.0f),  glm::vec3(0.0f, 0.0f, 1.0f),  glm::vec2(0.0f, 0.0f), glm::vec3(0.0f) },

        // Bottom-right
        { glm::vec3( 1.0f, -1.0f, 0.0f),  glm::vec3(0.0f, 0.0f, 1.0f),  glm::vec2(1.0f, 0.0f), glm::vec3(0.0f) },

        // Top-left
        { glm::vec3(-1.0f,  1.0f, 0.0f),  glm::vec3(0.0f, 0.0f, 1.0f),  glm::vec2(0.0f, 1.0f), glm::vec3(0.0f) },

        // Top-right
        { glm::vec3( 1.0f,  1.0f, 0.0f),  glm::vec3(0.0f, 0.0f, 1.0f),  glm::vec2(1.0f, 1.0f), glm::vec3(0.0f) },
    };

    std::vector<unsigned int> indices = {
        0, 2, 1, // Bottom-left, Top-left, Bottom-right
        2, 3, 1  // Top-left, Top-right, Bottom-right
    };

    ComputeTangents(vertices, indices); // compute tangent vectors based on texture layouts
    return createMesh(vertices, indices); // send to GPU
}