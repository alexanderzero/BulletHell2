#pragma once

#include <string>
#include <memory>

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
   void update();
   bool isOpen();
private:
   class Impl;
   std::unique_ptr<Impl> pImpl;
};

