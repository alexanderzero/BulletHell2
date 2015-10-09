#include <stdio.h>
#include <iostream>

#include "Functions.h"

//#include "glew\GL/glew.h"
//#include <gl/GL.h>
//#include "glfw_include/GLFW/glfw3.h"


int main(int argc, char** argv)
{
	if (!glfwInit())
	{
		std::cout << "GLFW init failed :<" << std::endl;
		std::cin.get();
		return 0;
	}

	//glfwEnable(GLFW_AUTO_POLL_EVENTS); /* No explicit call to glfwPollEvents() */

	//glfwOpenWindowHint(GLFW_WINDOW_NO_RESIZE, GL_TRUE);
	//glfwOpenWindowHint(GLFW_OPENGL_VERSION_MAJOR, 3);
	//glfwOpenWindowHint(GLFW_OPENGL_VERSION_MINOR, 2);
	//glfwOpenWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	//glfwOpenWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);

	GLFWwindow* window = glfwCreateWindow(640, 480, "Simple example", NULL, NULL);
	if (!window)
	{
		glfwTerminate();
		std::cout << "GLFW window failed :<" << std::endl;
		std::cin.get();
		return 0;
	}

	glfwMakeContextCurrent(window);

	GLenum err = glewInit();
	//if(GLEW_OK != err)
	//{
	//   std::cout << "It didnt work :<" << std::endl;
	//   std::cout << glewGetErrorString(err) << std::endl;
	//   std::cin.get();
	//   return 0;
	//}

	int major, minor;
	//getOpenGLVersion(major, minor);
	//std::cout << "Your openGL version is: " << major << "." << minor << std::endl;
	//std::cout << "boop!" << std::endl;
	//std::cin.get();

	glfwSwapInterval(1);
	//glfwSetKeyCallback(window, key_callback);

	//init();

	while (!glfwWindowShouldClose(window))
	{
		render(window);
	}

	glfwDestroyWindow(window);
	glfwTerminate();
	return 0;
}

void render(GLFWwindow* window)
{
	////
	glClear(GL_COLOR_BUFFER_BIT);

	//glRotatef((float)glfwGetTime() * 50.f, 0.f, 0.f, 1.f);
	glBegin(GL_TRIANGLES);
	glColor3f(1.f, 0.f, 0.f);
	glVertex3f(-0.6f, -0.4f, 0.f);
	glColor3f(0.f, 1.f, 0.f);
	glVertex3f(0.6f, -0.4f, 0.f);
	glColor3f(0.f, 0.f, 1.f);
	glVertex3f(0.f, 0.6f, 0.f);
	glEnd();
	glfwSwapBuffers(window);
}