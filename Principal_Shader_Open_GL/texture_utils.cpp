#include "texture_utils.h"
#include <fstream>
#include <sstream>
#include <iostream>

#define STB_IMAGE_IMPLEMENTATION
#include "External/stb_image.h"



GLuint LoadTexture2D(const std::string& path, bool generateMipmaps, bool flipY) {
    stbi_set_flip_vertically_on_load(flipY);

    int width, height, nrChannels;
    unsigned char* data = stbi_load(path.c_str(), &width, &height, &nrChannels, 0);
    if (!data) {
        std::cerr << "Failed to load texture at: " << path << std::endl;
        std::cerr << "STB Error: " << stbi_failure_reason() << std::endl;
        
        // Create a default 1x1 white texture instead of returning 0
        GLuint texture;
        glGenTextures(1, &texture);
        glBindTexture(GL_TEXTURE_2D, texture);
        unsigned char white[] = {255, 255, 255, 255};
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, white);
        return texture;
    }

    // Generate texture and upload data to GPU
    GLuint texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);

    // Texture sampling and wrapping behavior
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, generateMipmaps ? GL_LINEAR_MIPMAP_LINEAR : GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    GLenum format;
    if (nrChannels == 1)
        format = GL_RED;  // Grayscale
    else if (nrChannels == 3)
        format = GL_RGB;
    else if (nrChannels == 4)
        format = GL_RGBA;
    else {
        std::cerr << "Unexpected number of channels: " << nrChannels << std::endl;
        stbi_image_free(data);
        return 0;
    }

    GLint internalFormat;
    if (nrChannels == 1)
        internalFormat = GL_RED;
    else if (nrChannels == 3)
        internalFormat = GL_RGB;
    else
        internalFormat = GL_RGBA;
    glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, width, height, 0, format, GL_UNSIGNED_BYTE, data);

    if (generateMipmaps)
        glGenerateMipmap(GL_TEXTURE_2D);

    stbi_image_free(data);
    return texture;
}

GLuint LoadHDRTexture(const std::string& path) {
    // Use stbi_loadf for floating point data
    // HDR files store linear values that can exceed 1.0
    int width, height, nrChannels;
    stbi_set_flip_vertically_on_load(true);
    float* data = stbi_loadf(path.c_str(), &width, &height, &nrChannels, 0);

    if (!data) {
        std::cerr << "Failed to load hdr texture at: " << path << std::endl;
        std::cerr << "STB Error: " << stbi_failure_reason() << std::endl;
        return 0;
    }

    // Generate texture and upload data to GPU
    GLuint hdrTexture;
    glGenTextures(1, &hdrTexture);
    glBindTexture(GL_TEXTURE_2D, hdrTexture);

    // set parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    GLenum format;
    if (nrChannels == 1)
        format = GL_RED;  // Grayscale
    else if (nrChannels == 3)
        format = GL_RGB;
    else if (nrChannels == 4)
        format = GL_RGBA;
    else {
        std::cerr << "Unexpected number of channels: " << nrChannels << std::endl;
        stbi_image_free(data);
        return 0;
    }

   GLint internalFormat;
    if (nrChannels == 1)
        internalFormat = GL_R16F;
    else if (nrChannels == 3)
        internalFormat = GL_RGB16F;
    else
        internalFormat = GL_RGBA16F;
    
    glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, width, height, 0, format, GL_FLOAT, data);

    stbi_image_free(data);
    return hdrTexture;
    
    // Your task:
    // 1. Call stbi_set_flip_vertically_on_load(true)
    // 2. Use stbi_loadf() instead of stbi_load()
    // 3. Create texture with GL_RGB16F or GL_RGB32F internal format
    // 4. Use GL_FLOAT as type instead of GL_UNSIGNED_BYTE
}

GLuint EquirectToCubemap(GLuint hdrTex, GLuint /*unused*/, GLuint /*unused*/, int size) {
    GLuint captureFBO, captureRBO;
    glGenFramebuffers(1, &captureFBO);
    glGenRenderbuffers(1, &captureRBO);
    glBindFramebuffer(GL_FRAMEBUFFER, captureFBO);
    glBindRenderbuffer(GL_RENDERBUFFER, captureRBO);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, size, size);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, captureRBO);

    GLuint envCubemap;
    glGenTextures(1, &envCubemap);
    glBindTexture(GL_TEXTURE_CUBE_MAP, envCubemap);
    for (int i = 0; i < 6; ++i)
        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB16F, size, size, 0, GL_RGB, GL_FLOAT, nullptr);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glm::mat4 proj = glm::perspective(glm::radians(90.0f), 1.0f, 0.1f, 10.0f);
    glm::mat4 views[] = {
        glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3( 1.0f,  0.0f,  0.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
        glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(-1.0f,  0.0f,  0.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
        glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3( 0.0f,  1.0f,  0.0f), glm::vec3(0.0f,  0.0f,  1.0f)),
        glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3( 0.0f, -1.0f,  0.0f), glm::vec3(0.0f,  0.0f, -1.0f)),
        glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3( 0.0f,  0.0f,  1.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
        glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3( 0.0f,  0.0f, -1.0f), glm::vec3(0.0f, -1.0f,  0.0f))
    };

    // compile shaders
    std::string vertexSource = ReadTextFile("shaders/cubemap_vertex.vert");
    std::string fragSource = ReadTextFile("shaders/equirect_to_cubemap.frag");

    GLuint vertex_shader = CompileShader(GL_VERTEX_SHADER, vertexSource.c_str());
    GLuint frag_shader = CompileShader(GL_FRAGMENT_SHADER, fragSource.c_str());
    GLuint shader_program = LinkProgram(vertex_shader, frag_shader);
    glUseProgram(shader_program);

    GLint loc_equirectangularMap = glGetUniformLocation(shader_program, "equirectangularMap");
    GLint loc_proj     = glGetUniformLocation(shader_program, "projection");
    GLint loc_view     = glGetUniformLocation(shader_program, "view");
    glUniform1i(loc_equirectangularMap, 0);
    glUniformMatrix4fv(loc_proj, 1, GL_FALSE, glm::value_ptr(proj));

    // bind HDR
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, hdrTex);

    // state
    GLint prevViewport[4]; glGetIntegerv(GL_VIEWPORT, prevViewport);
    glViewport(0, 0, size, size);
    glEnable(GL_DEPTH_TEST);
    GLint prevDepthFunc; glGetIntegerv(GL_DEPTH_FUNC, &prevDepthFunc);
    glDepthFunc(GL_LEQUAL);

    // render into each face
    glBindFramebuffer(GL_FRAMEBUFFER, captureFBO);
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        std::cerr << "Cubemap capture FBO incomplete\n";
    }
    for (int i = 0; i < 6; ++i) {
        glUniformMatrix4fv(loc_view, 1, GL_FALSE, glm::value_ptr(views[i]));
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
                               GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, envCubemap, 0);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        renderCube(); // your helper that binds its own VAO
    }

    // restore state
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glViewport(prevViewport[0], prevViewport[1], prevViewport[2], prevViewport[3]);
    glDepthFunc(prevDepthFunc);

    glDeleteRenderbuffers(1, &captureRBO);
    glDeleteFramebuffers(1, &captureFBO);
    glDeleteProgram(shader_program);
    glDeleteShader(vertex_shader);
    glDeleteShader(frag_shader);

    return envCubemap;
}
