#pragma once
#include <glad/glad.h>
#include <glm/glm.hpp>

struct LightingUniforms {
GLint uLightType;
GLint uLightPos;
GLint uLightColor;
GLint uAmbient;
GLint uDirDir;
GLint uSpotCosInner;
GLint uSpotCosOuter;
GLint uCamPos;
};


struct MaterialUniforms {
GLint uUseBaseTex;
GLint uBaseTex;
GLint uBaseTint;
GLint uRoughness;
GLint uMetallic;
GLint uDielectricF0;
};


LightingUniforms getLightingUniforms(GLuint program);
MaterialUniforms getMaterialUniforms(GLuint program);