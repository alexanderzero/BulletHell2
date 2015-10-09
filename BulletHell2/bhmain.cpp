#include <stdio.h>
#include <iostream>

#include "glfw_include/GLFW/glfw3.h"

int main(int argc, char** argv)
{
	if (!glfwInit())
	{
		std::cout << "GLFW init failed :<" << std::endl;
		std::cin.get();
		return 0;
	}

	printf("hello bullet hell world!\n");

	std::cin.get();
}