#include "shader_utils.h"
#include <fstream>
#include <sstream>
#include <iostream>

std::string ReadTextFile(const char* shader_file) {
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

GLuint CompileShader(GLenum type, const char* src) {
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

GLuint LinkProgram(GLuint vertex_shader, GLuint frag_shader) {
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

GLint  ULoc(GLuint program, const char* name) {  // glGetUniformLocation wrapper
    GLint loc = glGetUniformLocation(program, name);
    if (loc == -1) std::cerr << "Warning: uniform not found: " << name << "\n";
    return loc;
}
