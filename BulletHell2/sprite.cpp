#include "sprite.h"

#include "Functions.h"

Sprite::Sprite(std::string file_path)
{
	texture_handle = png_texture_load(file_path.c_str(), &width, &height);
}

void Sprite::DrawSprite(float x, float y)
{
	glBindTexture(GL_TEXTURE_2D, texture_handle);
}