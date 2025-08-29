#pragma once
#include <string>
#include <glad/glad.h>

std::string ReadTextFile(const char* path);
GLuint CompileShader(GLenum type, const char* src);
GLuint LinkProgram(GLuint vs, GLuint fs);
GLint  ULoc(GLuint program, const char* name);  // glGetUniformLocation wrapper
