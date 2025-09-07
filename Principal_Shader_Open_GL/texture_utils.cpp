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
