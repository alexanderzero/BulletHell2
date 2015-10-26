#include "constants.hpp"





//window stuff
const int constants::defaultResolutionWidth = 1600 ;//simple 16x9
const int constants::defaultResolutionHeight = 900; 
const char* constants::windowTitle = "Bullet Hell 2";

//FPS
const int constants::targetFPS = 60;
const microseconds constants::microsecondsPerFrame = 1000000 / constants::targetFPS;

//UI
const Vec2 constants::UICameraSize(1920.0f, 1080.0f);
const float constants::UIPanelWidth = 450; 
const Vec2 constants::cameraSize(UICameraSize.x-UIPanelWidth, UICameraSize.y);


//PLAYER STATS
const float constants::playerSpeed = 6.0f;
const float constants::playerFocusedSpeed = 3.0f;


const int constants::physicsTicksPerTick = 4;