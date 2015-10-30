#pragma once

#include <GL/glew.h>
#include <gl/GL.h>

#include <GLFW/glfw3.h>

#include <string>

void getOpenGLVersion(int& major, int& minor);
std::string read_shader(std::string file_path);
void read_shaders(std::string& vert_shader, std::string& frag_shader);
void render(GLFWwindow* window);
void init();
GLuint LoadShader(std::string vertex_path, std::string fragment_path);
GLuint png_texture_load(const char * file_name, int * width, int * height);