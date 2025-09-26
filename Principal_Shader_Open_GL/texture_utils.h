// texture_utils.h
#pragma once
#include "shader_utils.h"
#include "mesh_utils.h"
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <string>

GLuint LoadTexture2D(const std::string& path, bool generateMipmaps=true, bool flipY=true); // returns GL texture id
GLuint LoadHDRTexture(const std::string& path);
GLuint EquirectToCubemap(GLuint hdrTex, GLuint cubeVAO, GLuint cubeVBO, int size = 512);
GLuint ConvolveIrradiance(GLuint envCubemap);