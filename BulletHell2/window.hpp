#pragma once

#include <string>
#include <memory>
#include "geometry.hpp"

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

   bool isOpen();
private:
   class Impl;
   std::unique_ptr<Impl> pImpl;
};

