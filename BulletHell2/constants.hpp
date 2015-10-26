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
   extern const Vec2 UICameraSize; //entire UI - entire screen
   extern const Vec2 cameraSize;  //for playing field
   extern const float UIPanelWidth; //panel on right hand side of playing field.


   //PLAYER STATS
   extern const float playerSpeed;
   extern const float playerFocusedSpeed;

   //physics
   extern const int physicsTicksPerTick;
}
