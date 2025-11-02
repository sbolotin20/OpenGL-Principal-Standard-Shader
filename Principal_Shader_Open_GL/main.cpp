// main.cpp - Complete PBR Shader with ImGui Controls
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

// IMGUI
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include "ImGuiFileDialog.h"
using IGFD::FileDialogConfig;   // keep this once, near your includes


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

static void Reload2D(GLuint &tex, const std::string& path) {
    if (tex) glDeleteTextures(1, &tex);
    tex = LoadTexture2D(path);
}
static void ReloadHDR(GLuint &hdrTex, GLuint &envCubemap, GLuint &irradianceMap, const std::string& path) {
    if (hdrTex) glDeleteTextures(1, &hdrTex);
    hdrTex = LoadHDRTexture(path);
    if (envCubemap) glDeleteTextures(1, &envCubemap);
    envCubemap = EquirectToCubemap(hdrTex, 0, 0, 512);
    glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);
    if (irradianceMap) glDeleteTextures(1, &irradianceMap);
    irradianceMap = ConvolveIrradiance(envCubemap);
}


// ─────────────────────────────────────────────
// Main
int main() {
    std::cout << "OpenGL PBR Project Starting..." << std::endl;
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

    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "PBR Shader Tool", NULL, NULL);
    if (window == NULL) {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    
    // ----- Load OpenGL functions with GLAD -----
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cerr << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    int w, h;
    glfwGetFramebufferSize(window, &w, &h);
    glViewport(0, 0, w, h);

    // ----- Initialize ImGui -----
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;

    // ----- Initialize L2DFileDialog

    // Setup Platform/Renderer bindings
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 330");

    // Setup Style
    ImGui::StyleColorsDark();

    // ----- Compile and Link Shaders ------
    std::string vertexSource = ReadTextFile("shaders/basic.vert");
    std::string fragSource = ReadTextFile("shaders/basic.frag");

    std::cout << "Vertex shader source length: " << vertexSource.length() << std::endl;
    std::cout << "Fragment shader source length: " << fragSource.length() << std::endl;

    GLuint vertex_shader = CompileShader(GL_VERTEX_SHADER, vertexSource.c_str());
    GLuint frag_shader = CompileShader(GL_FRAGMENT_SHADER, fragSource.c_str());
    GLuint shader_program = LinkProgram(vertex_shader, frag_shader);

    // Check shader program status
    GLint success;
    glGetProgramiv(shader_program, GL_LINK_STATUS, &success);
    if (!success) {
        char infoLog[512];
        glGetProgramInfoLog(shader_program, 512, NULL, infoLog);
        std::cout << "SHADER LINKING FAILED: " << infoLog << std::endl;
    } else {
        std::cout << "Main shader program linked successfully!" << std::endl;
    }

    // ----- Set up Cube Geometry -----
    Mesh mesh = createCube();
    std::cout << "Cube mesh created - VAO: " << mesh.VAO << ", Index count: " << mesh.indexCount << std::endl;

    // ---- Load Textures -----
    baseColorTextureID = LoadTexture2D("textures/base_color.jpg");
    normalMapTextureID = LoadTexture2D("textures/normal_map.png");
    roughnessTextureID = LoadTexture2D("textures/roughness_map.png");

    // baseColorTextureID = LoadTexture2D("textures/Metal_color.png");
    // normalMapTextureID = LoadTexture2D("textures/Metal_NormalGL.png");
    // roughnessTextureID = LoadTexture2D("textures/Metal_Roughness.png");
    // metallicTextureID = LoadTexture2D("textures/Metal_Metalness.png");
    
    std::cout << "Texture IDs - Base: " << baseColorTextureID 
              << ", Normal: " << normalMapTextureID 
              << ", Roughness: " << roughnessTextureID << std::endl;
    
    hdrTextureID = LoadHDRTexture("textures/test.hdr");
    std::cout << "HDR texture ID: " << hdrTextureID << std::endl;
    
    GLuint envCubemap = EquirectToCubemap(hdrTextureID, 0, 0, 512);
    glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);
    GLuint irradianceMap = ConvolveIrradiance(envCubemap);
    
    std::cout << "Environment cubemap ID: " << envCubemap << ", Irradiance map ID: " << irradianceMap << std::endl;

    // ----- Compile Skybox Shaders -----
    std::string sbVS = ReadTextFile("shaders/skybox.vert");
    std::string sbFS = ReadTextFile("shaders/skybox.frag");
    GLuint sbV = CompileShader(GL_VERTEX_SHADER, sbVS.c_str());
    GLuint sbF = CompileShader(GL_FRAGMENT_SHADER, sbFS.c_str());
    GLuint sbProg = LinkProgram(sbV, sbF);
    
    glGetProgramiv(sbProg, GL_LINK_STATUS, &success);
    if (!success) {
        char infoLog[512];
        glGetProgramInfoLog(sbProg, 512, NULL, infoLog);
        std::cout << "SKYBOX SHADER LINKING FAILED: " << infoLog << std::endl;
    } else {
        std::cout << "Skybox shader linked successfully!" << std::endl;
    }
    
    glUseProgram(sbProg);
    glUniform1i(glGetUniformLocation(sbProg, "env"), 0);
    GLint sbView = glGetUniformLocation(sbProg, "view");
    GLint sbProj = glGetUniformLocation(sbProg, "projection");

    // ----- Get Uniform Locations -----
    LightingUniforms lightUniforms = getLightingUniforms(shader_program);
    MaterialUniforms matUniforms = getMaterialUniforms(shader_program);
    VertexUniforms vertUniforms = getVertexUniforms(shader_program);

    // ----- ImGui Control Variables -----
    static float roughness = 0.8f;
    static float metallic = 0.0f;
    static float baseTintColor[3] = {1.0f, 1.0f, 1.0f};
    static float lightDir[3] = {0.0f, -0.7f, 0.3f};
    static float lightColor[3] = {1.0f, 1.0f, 1.0f};
    static float lightIntensity = 3.0f;
    static bool useBaseColorTex = true;
    static bool useNormalMap = true;
    static bool useRoughnessMap = true;
    static bool useIBL = true;
    static float exposure = 1.0f;
    static int currentToneMapping = 0;

    // ----- Set Initial Uniform Values -----
    glUseProgram(shader_program);
    
    glUniform1i(matUniforms.uUseBaseTex, useBaseColorTex ? 1 : 0);
    glUniform1i(matUniforms.uBaseTex, 0);
    glUniform3f(matUniforms.uBaseTint, baseTintColor[0], baseTintColor[1], baseTintColor[2]);
    glUniform1f(matUniforms.uRoughness, roughness);  
    glUniform1f(matUniforms.uMetallic, metallic);
    glUniform3f(matUniforms.uDielectricF0, 0.04f, 0.04f, 0.04f);
    glUniform1i(matUniforms.uNormalTex, 1);
    glUniform1i(matUniforms.uUseNormalTex, useNormalMap ? 1 : 0);
    glUniform1i(matUniforms.uRoughnessMap, 2);
    glUniform1i(matUniforms.uUseRoughnessMap, useRoughnessMap ? 1 : 0);
    glUniform1i(matUniforms.uMetallicMap, 3);
    glUniform1i(matUniforms.uUseMetallicMap, 1);

    glUniform1i(lightUniforms.uLightType, 0);
    glUniform3f(lightUniforms.uLightColor, lightColor[0] * lightIntensity, lightColor[1] * lightIntensity, lightColor[2] * lightIntensity);
    glUniform3f(lightUniforms.uAmbient, 0.1f, 0.1f, 0.1f);
    glUniform1f(lightUniforms.uSpotCosInner, cosf(glm::radians(15.0f)));
    glUniform1f(lightUniforms.uSpotCosOuter, cosf(glm::radians(25.0f)));
    glUniform3f(lightUniforms.uCamPos, 0.0f, 0.0f, 5.0f);

    // Set projection matrix
    glm::mat4 projection = glm::perspective(
        glm::radians(45.0f),
        (float)SCR_WIDTH / SCR_HEIGHT,
        0.1f,
        100.0f
    );
    glUniformMatrix4fv(vertUniforms.projectionMatrix, 1, GL_FALSE, glm::value_ptr(projection));

    // ----- Render Settings -----
    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
    glEnable(GL_DEPTH_TEST);
    glDisable(GL_CULL_FACE);

    std::cout << "Starting render loop..." << std::endl;

    // ===== MAIN RENDER LOOP =====
    while (!glfwWindowShouldClose(window)) {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // ----- Start ImGui Frame -----
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();



        // ----- ImGui Controls -----
        ImGui::Begin("PBR Material Controls");

        ImGui::Separator();
        ImGui::Text("Load Texture Maps");
        // --- File pickers ---
        if (ImGui::Button("Load Base Color")) {
            FileDialogConfig cfg; 
            cfg.path = ".";                   // start folder
            cfg.countSelectionMax = 1;
            cfg.flags = ImGuiFileDialogFlags_Modal;
            ImGuiFileDialog::Instance()->OpenDialog(
                "PickBase", "Choose Base Color",
                "Image files{.png,.jpg,.jpeg,.bmp,.tga}", cfg);
        }

        if (ImGui::Button("Load Normal")) {
            FileDialogConfig cfg; cfg.path = "."; cfg.countSelectionMax = 1; cfg.flags = ImGuiFileDialogFlags_Modal;
            ImGuiFileDialog::Instance()->OpenDialog(
                "PickNormal", "Choose Normal Map",
                "Image files{.png,.jpg,.jpeg,.bmp,.tga}", cfg);
        }

        if (ImGui::Button("Load Roughness")) {
            FileDialogConfig cfg; cfg.path = "."; cfg.countSelectionMax = 1; cfg.flags = ImGuiFileDialogFlags_Modal;
            ImGuiFileDialog::Instance()->OpenDialog(
                "PickMetallic", "Choose Roughness Map",
                "Image files{.png,.jpg,.jpeg,.bmp,.tga}", cfg);
        }

        if (ImGui::Button("Load Metallic")) {
            FileDialogConfig cfg; cfg.path = "."; cfg.countSelectionMax = 1; cfg.flags = ImGuiFileDialogFlags_Modal;
            ImGuiFileDialog::Instance()->OpenDialog(
                "PickRough", "Choose Metallic Map",
                "Image files{.png,.jpg,.jpeg,.bmp,.tga}", cfg);
        }

        // --- Handle results ---
        if (ImGuiFileDialog::Instance()->Display("PickBase")) {
            if (ImGuiFileDialog::Instance()->IsOk()) {
                std::string path = ImGuiFileDialog::Instance()->GetFilePathName();
                Reload2D(baseColorTextureID, path);
            }
            ImGuiFileDialog::Instance()->Close();
        }
        if (ImGuiFileDialog::Instance()->Display("PickNormal")) {
            if (ImGuiFileDialog::Instance()->IsOk()) {
                std::string path = ImGuiFileDialog::Instance()->GetFilePathName();
                Reload2D(normalMapTextureID, path);
            }
            ImGuiFileDialog::Instance()->Close();
        }
        if (ImGuiFileDialog::Instance()->Display("PickRough")) {
            if (ImGuiFileDialog::Instance()->IsOk()) {
                std::string path = ImGuiFileDialog::Instance()->GetFilePathName();
                Reload2D(roughnessTextureID, path);
            }
            ImGuiFileDialog::Instance()->Close();
        }
        if (ImGuiFileDialog::Instance()->Display("PickMetallic")) {
            if (ImGuiFileDialog::Instance()->IsOk()) {
                std::string path = ImGuiFileDialog::Instance()->GetFilePathName();
                Reload2D(metallicTextureID, path);
            }
            ImGuiFileDialog::Instance()->Close();
        }

        ImGui::Separator();
        if (ImGui::Checkbox("Use Base Color Texture", &useBaseColorTex)) {
            glUseProgram(shader_program);
            glUniform1i(matUniforms.uUseBaseTex, useBaseColorTex ? 1 : 0);
        }
        if (ImGui::Checkbox("Use Normal Map", &useNormalMap)) {
            glUseProgram(shader_program);
            glUniform1i(matUniforms.uUseNormalTex, useNormalMap ? 1 : 0);
        }
        if (ImGui::Checkbox("Use Roughness Map", &useRoughnessMap)) {
            glUseProgram(shader_program);
            glUniform1i(matUniforms.uUseRoughnessMap, useRoughnessMap ? 1 : 0);
        }
        // if (ImGui::Checkbox("Use Metallic Map", &usem)) {
        //     glUseProgram(shader_program);
        //     glUniform1i(matUniforms.uUseRoughnessMap, useRoughnessMap ? 1 : 0);
        // }
        if (ImGui::Checkbox("Use IBL", &useIBL)) {
            glUseProgram(shader_program);
            glUniform1i(glGetUniformLocation(shader_program, "useIBL"), useIBL ? 1 : 0);
        }



        ImGui::Text("Material Properties");
        if (ImGui::SliderFloat("Roughness", &roughness, 0.0f, 1.0f)) {
            glUseProgram(shader_program);
            glUniform1f(matUniforms.uRoughness, roughness);
        }
        if (ImGui::SliderFloat("Metallic", &metallic, 0.0f, 1.0f)) {
            glUseProgram(shader_program);
            glUniform1f(matUniforms.uMetallic, metallic);
        }
        if (ImGui::ColorEdit3("Base Tint", baseTintColor)) {
            glUseProgram(shader_program);
            glUniform3f(matUniforms.uBaseTint, baseTintColor[0], baseTintColor[1], baseTintColor[2]);
        }

        ImGui::Separator();
        ImGui::Text("Lighting");
        if (ImGui::SliderFloat3("Light Direction", lightDir, -1.0f, 1.0f)) {
            glm::vec3 dir = glm::normalize(glm::vec3(lightDir[0], lightDir[1], lightDir[2]));
            glUseProgram(shader_program);
            glUniform3f(lightUniforms.uDirDir, dir.x, dir.y, dir.z);
        }
        if (ImGui::ColorEdit3("Light Color", lightColor)) {
            glUseProgram(shader_program);
            glUniform3f(lightUniforms.uLightColor, lightColor[0] * lightIntensity, lightColor[1] * lightIntensity, lightColor[2] * lightIntensity);
        }
        if (ImGui::SliderFloat("Light Intensity", &lightIntensity, 0.0f, 10.0f)) {
            glUseProgram(shader_program);
            glUniform3f(lightUniforms.uLightColor, lightColor[0] * lightIntensity, lightColor[1] * lightIntensity, lightColor[2] * lightIntensity);
        }

        
        ImGui::End();

        // ----- Render Main Object -----
        // Make sure viewport is correct for 3D rendering
        glfwGetFramebufferSize(window, &w, &h);
        glViewport(0, 0, w, h);
        
        glUseProgram(shader_program);

        // Bind textures
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, baseColorTextureID);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, normalMapTextureID);
        glActiveTexture(GL_TEXTURE2);
        glBindTexture(GL_TEXTURE_2D, roughnessTextureID);
        glActiveTexture(GL_TEXTURE4);
        glBindTexture(GL_TEXTURE_CUBE_MAP, irradianceMap);
        
        // Set IBL uniforms
        glUniform1i(glGetUniformLocation(shader_program, "useIBL"), useIBL ? 1 : 0);
        glUniform1i(glGetUniformLocation(shader_program, "irradianceMap"), 4);

        // Update time-based lighting
        float time = glfwGetTime();
        float elev = 0.15f + 0.65f * 0.5f * (1.0f + sin(time * 0.7f));
        glm::vec3 animatedDir = glm::normalize(glm::vec3(0.0f, -cos(elev), sin(elev)));
        
        // Use manual light direction if being controlled, otherwise use animated
        if (lightDir[0] == 0.0f && lightDir[1] == -0.7f && lightDir[2] == 0.3f) {
            glUniform3f(lightUniforms.uDirDir, animatedDir.x, animatedDir.y, animatedDir.z);
        }

        // Update matrices
        glm::mat4 model = glm::scale(glm::mat4(1.0f), glm::vec3(2.0f));
        model = glm::rotate(model, time * 0.5f, glm::vec3(0.0f, 1.0f, 0.0f));
        
        glm::mat4 view = glm::lookAt(
            glm::vec3(0.0f, 0.0f, 5.0f),
            glm::vec3(0.0f, 0.0f, 0.0f),
            glm::vec3(0.0f, 1.0f, 0.0f)
        );

        glUniformMatrix4fv(vertUniforms.modelMatrix, 1, GL_FALSE, glm::value_ptr(model));
        glUniformMatrix4fv(vertUniforms.viewMatrix, 1, GL_FALSE, glm::value_ptr(view));

        // Draw the cube
        mesh.draw();

        // ----- Render Skybox -----
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

        // ----- Render ImGui -----
        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // ----- Cleanup -----
    glDeleteShader(vertex_shader);
    glDeleteShader(frag_shader);
    glDeleteProgram(shader_program);
    glDeleteProgram(sbProg);
    glDeleteTextures(1, &baseColorTextureID);
    glDeleteTextures(1, &normalMapTextureID);
    glDeleteTextures(1, &roughnessTextureID);
    glDeleteTextures(1, &metallicTextureID);
    glDeleteTextures(1, &hdrTextureID);
    glDeleteTextures(1, &envCubemap);
    glDeleteTextures(1, &irradianceMap);
    mesh.cleanup();
    
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    
    glfwTerminate();
    return 0;
}