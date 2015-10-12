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

   void drawSpriteHACK(Vec2 const& bottomMiddle, Vec2 const& size);
   void drawSprite(float x, float y);

   bool isOpen();
private:
   class Impl;
   std::unique_ptr<Impl> pImpl;

   Sprite test_sprite;
};

