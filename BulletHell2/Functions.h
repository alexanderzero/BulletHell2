#pragma once

#include <GL/glew.h>
#include <gl/GL.h>

#include <GLFW/glfw3.h>

#include <string>

void getOpenGLVersion(int& major, int& minor);
void read_shaders(std::string& vert_shader, std::string& frag_shader);
void render(GLFWwindow* window);
void init();
GLuint png_texture_load(const char * file_name, int * width, int * height);