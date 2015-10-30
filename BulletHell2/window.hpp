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

   void clear();

   void targetViewport(Vec2 min, Vec2 max);

   void drawSpriteStretched(float x, float y, float width, float height, int flip_horizontal, int flip_vertical, float rotation, Sprite* sprite);
   void drawSprite(float x, float y, int flip_horizontal, int flip_vertical, float rotation);
   void drawSprite(float x, float y, int flip_horizontal, int flip_vertical, float rotation, Sprite* sprite);
   void drawSprite3D(Sprite* sprite);

   bool isOpen();
private:
   class Impl;
   std::unique_ptr<Impl> pImpl;

   Sprite test_sprite;
};

