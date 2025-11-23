// mesh_utils.cpp
#include "mesh_utils.h"
#include "External/tinyobjloader/tiny_obj_loader.h"
#include <glad/glad.h>
#include <cstddef>
#include <fstream>
#include <sstream>
#include <string>
#include <iostream>
#include <filesystem>


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
    glBufferData(GL_ARRAY_BUFFER, 
        vertices.size() * sizeof(Vertex), 
        vertices.data(), GL_STATIC_DRAW);
    

    // EBO
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh.EBO); // bind the buffer (target = array buffer)
    glBufferData(GL_ELEMENT_ARRAY_BUFFER,
            indices.size() * sizeof(unsigned int),   // not sizeof(Vertex)
            indices.data(), GL_STATIC_DRAW);


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

void renderCube() {
    static unsigned int cubeVAO = 0;
    static unsigned int cubeVBO = 0;
    if (cubeVAO == 0) {
        float vertices[] = {
            // positions          
            -1.0f, -1.0f, -1.0f,
             1.0f, -1.0f, -1.0f,
             1.0f,  1.0f, -1.0f,
             1.0f,  1.0f, -1.0f,
            -1.0f,  1.0f, -1.0f,
            -1.0f, -1.0f, -1.0f,

            -1.0f, -1.0f,  1.0f,
             1.0f, -1.0f,  1.0f,
             1.0f,  1.0f,  1.0f,
             1.0f,  1.0f,  1.0f,
            -1.0f,  1.0f,  1.0f,
            -1.0f, -1.0f,  1.0f,

            -1.0f,  1.0f,  1.0f,
            -1.0f,  1.0f, -1.0f,
            -1.0f, -1.0f, -1.0f,
            -1.0f, -1.0f, -1.0f,
            -1.0f, -1.0f,  1.0f,
            -1.0f,  1.0f,  1.0f,

             1.0f,  1.0f,  1.0f,
             1.0f,  1.0f, -1.0f,
             1.0f, -1.0f, -1.0f,
             1.0f, -1.0f, -1.0f,
             1.0f, -1.0f,  1.0f,
             1.0f,  1.0f,  1.0f,

            -1.0f, -1.0f, -1.0f,
             1.0f, -1.0f, -1.0f,
             1.0f, -1.0f,  1.0f,
             1.0f, -1.0f,  1.0f,
            -1.0f, -1.0f,  1.0f,
            -1.0f, -1.0f, -1.0f,

            -1.0f,  1.0f, -1.0f,
             1.0f,  1.0f, -1.0f,
             1.0f,  1.0f,  1.0f,
             1.0f,  1.0f,  1.0f,
            -1.0f,  1.0f,  1.0f,
            -1.0f,  1.0f, -1.0f
        };
        glGenVertexArrays(1, &cubeVAO);
        glGenBuffers(1, &cubeVBO);
        glBindBuffer(GL_ARRAY_BUFFER, cubeVBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
        glBindVertexArray(cubeVAO);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    }
    glBindVertexArray(cubeVAO);
    glDrawArrays(GL_TRIANGLES, 0, 36);
    glBindVertexArray(0);
}

Mesh createCube() {
    // Each face needs its own vertices to have correct UV mapping
    // 24 vertices total (4 per face, 6 faces)
    std::vector<Vertex> vertices = {
        // Front face (Z+)
        { glm::vec3(-0.5f, -0.5f,  0.5f), glm::vec3(0.0f, 0.0f, 1.0f), glm::vec2(0.0f, 0.0f), glm::vec3(0.0f) },
        { glm::vec3( 0.5f, -0.5f,  0.5f), glm::vec3(0.0f, 0.0f, 1.0f), glm::vec2(1.0f, 0.0f), glm::vec3(0.0f) },
        { glm::vec3( 0.5f,  0.5f,  0.5f), glm::vec3(0.0f, 0.0f, 1.0f), glm::vec2(1.0f, 1.0f), glm::vec3(0.0f) },
        { glm::vec3(-0.5f,  0.5f,  0.5f), glm::vec3(0.0f, 0.0f, 1.0f), glm::vec2(0.0f, 1.0f), glm::vec3(0.0f) },

        // Back face (Z-)
        { glm::vec3( 0.5f, -0.5f, -0.5f), glm::vec3(0.0f, 0.0f, -1.0f), glm::vec2(0.0f, 0.0f), glm::vec3(0.0f) },
        { glm::vec3(-0.5f, -0.5f, -0.5f), glm::vec3(0.0f, 0.0f, -1.0f), glm::vec2(1.0f, 0.0f), glm::vec3(0.0f) },
        { glm::vec3(-0.5f,  0.5f, -0.5f), glm::vec3(0.0f, 0.0f, -1.0f), glm::vec2(1.0f, 1.0f), glm::vec3(0.0f) },
        { glm::vec3( 0.5f,  0.5f, -0.5f), glm::vec3(0.0f, 0.0f, -1.0f), glm::vec2(0.0f, 1.0f), glm::vec3(0.0f) },

        // Left face (X-)
        { glm::vec3(-0.5f, -0.5f, -0.5f), glm::vec3(-1.0f, 0.0f, 0.0f), glm::vec2(0.0f, 0.0f), glm::vec3(0.0f) },
        { glm::vec3(-0.5f, -0.5f,  0.5f), glm::vec3(-1.0f, 0.0f, 0.0f), glm::vec2(1.0f, 0.0f), glm::vec3(0.0f) },
        { glm::vec3(-0.5f,  0.5f,  0.5f), glm::vec3(-1.0f, 0.0f, 0.0f), glm::vec2(1.0f, 1.0f), glm::vec3(0.0f) },
        { glm::vec3(-0.5f,  0.5f, -0.5f), glm::vec3(-1.0f, 0.0f, 0.0f), glm::vec2(0.0f, 1.0f), glm::vec3(0.0f) },

        // Right face (X+)
        { glm::vec3( 0.5f, -0.5f,  0.5f), glm::vec3(1.0f, 0.0f, 0.0f), glm::vec2(0.0f, 0.0f), glm::vec3(0.0f) },
        { glm::vec3( 0.5f, -0.5f, -0.5f), glm::vec3(1.0f, 0.0f, 0.0f), glm::vec2(1.0f, 0.0f), glm::vec3(0.0f) },
        { glm::vec3( 0.5f,  0.5f, -0.5f), glm::vec3(1.0f, 0.0f, 0.0f), glm::vec2(1.0f, 1.0f), glm::vec3(0.0f) },
        { glm::vec3( 0.5f,  0.5f,  0.5f), glm::vec3(1.0f, 0.0f, 0.0f), glm::vec2(0.0f, 1.0f), glm::vec3(0.0f) },

        // Bottom face (Y-)
        { glm::vec3(-0.5f, -0.5f, -0.5f), glm::vec3(0.0f, -1.0f, 0.0f), glm::vec2(0.0f, 0.0f), glm::vec3(0.0f) },
        { glm::vec3( 0.5f, -0.5f, -0.5f), glm::vec3(0.0f, -1.0f, 0.0f), glm::vec2(1.0f, 0.0f), glm::vec3(0.0f) },
        { glm::vec3( 0.5f, -0.5f,  0.5f), glm::vec3(0.0f, -1.0f, 0.0f), glm::vec2(1.0f, 1.0f), glm::vec3(0.0f) },
        { glm::vec3(-0.5f, -0.5f,  0.5f), glm::vec3(0.0f, -1.0f, 0.0f), glm::vec2(0.0f, 1.0f), glm::vec3(0.0f) },

        // Top face (Y+)
        { glm::vec3(-0.5f,  0.5f,  0.5f), glm::vec3(0.0f, 1.0f, 0.0f), glm::vec2(0.0f, 0.0f), glm::vec3(0.0f) },
        { glm::vec3( 0.5f,  0.5f,  0.5f), glm::vec3(0.0f, 1.0f, 0.0f), glm::vec2(1.0f, 0.0f), glm::vec3(0.0f) },
        { glm::vec3( 0.5f,  0.5f, -0.5f), glm::vec3(0.0f, 1.0f, 0.0f), glm::vec2(1.0f, 1.0f), glm::vec3(0.0f) },
        { glm::vec3(-0.5f,  0.5f, -0.5f), glm::vec3(0.0f, 1.0f, 0.0f), glm::vec2(0.0f, 1.0f), glm::vec3(0.0f) }
    };

    std::vector<unsigned int> indices = {
        // Front face
        0,  1,  2,    2,  3,  0,
        // Back face
        4,  5,  6,    6,  7,  4,
        // Left face
        8,  9, 10,   10, 11,  8,
        // Right face
        12, 13, 14,   14, 15, 12,
        // Bottom face
        16, 17, 18,   18, 19, 16,
        // Top face
        20, 21, 22,   22, 23, 20
    };

    ComputeTangents(vertices, indices);
    return createMesh(vertices, indices);
}

Mesh loadObjModel(const std::string& path) {
    tinyobj::attrib_t attrib;
    std::vector<tinyobj::shape_t> shapes;
    std::vector<tinyobj::material_t> materials;
    std::string warn, err;

    bool success = tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, path.c_str());

    if (!warn.empty()) std::cout << "tinyobj warning: " << warn << std::endl;
    if (!err.empty()) std::cerr << "tinyobj error: " << err << std::endl;
    if (!success) {
        std::cerr << "Failed to load OBJ: " << path << std::endl;
        return createCube(); // fallback
    }

    std::vector<Vertex> vertices;
    std::vector<unsigned int> indices;

    std::unordered_map<std::string, unsigned int> uniqueVertexMap;

    for (const auto& shape : shapes) {
        size_t index_offset = 0;
        for (size_t f = 0; f < shape.mesh.num_face_vertices.size(); f++) {
            int fv = shape.mesh.num_face_vertices[f];
            for (int v = 0; v < fv; v++) {
                tinyobj::index_t idx = shape.mesh.indices[index_offset + v];

                glm::vec3 pos = {
                    attrib.vertices[3 * idx.vertex_index + 0],
                    attrib.vertices[3 * idx.vertex_index + 1],
                    attrib.vertices[3 * idx.vertex_index + 2]
                };

                glm::vec3 normal = glm::vec3(0.0f);
                if (idx.normal_index >= 0 && !attrib.normals.empty()) {
                    normal = {
                        attrib.normals[3 * idx.normal_index + 0],
                        attrib.normals[3 * idx.normal_index + 1],
                        attrib.normals[3 * idx.normal_index + 2]
                    };
                }

                glm::vec2 uv = glm::vec2(0.0f);
                if (idx.texcoord_index >= 0 && !attrib.texcoords.empty()) {
                    uv = {
                        attrib.texcoords[2 * idx.texcoord_index + 0],
                        attrib.texcoords[2 * idx.texcoord_index + 1]
                    };
                }

                Vertex vert;
                vert.position = pos;
                vert.normal = normal;
                vert.texCoord = uv;
                vert.tangent = glm::vec3(0.0f); // to be calculated

                std::string key = std::to_string(idx.vertex_index) + "/" +
                                  std::to_string(idx.normal_index) + "/" +
                                  std::to_string(idx.texcoord_index);

                if (uniqueVertexMap.count(key) == 0) {
                    uniqueVertexMap[key] = static_cast<unsigned int>(vertices.size());
                    vertices.push_back(vert);
                }

                indices.push_back(uniqueVertexMap[key]);
            }
            index_offset += fv;
        }
    }

    ComputeTangents(vertices, indices);
    return createMesh(vertices, indices);
}
