#pragma once

#include "geometry.hpp"
#include "Time.hpp"

namespace constants
{
   //window stuff
   extern const int defaultResolutionWidth;
   extern const int defaultResolutionHeight;
   extern const char* windowTitle;
   
   //FPS
   extern const int targetFPS;
   extern const microseconds microsecondsPerFrame;
   
   //UI
   extern const Vec2 cameraSize;
}
