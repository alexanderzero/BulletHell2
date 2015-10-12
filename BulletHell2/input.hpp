#pragma once

#include <vector>

//translating GLFW events -> game events.

enum class KeyPressType
{
   Down, Up, Repeated
};

//for special types.
struct KeyType
{
   enum e_KeyType
   {
      //ArrowKeys
      Left = 256,
      Right,
      Up,
      Down,

      //shift
      LeftShift, RightShift,
   };
};


typedef KeyType::e_KeyType e_KeyType;

//keycode format:
//uses upper case for keys 'A' - 'Z' (standard ascii)
//numbers are '0'-'9'(standard ascii)
//standard ascii punctuation also matches up, eg `~!@#$%^&*()()-_+=\|[]{} etc.
//anything "special" is extended codes, provided by the KeyType enum (C++98 style enum for conversion to and from int)

struct KeyEvent
{
   int key;  
   KeyPressType type;
};


//TODO - this should probably be fleshed out a bit better, but this is a simple hacked wrapper for now.
//input gets sent here, then it can be flushed... simple event queue.
void inputPushEvent(KeyEvent event);
std::vector<KeyEvent> inputFlushQueue();


