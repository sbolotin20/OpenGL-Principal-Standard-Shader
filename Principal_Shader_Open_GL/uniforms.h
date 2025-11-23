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
GLint uNormalTex;
GLint uUseNormalTex;
GLint uRoughnessMap;
GLint uMetallicMap;
GLint uAOMap;
GLint uUseRoughnessMap;
GLint uUseMetallicMap;
GLint uUseAOMap;
};

struct VertexUniforms {
GLint modelMatrix;
GLint viewMatrix;
GLint projectionMatrix;
};


LightingUniforms getLightingUniforms(GLuint program);
MaterialUniforms getMaterialUniforms(GLuint program);
VertexUniforms getVertexUniforms(GLuint program);