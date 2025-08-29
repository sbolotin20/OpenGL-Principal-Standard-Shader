// texture_utils.h
#pragma once
#include <glad/glad.h>
#include <string>

GLuint LoadTexture2D(const std::string& path, bool generateMipmaps=true,
                     bool flipY=true); // returns GL texture id
