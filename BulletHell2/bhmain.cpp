#include <stdio.h>
#include <iostream>

#include "BulletHell2.hpp"

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


	BulletHell2 game;

	game.startup();
	game.run();
	game.shutdown();

	std::cin.get();
}