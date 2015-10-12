#pragma once

#include <string>

#include <GL/glew.h>
#include <gl/GL.h>


class Sprite
{
public:
	int width, height;
	GLuint texture_handle;

	//Sprite(std::string file_path);
	Sprite();

	void SetFile(std::string file_path);
	//void DrawSprite(float x, float y);

private:
	std::string path_name;


};
