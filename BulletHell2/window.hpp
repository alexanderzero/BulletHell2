#pragma once

#include <string>
#include <memory>
#include "geometry.hpp"
#include "sprite.h"

struct WindowInit
{
   int width, height;
   std::string title;
};

class Window
{
public:
   Window(WindowInit const& initData);
   ~Window();

   void updateInput();
   
   void startDraw();
   void endDraw();

   void drawSprite(float x, float y, int flip_horizontal, int flip_vertical, float rotation);
   void drawSprite(float x, float y, int flip_horizontal, int flip_vertical, float rotation, Sprite* sprite);

   bool isOpen();
private:
   class Impl;
   std::unique_ptr<Impl> pImpl;

   Sprite test_sprite;
};

