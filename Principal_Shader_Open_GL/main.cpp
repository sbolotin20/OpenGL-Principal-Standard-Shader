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

// ─────────────────────────────────────────────
// Main
int main() {
    std::cout << "OpenGL project is running!" << std::endl;
    std::cout << "Working directory: " << std::filesystem::current_path() << std::endl;

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
    Mesh mesh = createTriangle();
    GLuint VAO = mesh.VAO;

    // ---- Load Texture -----
    GLuint texture = LoadTexture2D("texture.png");

    // ---- Material and Lighting Setup -----
    // ---- Material and Lighting Setup -----
    LightingUniforms lightUniforms = getLightingUniforms(shader_program);
    MaterialUniforms matUniforms = getMaterialUniforms(shader_program);


    // ---- Set initial Material and Lighting Values -----
    glUniform1i(matUniforms.uUseBaseTex, 1);
    glUniform1i(matUniforms.uBaseTex, 0);
    glUniform3f(matUniforms.uBaseTint, 1.0f, 1.0f, 1.0f);
    glUniform1f(matUniforms.uRoughness, 0.4f);
    glUniform1f(matUniforms.uMetallic, 0.0f);
    glUniform3f(matUniforms.uDielectricF0, 0.04f, 0.04f, 0.04f);


    glUniform1i(lightUniforms.uLightType, 0);
    glUniform3f(lightUniforms.uLightColor, 1.0f, 1.0f, 1.0f);
    glUniform3f(lightUniforms.uAmbient, 0.05f, 0.05f, 0.05f);
    glUniform1f(lightUniforms.uSpotCosInner, cosf(glm::radians(15.0f)));
    glUniform1f(lightUniforms.uSpotCosOuter, cosf(glm::radians(25.0f)));
    glUniform3f(lightUniforms.uCamPos, 0.0f, 0.0f, 1.0f);

    // ---- Render loop ----
    glClearColor(0.1f, 0.1f, 0.1f, 1.0f); // dark gray background

    while (!glfwWindowShouldClose(window)) {
        glClear(GL_COLOR_BUFFER_BIT);

        glUseProgram(shader_program);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, texture);

        // --- Animation Light ---
        // --- Animate Light Direction ---
        double time = glfwGetTime();
        float elev = 0.15f + 0.65f * 0.5f * (1.0f + sin(time * 0.7f));
        glm::vec3 dir = glm::normalize(glm::vec3(0.0f, -cos(elev), sin(elev)));
        glUniform3f(lightUniforms.uDirDir, dir.x, dir.y, dir.z);

        glBindVertexArray(VAO); // bind the VAO (it remembers VBO + attributes)
        glDrawArrays(GL_TRIANGLES, 0, 3); // draw 3 vertices as one triangle

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // ---- Clean up GPU resources -----
    glDeleteShader(vertex_shader);
    glDeleteShader(frag_shader);
    glDeleteTextures(1, &texture);
    glDeleteVertexArrays(1, &VAO);
    glfwTerminate();

    return 0;
}
