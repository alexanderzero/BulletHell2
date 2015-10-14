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
   bool focused, firing;

}; //is-a player

struct PlayerBulletComponent {};
struct EnemyComponent {}; //is-a enemy 
struct CameraComponent {}; //makes is-a camera.


struct WorldBoundedComponent {}; //forced to always be in the world bounds.
struct DieOffscreenComponent {}; //delete this entity if it's offscreen


//weapon-specific

struct CooldownComponent
{
   CooldownComponent() : ticks(0) {}
   CooldownComponent(int ticks_in) : ticks(ticks_in) {}

   int ticks;  //todo - partial ticks are legit for some weapons, be a bit nicer here
};


struct Shot
{
   Shot() : nextFireTime(0) {}
   Shot(std::string name) : type(std::move(name)), nextFireTime(0) {}
   std::string type;
   uint64_t nextFireTime;
};

struct ShotComponent
{
   ShotComponent() {}
   ShotComponent(Shot shot) { shots.push_back(shot); }
   ShotComponent(std::vector<Shot> shots_in) : shots(std::move(shots_in)) {}
   std::vector<Shot> shots;
};

struct MarkedForDeletionComponent {};