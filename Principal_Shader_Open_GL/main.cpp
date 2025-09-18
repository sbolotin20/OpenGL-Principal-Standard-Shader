// main.cpp
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

    GLuint vertex_shader = CompileShader(GL_VERTEX_SHADER, vertexSource.c_str());
    GLuint frag_shader = CompileShader(GL_FRAGMENT_SHADER, fragSource.c_str());
    GLuint shader_program = LinkProgram(vertex_shader, frag_shader);

    glUseProgram(shader_program);

    

    // ----- Set up Vertex Data (Position + UV) -----
    Mesh mesh = createQuad();
    GLuint VAO = mesh.VAO;

    // ---- Load Texture -----
    baseColorTextureID = LoadTexture2D("textures/base_color.jpg");
    normalMapTextureID = LoadTexture2D("textures/normal_map.png");
    roughnessTextureID = LoadTexture2D("textures/roughness_map.png");
    
    hdrTextureID = LoadHDRTexture("textures/test.hdr"); // use .hdr
    GLuint envCubemap = EquirectToCubemap(hdrTextureID, 0, 0, 512);
    glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);
    // later bind envCubemap to your IBL shaders
    // compile skybox program once
    std::string sbVS = ReadTextFile("shaders/skybox.vert");
    std::string sbFS = ReadTextFile("shaders/skybox.frag");
    GLuint sbV = CompileShader(GL_VERTEX_SHADER, sbVS.c_str());
    GLuint sbF = CompileShader(GL_FRAGMENT_SHADER, sbFS.c_str());
    GLuint sbProg = LinkProgram(sbV, sbF);
    glUseProgram(sbProg);
    glUniform1i(glGetUniformLocation(sbProg, "env"), 0);
    GLint sbView = glGetUniformLocation(sbProg, "view");
    GLint sbProj = glGetUniformLocation(sbProg, "projection");

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    int width, height; glfwGetFramebufferSize(window, &width, &height);
    glViewport(0, 0, w, h);


    // ---- Material and Lighting Setup -----
    LightingUniforms lightUniforms = getLightingUniforms(shader_program);
    MaterialUniforms matUniforms = getMaterialUniforms(shader_program);
    VertexUniforms vertUniforms = getVertexUniforms(shader_program);

    // ---- Set initial Material and Lighting Values -----
    glUniform1i(matUniforms.uUseBaseTex, 1);
    glUniform1i(matUniforms.uBaseTex, 0);
    glUniform3f(matUniforms.uBaseTint, 1.0f, 1.0f, 1.0f);
    glUniform1f(matUniforms.uRoughness, 0.8f);
    glUniform1f(matUniforms.uMetallic, 0.0f);
    glUniform3f(matUniforms.uDielectricF0, 0.04f, 0.04f, 0.04f);
    glUniform1i(matUniforms.uNormalTex, 1); // use texture unit 1
    glUniform1i(matUniforms.uUseNormalTex, true); // toggle normal mapping on 
    glUniform1i(matUniforms.uRoughnessMap, 2);
    glUniform1i(matUniforms.uUseRoughnessMap, 1);
    glUniform1i(matUniforms.uMetallicMap, 3);
    glUniform1i(matUniforms.uUseMetallicMap, 0);

    glUniform1i(lightUniforms.uLightType, 0);
    glUniform3f(lightUniforms.uLightColor, 1.0f, 1.0f, 1.0f);
    glUniform3f(lightUniforms.uAmbient, 0.05f, 0.05f, 0.05f);
    glUniform1f(lightUniforms.uSpotCosInner, cosf(glm::radians(15.0f)));
    glUniform1f(lightUniforms.uSpotCosOuter, cosf(glm::radians(25.0f)));
    glUniform3f(lightUniforms.uCamPos, 0.0f, 0.0f, 1.0f);

    glm::mat4 projection = glm::perspective(
        glm::radians(45.0f),              // FieldOfView: 45° is natural, like human vision
        (float)SCR_WIDTH / SCR_HEIGHT,    // Aspect: Must match window or things stretch
        0.1f,                             // Near: Objects closer than 0.1 units get clipped
        100.0f                            // Far: Objects beyond 100 units disappear
    );
    glUniformMatrix4fv(vertUniforms.projectionMatrix, 1, GL_FALSE, glm::value_ptr(projection));

    // ---- Render loop ----
    glClearColor(0.1f, 0.1f, 0.1f, 1.0f); // dark gray background
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glEnable(GL_DEPTH_TEST);
    glDisable(GL_CULL_FACE);  // Disable culling so both sides of the quad are drawn


    while (!glfwWindowShouldClose(window)) {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glUseProgram(shader_program);

        // bind base color texture
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, baseColorTextureID);  // make sure you defined this

        // bind normal map texture
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, normalMapTextureID);

        // bind roughness map texture
        glActiveTexture(GL_TEXTURE2);
        glBindTexture(GL_TEXTURE_2D, roughnessTextureID);

         // bind metallic map texture
        glActiveTexture(GL_TEXTURE3);
        glBindTexture(GL_TEXTURE_2D, metallicTextureID);

        glUniform1i(glGetUniformLocation(shader_program, "useNormalMap"), true);



        // --- Animation Light ---
        // --- Animate Light Direction ---
        float time = glfwGetTime();
        float elev = 0.15f + 0.65f * 0.5f * (1.0f + sin(time * 0.7f));
        glm::vec3 dir = glm::normalize(glm::vec3(0.0f, -cos(elev), sin(elev)));
        glUniform3f(lightUniforms.uDirDir, dir.x, dir.y, dir.z);

        // matrices
        glm::mat4 model = glm::mat4(1.0f); // identity 
        model = glm::rotate(model, time, glm::vec3(0.0f, 1.0f, 0.0f)); // spin around Y
        glm::mat4 view = glm::lookAt(
            glm::vec3(0.0f, 0.0f, 3.0f),  // Eye: Camera 3 units back on Z axis
            glm::vec3(0.0f, 0.0f, 0.0f),  // Center: Looking at origin
            glm::vec3(0.0f, 1.0f, 0.0f)   // Up: Y-axis is "up" (standard)
        );

        glUniformMatrix4fv(vertUniforms.modelMatrix, 1, GL_FALSE, glm::value_ptr(model));
        glUniformMatrix4fv(vertUniforms.viewMatrix, 1, GL_FALSE, glm::value_ptr(view));

        glm::mat4 R = glm::rotate(glm::mat4(1.0f), time * 0.25f, glm::vec3(0,1,0));     // yaw
        R = glm::rotate(R, 0.3f * sin(time * 0.2f), glm::vec3(1,0,0));                   // gentle pitch
        glm::mat4 viewSky = glm::mat4(glm::mat3(view * R));                           // strip translation

        // draw skybox from envCubemap
        glDepthFunc(GL_LEQUAL);
        glUseProgram(sbProg);             // your existing view matrix
        glUniformMatrix4fv(sbView, 1, GL_FALSE, glm::value_ptr(viewSky));
        glUniformMatrix4fv(sbProj, 1, GL_FALSE, glm::value_ptr(projection));
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_CUBE_MAP, envCubemap);
        renderCube();
        glDepthFunc(GL_LESS);

        glfwGetFramebufferSize(window, &w, &h);
        glViewport(0, 0, w, h);

        mesh.draw(); 

        glfwSwapBuffers(window);
        glfwPollEvents();
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
