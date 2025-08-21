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


#define STB_IMAGE_IMPLEMENTATION
#include "External/stb_image.h"

// ─────────────────────────────────────────────
// Window Settings
// ─────
const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;

// ─────────────────────────────────────────────
// Helper Functions: Load Shader Files and Compile Shaders
// 


static std::string read_shader_file(const char* shader_file)
{
    std::ifstream file(shader_file);
    if (!file.is_open()) {
        std::cerr << "Cannot open file: " << shader_file << std::endl;
        return "";
    }

    std::stringstream buffer;
    buffer << file.rdbuf();
    file.close();
    return buffer.str();

}
GLuint compile_shader(GLenum type, const char *src) {
    GLuint shader = glCreateShader(type);
    glShaderSource(shader, 1, &src, nullptr);
    glCompileShader(shader);

    GLint success;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        GLint length;
        glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &length);
        if (length == 0) {
            std::cout << "Failed to compile shader. No info log" << std::endl;
        }
        else {
            std::string info_log(length, 0);
            glGetShaderInfoLog(shader, length, nullptr, info_log.data());
            std::cout << "Failed to compile shader. Info log:\n" << info_log << std::endl;
        }
    }
    return shader; 
}

GLuint create_shader_program(GLuint vertex_shader, GLuint frag_shader) {
    GLuint program = glCreateProgram();
    glAttachShader(program, vertex_shader);
    glAttachShader(program, frag_shader);
    glLinkProgram(program);
    GLint success;
    glGetProgramiv(program, GL_LINK_STATUS, &success);
    if (!success) {
        GLint length;
        glGetProgramiv(program, GL_INFO_LOG_LENGTH, &length);
        if (length == 0) {
            std::cout << "Failed to link program. No info log" << std::endl;
        }
        else {
            std::string info_log(length, 0);
            glGetProgramInfoLog(program, length, nullptr, info_log.data());
            std::cout << "Failed to link program. Info log:\n" << info_log << std::endl;
        }
    }
    std::cout << "Successfully linked shader program!" << std::endl;
    return program;
}

glm::vec3 normalize_vec(float x, float y, float z) {
    float len = sqrtf(x*x + y*y + z*z);
    if (len > 0.0f) {
        x /= len;
        y /= len;
        z /= len;
    } else {
        // fallback if someone passes (0,0,0)
        x = 0.0f; y = 0.0f; z = 1.0f;
    }
    return glm::vec3(x, y, z);
}


// ─────────────────────────────────────────────
// Main
//
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
    std::string vertexSource = read_shader_file("shaders/basic.vert");
    std::string fragSource = read_shader_file("shaders/basic.frag");

    GLuint vertex_shader = compile_shader(GL_VERTEX_SHADER, vertexSource.c_str());
    GLuint frag_shader = compile_shader(GL_FRAGMENT_SHADER, fragSource.c_str());
    GLuint shader_program = create_shader_program(vertex_shader, frag_shader);

    // Set sampler2D uniform to use texture unit 0
    glUseProgram(shader_program);

    // ----- Set up Vertex Data (Position + UV) -----
    float vertices[] = {
        // positions         // texture coords
        -0.5f, -0.5f, 0.0f,  0.0f, 0.0f,
         0.5f, -0.5f, 0.0f,  1.0f, 0.0f,
         0.0f,  0.5f, 0.0f,  0.5f, 1.0f
    };

    GLuint VAO; // Vertex Array Object: blueprint of how OpenGL should handle vertex data later in rendering
    GLuint VBO; // Vertex Buffer Object: holds actual vertex data (like triangle positions)
    glGenVertexArrays(1, &VAO); // generate 1 VAO
    glGenBuffers(1, &VBO); // create a buffer ID

    glBindVertexArray(VAO);  // bind it (make it active)
    glBindBuffer(GL_ARRAY_BUFFER, VBO); // bind the buffer (target = array buffer)
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    // position attribute (location = 0)
    glVertexAttribPointer(
        0,                // index (matches "layout (location = 0)" in shader)
        3,                // size (vec3 = 3 floats)
        GL_FLOAT,         // type
        GL_FALSE,         // normalize?
        5 * sizeof(float),// stride (distance between sets of attributes - there are 5 floats per vertex)
        (void*)0          // offset (start at beginning of array)
    );
    glEnableVertexAttribArray(0);  // enable that vertex attribute

    // texture coordinate attribute (location = 1)
    glVertexAttribPointer(
        1,                // index (matches "layout (location = 1)" in shader)
        2,                // size (vec2 = 2 floats)
        GL_FLOAT,         // type
        GL_FALSE,         // normalize?
        5 * sizeof(float),// stride (distance between sets of attributes)
        (void*)(3 * sizeof(float))          // offset (start at beginning of array - skips past the position attributes)
    );
    glEnableVertexAttribArray(1);  // enable that vertex attribute

    // ---- Texture/material uniforms -----
    int uMat_useBaseColorTex = glGetUniformLocation(shader_program, "useBaseColorTex");
    int uMat_baseColorTint = glGetUniformLocation(shader_program, "baseColorTint");
    int uMat_baseColorTex = glGetUniformLocation(shader_program, "baseColorTex");
    glUniform1i(uMat_useBaseColorTex, 1);     
    glUniform3f(uMat_baseColorTint, 1.0, 0.0, 0.0);
    glUniform1i(uMat_baseColorTex, 0);

    // ---- Lighting uniforms ----

    // 1. choose test light direction in WORLD space (surface -> light)
    glm::vec3 normal_vec = normalize_vec(1.0f, 1.0f, 0.0f);

    // 3. cache uniform lighting location uniforms
    int uLight_Direction = glGetUniformLocation(shader_program, "uLight_Direction");
    int uLight_Color = glGetUniformLocation(shader_program, "uLight_Color");
    int uAmbient = glGetUniformLocation(shader_program, "uAmbient");
    int uLight_Position = glGetUniformLocation(shader_program, "uLight_Position");
    int uCamera_Position = glGetUniformLocation(shader_program, "uCamera_Position");
    int uSpecularColor = glGetUniformLocation(shader_program, "uSpecularColor");
    int uRoughness = glGetUniformLocation(shader_program, "uRoughness");
    int uMetallic = glGetUniformLocation(shader_program, "uMetallic");
    int uDielectricF0 = glGetUniformLocation(shader_program, "uDielectricF0");
    glUniform3f(uLight_Direction, normal_vec.x, normal_vec.y, normal_vec.z); // normalized direction    
    glUniform3f(uLight_Color, 1.0, 1.0, 1.0); // white light for testing
    glUniform3f(uAmbient, 0.05, 0.05, 0.05); // subtle global fill
    glUniform3f(uLight_Position, 0.8, 0.8, 0.7); // light position
    glUniform3f(uCamera_Position, 0.0, 0.0, 1.0); // since triangle sits at z=0 and faces +z
    glUniform3f(uSpecularColor, 1.0, 1.0, 1.0); 
    glUniform1f(uRoughness, 0.2);
    glUniform1f(uMetallic, 0.6);
    glUniform3f(uDielectricF0, 0.4, 0.4, 0.4); 

    // ---- Load Texture -----
    stbi_set_flip_vertically_on_load(true);

    int width, height, nrChannels; // nrChannels = number of color channels 
    unsigned char* data = stbi_load("texture.png", &width, &height, &nrChannels, 0); // stores pixel data in row-major order
    if (!data) {
        std::cout << "Failed to load texture!" << std::endl;
    }

    // Upload texture to GPU
    GLuint texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    
    // define how OpenGL samples the texture when UVs are scaled or go out of bounds (texture sampling and wrapping behavior)
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    
    GLenum format = GL_RGB;
    if (nrChannels == 4)
        format = GL_RGBA;
    
    glTexImage2D(
        GL_TEXTURE_2D,      // texture target
        0,                  // mipmap level (0 = base level)
        format,             // internal format on GPU
        width, height,      // width/height
        0,                  // border (must be 0)
        format,             // format of source data (must match above)
        GL_UNSIGNED_BYTE,   // data type of pixels
        data                // pointer to image data (from stbi_load)
    );
    glGenerateMipmap(GL_TEXTURE_2D); // smaller versions of image when texture is far away
    
    stbi_image_free(data); // free CPU-side memeory (on GPU now)

    // ---- Render loop ----
    glClearColor(0.1f, 0.1f, 0.1f, 1.0f); // dark gray background

    while (!glfwWindowShouldClose(window)) {
        glClear(GL_COLOR_BUFFER_BIT);     // clear the screen

        glUseProgram(shader_program);     // activate your shader
        glActiveTexture(GL_TEXTURE0); // activate texture unit 0
        glBindTexture(GL_TEXTURE_2D, texture); // bind texture to it
        glUniform3f(uMat_baseColorTint, 0.0, 1.0, 0.0);

        glBindVertexArray(VAO);           // bind the VAO (it remembers VBO + attributes)

        // animate light
        double time = glfwGetTime();
        float radius = 0.4f;
        float height = 0.15f;
        glUniform3f(uLight_Position, radius * cos(time * 0.7), radius * sin(time * 0.7), height); // normalized direction 

        glDrawArrays(GL_TRIANGLES, 0, 3); // draw 3 vertices as one triangle

        glfwSwapBuffers(window);          // swap buffers
        glfwPollEvents();                 // handle input/window events
    }

    // ---- Clean up GPU resources -----
    glDeleteShader(vertex_shader);
    glDeleteShader(frag_shader);
    glDeleteTextures(1, &texture);
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glfwTerminate();

    return 0;
}

