#include <fstream>
#include <sstream>
#include <string>
#include <iostream>
#include <filesystem>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

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
    glViewport(0,0,SCR_WIDTH,SCR_HEIGHT);

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

    // shader uniforms
    int useBaseColorTex = glGetUniformLocation(shader_program, "useBaseColorTex");
    int baseColorTint = glGetUniformLocation(shader_program, "baseColorTint");
    int baseColorTex = glGetUniformLocation(shader_program, "baseColorTex");
    glUniform1i(useBaseColorTex, 1);     
    glUniform3f(baseColorTint, 0.1, 0.2, 0.7);
    glUniform1i(baseColorTex, 0);
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
        

        glBindVertexArray(VAO);           // bind the VAO (it remembers VBO + attributes)

        glDrawArrays(GL_TRIANGLES, 0, 3); // draw 3 vertices as one triangle

        glfwSwapBuffers(window);          // swap buffers
        glfwPollEvents();                 // handle input/window events
    }

    // ---- Clean up GPU resources -----
    glDeleteShader(vertex_shader);
    glDeleteShader(frag_shader);
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glfwTerminate();

    return 0;
}

