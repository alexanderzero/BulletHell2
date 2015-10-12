#pragma once

//components are all small...  it's easy to just define/ include all in this one file.
#include <string>
#include <vector>
#include <unordered_map>
#include "geometry.hpp"

struct NameComponent
{
	NameComponent() {}
	NameComponent(std::string name_in) : name(std::move(name_in)) {}
	std::string name;
};

struct PositionComponent
{
	PositionComponent() {}
	PositionComponent(Vec2 pos_in) : pos(pos_in) {}
	PositionComponent(float x, float y) : pos(Vec2(x,y)) {}
	Vec2 pos;
};


struct VelocityComponent
{
   VelocityComponent() {}
   VelocityComponent(Vec2 vel_in) : vel(vel_in) {}
   VelocityComponent(float x, float y) : vel(Vec2(x, y)) {}
   Vec2 vel;
};

struct SizeComponent
{
	SizeComponent() {}
	SizeComponent(Vec2 pos_in) : sz(pos_in) {}
	SizeComponent(float x, float y) : sz(Vec2(x, y)) {}
	Vec2 sz;
};


struct PlayerComponent {

   PlayerComponent()
   {
      memset(this, 0, sizeof(*this)); //clear initial flags to false.
   }

   //controls state.
   bool left, right, up, down;
   bool focused;

}; //is-a player
struct EnemyComponent {}; //is-a enemy 
struct CameraComponent {}; //makes is-a camera.


struct WorldBoundedComponent {}; //forced to always be in the world bounds.