#include "uniforms.h"
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

LightingUniforms getLightingUniforms(GLuint program) {
    LightingUniforms u;
    u.uLightType = glGetUniformLocation(program, "uLightType");
    u.uLightPos = glGetUniformLocation(program, "uLight_Position");
    u.uLightColor = glGetUniformLocation(program, "uLight_Color");
    u.uAmbient = glGetUniformLocation(program, "uAmbient");
    u.uDirDir = glGetUniformLocation(program, "uDir_Direction");
    u.uSpotCosInner = glGetUniformLocation(program, "uSpotCosInner");
    u.uSpotCosOuter = glGetUniformLocation(program, "uSpotCosOuter");
    u.uCamPos = glGetUniformLocation(program, "uCamera_Position");
    return u;
}


MaterialUniforms getMaterialUniforms(GLuint program) {
    MaterialUniforms u;
    u.uUseBaseTex = glGetUniformLocation(program, "useBaseColorTex");
    u.uBaseTex = glGetUniformLocation(program, "baseColorTex");
    u.uBaseTint = glGetUniformLocation(program, "baseColorTint");
    u.uRoughness = glGetUniformLocation(program, "uRoughness");
    u.uMetallic = glGetUniformLocation(program, "uMetallic");
    u.uDielectricF0 = glGetUniformLocation(program, "uDielectricF0");
    u.uNormalTex = glGetUniformLocation(program, "uNormalTex");
    u.uUseNormalTex = glGetUniformLocation(program, "uUseNormalTex");
    u.uRoughnessMap = glGetUniformLocation(program, "roughnessMap");
    u.uAOMap = glGetUniformLocation(program, "aoMap");
    u.uMetallicMap = glGetUniformLocation(program, "metallicMap");
    u.uUseRoughnessMap = glGetUniformLocation(program, "useRoughnessMap");
    u.uUseMetallicMap = glGetUniformLocation(program, "useMetallicMap");
    u.uUseAOMap = glGetUniformLocation(program, "useAOMap");
    return u;
}

VertexUniforms getVertexUniforms(GLuint program) {
    VertexUniforms u;
    u.modelMatrix = glGetUniformLocation(program, "modelMatrix");
    u.viewMatrix = glGetUniformLocation(program, "viewMatrix");
    u.projectionMatrix = glGetUniformLocation(program, "projectionMatrix");
    return u;
}

