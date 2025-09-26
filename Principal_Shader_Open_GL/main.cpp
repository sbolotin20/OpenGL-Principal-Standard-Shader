// main.cpp - DEBUG VERSION
#include <fstream>
#include <sstream>
#include <string>
#include <iostream>
#include <filesystem>

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "External/stb_image.h"
#include "shader_utils.h"
#include "texture_utils.h"
#include "mesh_utils.h"
#include "uniforms.h"

// ─────────────────────────────────────────────
// Window Settings
// ─────
const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;

// globals
GLuint baseColorTextureID;
GLuint normalMapTextureID;
GLuint roughnessTextureID;
GLuint metallicTextureID;
GLuint hdrTextureID;

// ─────────────────────────────────────────────
// Main
int main() {
    std::cout << "OpenGL project is running!" << std::endl;
    std::cout << "Working directory: " << std::filesystem::current_path() << std::endl;

    // Check if texture files exist
    std::cout << "Checking for textures:" << std::endl;
    std::cout << "  base_color.jpg exists: " << std::filesystem::exists("textures/base_color.jpg") << std::endl;
    std::cout << "  normal_map.png exists: " << std::filesystem::exists("textures/normal_map.png") << std::endl;
    std::cout << "  roughness_map.png exists: " << std::filesystem::exists("textures/roughness_map.png") << std::endl;

    // ------ Initialize GLFW and Create Window ------
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "LearnOpenGL", NULL, NULL);
    if (window == NULL) {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    
     // ----- Load OpenGL functions with GLAD -----
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cerr << "Failed to initialize GLAD\n";
        return -1;
    }

    int w, h;
    glfwGetFramebufferSize(window, &w, &h);
    glViewport(0, 0, w, h);

    // ----- Compile and Link Shaders ------
    std::string vertexSource = ReadTextFile("shaders/basic.vert");
    std::string fragSource = ReadTextFile("shaders/basic.frag");

    std::cout << "Vertex shader source length: " << vertexSource.length() << std::endl;
    std::cout << "Fragment shader source length: " << fragSource.length() << std::endl;

    GLuint vertex_shader = CompileShader(GL_VERTEX_SHADER, vertexSource.c_str());
    GLuint frag_shader = CompileShader(GL_FRAGMENT_SHADER, fragSource.c_str());
    GLuint shader_program = LinkProgram(vertex_shader, frag_shader);

    // DEBUG: Check shader program status
    GLint success;
    glGetProgramiv(shader_program, GL_LINK_STATUS, &success);
    if (!success) {
        char infoLog[512];
        glGetProgramInfoLog(shader_program, 512, NULL, infoLog);
        std::cout << "SHADER LINKING FAILED: " << infoLog << std::endl;
    } else {
        std::cout << "Shader program linked successfully! Program ID: " << shader_program << std::endl;
    }

    glUseProgram(shader_program);

    // ----- Set up Vertex Data (Position + UV) -----
    Mesh mesh = createCube();
    GLuint VAO = mesh.VAO;

    std::cout << "Mesh created - VAO: " << mesh.VAO << ", VBO: " << mesh.VBO << ", Index count: " << mesh.indexCount << std::endl;

    // ---- Load Texture -----
    baseColorTextureID = LoadTexture2D("textures/base_color.jpg");
    normalMapTextureID = LoadTexture2D("textures/normal_map.png");
    roughnessTextureID = LoadTexture2D("textures/roughness_map.png");
    
    std::cout << "Texture IDs - Base: " << baseColorTextureID << ", Normal: " << normalMapTextureID 
              << ", Roughness: " << roughnessTextureID << std::endl;
    
    hdrTextureID = LoadHDRTexture("textures/test.hdr"); // use .hdr
    std::cout << "HDR texture ID: " << hdrTextureID << std::endl;
    
    GLuint envCubemap = EquirectToCubemap(hdrTextureID, 0, 0, 512);
    glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);
    GLuint irradianceMap = ConvolveIrradiance(envCubemap);
    
    std::cout << "Environment cubemap ID: " << envCubemap << ", Irradiance map ID: " << irradianceMap << std::endl;

    // compile skybox program once
    std::string sbVS = ReadTextFile("shaders/skybox.vert");
    std::string sbFS = ReadTextFile("shaders/skybox.frag");
    GLuint sbV = CompileShader(GL_VERTEX_SHADER, sbVS.c_str());
    GLuint sbF = CompileShader(GL_FRAGMENT_SHADER, sbFS.c_str());
    GLuint sbProg = LinkProgram(sbV, sbF);
    
    // Check skybox shader linking
    glGetProgramiv(sbProg, GL_LINK_STATUS, &success);
    if (!success) {
        char infoLog[512];
        glGetProgramInfoLog(sbProg, 512, NULL, infoLog);
        std::cout << "SKYBOX SHADER LINKING FAILED: " << infoLog << std::endl;
    } else {
        std::cout << "Skybox shader linked successfully! Program ID: " << sbProg << std::endl;
    }
    
    glUseProgram(sbProg);
    glUniform1i(glGetUniformLocation(sbProg, "env"), 0);
    GLint sbView = glGetUniformLocation(sbProg, "view");
    GLint sbProj = glGetUniformLocation(sbProg, "projection");

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    int width, height; 
    glfwGetFramebufferSize(window, &width, &height);
    glViewport(0, 0, w, h);

    // ---- Material and Lighting Setup -----
    LightingUniforms lightUniforms = getLightingUniforms(shader_program);
    MaterialUniforms matUniforms = getMaterialUniforms(shader_program);
    VertexUniforms vertUniforms = getVertexUniforms(shader_program);

    // DEBUG: Print uniform locations
    std::cout << "Material uniform locations:" << std::endl;
    std::cout << "  uUseBaseTex: " << matUniforms.uUseBaseTex << std::endl;
    std::cout << "  uBaseTex: " << matUniforms.uBaseTex << std::endl;
    std::cout << "  uNormalTex: " << matUniforms.uNormalTex << std::endl;
    std::cout << "  uUseNormalTex: " << matUniforms.uUseNormalTex << std::endl;

    // ---- Set initial Material and Lighting Values -----
    glUseProgram(shader_program); // Make sure we're using the right program
    
    // Now let's enable textures since basic rendering works
    glUniform1i(matUniforms.uUseBaseTex, 1);  // ENABLE base texture
    glUniform1i(matUniforms.uBaseTex, 0);
    glUniform3f(matUniforms.uBaseTint, 1.0f, 1.0f, 1.0f);  // White tint to show texture
    glUniform1f(matUniforms.uRoughness, 0.3f);  
    glUniform1f(matUniforms.uMetallic, 0.7f);
    glUniform3f(matUniforms.uDielectricF0, 0.04f, 0.04f, 0.04f);
    glUniform1i(matUniforms.uNormalTex, 1);
    glUniform1i(matUniforms.uUseNormalTex, 1);  // ENABLE normal mapping
    glUniform1i(matUniforms.uRoughnessMap, 2);
    glUniform1i(matUniforms.uUseRoughnessMap, 0);  // DISABLE for testing
    glUniform1i(matUniforms.uMetallicMap, 3);
    glUniform1i(matUniforms.uUseMetallicMap, 0);  // DISABLE for testing

    glUniform1i(lightUniforms.uLightType, 0);
    glUniform3f(lightUniforms.uLightColor, 3.0f, 3.0f, 3.0f);  // BRIGHTER light
    glUniform3f(lightUniforms.uAmbient, 0.2f, 0.2f, 0.2f);     // More ambient
    glUniform1f(lightUniforms.uSpotCosInner, cosf(glm::radians(15.0f)));
    glUniform1f(lightUniforms.uSpotCosOuter, cosf(glm::radians(25.0f)));
    glUniform3f(lightUniforms.uCamPos, 0.0f, 0.0f, 3.0f);

    glm::mat4 projection = glm::perspective(
        glm::radians(45.0f),
        (float)SCR_WIDTH / SCR_HEIGHT,
        0.1f,
        100.0f
    );
    glUniformMatrix4fv(vertUniforms.projectionMatrix, 1, GL_FALSE, glm::value_ptr(projection));

    // ---- Render loop ----
    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
    glEnable(GL_DEPTH_TEST);
    glDisable(GL_CULL_FACE);

    std::cout << "Starting render loop..." << std::endl;

    while (!glfwWindowShouldClose(window)) {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // ===== DRAW MAIN OBJECT FIRST =====
        glUseProgram(shader_program);
        
        // DEBUG: Verify current program
        GLint currentProgram;
        glGetIntegerv(GL_CURRENT_PROGRAM, &currentProgram);
        if (currentProgram != shader_program) {
            std::cout << "WARNING: Wrong shader program active!" << std::endl;
        }

        // Enable base color and normal map textures
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, baseColorTextureID);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, normalMapTextureID);
        
        // Enable roughness map
        glActiveTexture(GL_TEXTURE2);
        glBindTexture(GL_TEXTURE_2D, roughnessTextureID);
        
        // Enable IBL and irradiance map
        glActiveTexture(GL_TEXTURE4);
        glBindTexture(GL_TEXTURE_CUBE_MAP, irradianceMap);
        
        // Set IBL uniform
        glUniform1i(glGetUniformLocation(shader_program, "useIBL"), 1);
        glUniform1i(glGetUniformLocation(shader_program, "irradianceMap"), 4);

        // Animation Light
        float time = glfwGetTime();
        float elev = 0.15f + 0.65f * 0.5f * (1.0f + sin(time * 0.7f));
        glm::vec3 dir = glm::normalize(glm::vec3(0.0f, -cos(elev), sin(elev)));
        glUniform3f(lightUniforms.uDirDir, dir.x, dir.y, dir.z);

        // matrices - make the quad bigger for visibility
        glm::mat4 model = glm::scale(glm::mat4(1.0f), glm::vec3(2.0f)); // Make it 2x bigger
        model = glm::rotate(model, time, glm::vec3(0.0f, 1.0f, 0.0f));
        
        glm::mat4 view = glm::lookAt(
            glm::vec3(0.0f, 0.0f, 5.0f),  // Move camera further back
            glm::vec3(0.0f, 0.0f, 0.0f),
            glm::vec3(0.0f, 1.0f, 0.0f)
        );

        glUniformMatrix4fv(vertUniforms.modelMatrix, 1, GL_FALSE, glm::value_ptr(model));
        glUniformMatrix4fv(vertUniforms.viewMatrix, 1, GL_FALSE, glm::value_ptr(view));

        // DEBUG: Check if we're about to draw
        std::cout << "About to draw mesh. VAO: " << mesh.VAO << ", Index count: " << mesh.indexCount << std::endl;
        
        // Check OpenGL errors before drawing
        GLenum error = glGetError();
        if (error != GL_NO_ERROR) {
            std::cout << "OpenGL error before mesh.draw(): " << error << std::endl;
        }

        mesh.draw(); 
        
        // Check OpenGL errors after drawing
        error = glGetError();
        if (error != GL_NO_ERROR) {
            std::cout << "OpenGL error after mesh.draw(): " << error << std::endl;
        }

        // ===== DRAW SKYBOX LAST =====
        glm::mat4 R = glm::rotate(glm::mat4(1.0f), time * 0.25f, glm::vec3(0,1,0));
        R = glm::rotate(R, 0.3f * sin(time * 0.2f), glm::vec3(1,0,0));
        glm::mat4 viewSky = glm::mat4(glm::mat3(view * R));

        glDepthFunc(GL_LEQUAL);
        glUseProgram(sbProg);
        glUniformMatrix4fv(sbView, 1, GL_FALSE, glm::value_ptr(viewSky));
        glUniformMatrix4fv(sbProj, 1, GL_FALSE, glm::value_ptr(projection));
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_CUBE_MAP, envCubemap);
        renderCube();
        glDepthFunc(GL_LESS);

        glfwSwapBuffers(window);
        glfwPollEvents();
        
        // Only print this every 60 frames to avoid spam
        static int frameCount = 0;
        if (frameCount++ % 60 == 0) {
            std::cout << "Frame " << frameCount << " rendered" << std::endl;
        }
    }

    // ---- Clean up GPU resources -----
    glDeleteShader(vertex_shader);
    glDeleteShader(frag_shader);
    glDeleteTextures(1, &baseColorTextureID);
    glDeleteTextures(1, &normalMapTextureID);
    glDeleteTextures(1, &roughnessTextureID);
    glDeleteTextures(1, &metallicTextureID);
    glDeleteTextures(1, &hdrTextureID);
    glDeleteVertexArrays(1, &mesh.VAO);
    glfwTerminate();

    return 0;
}