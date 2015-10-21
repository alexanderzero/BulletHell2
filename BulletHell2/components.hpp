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

struct AccelerationComponent
{
   AccelerationComponent() {}
   AccelerationComponent(Vec2 accel_in) : accel(accel_in) {}
   AccelerationComponent(float x, float y) : accel(Vec2(x, y)) {}
   Vec2 accel;
};

struct MaxSpeedComponent
{
   MaxSpeedComponent() {}
   MaxSpeedComponent(float speed) : maxSpeed(speed) {}
   float maxSpeed;
};

struct SizeComponent
{
	SizeComponent() {}
	SizeComponent(Vec2 pos_in) : sz(pos_in) {}
	SizeComponent(float x, float y) : sz(Vec2(x, y)) {}
	Vec2 sz;
};

struct RadiusComponent
{
   RadiusComponent() {}
   RadiusComponent(float r) : radius(r) {}
   float radius;
};


//infinite-length laser
struct LaserComponent
{
   LaserComponent() {}
   LaserComponent(Vec2 direction_in, float width_in) : direction(direction_in), width(width_in) {}
   Vec2 direction;
   float width;
};

//typical laser with a startup - full width - ending cycle.
//useless without a laser component.
//auto-deletes if after end tick.
struct ResizingLaserComponent
{
	uint64_t startTick;
	uint64_t fullWidthTick;
	uint64_t startFadeTick;
	uint64_t endTick;
	float maxWidth;
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


struct SpriteComponent
{
   SpriteComponent() {}
   SpriteComponent(std::string sprite_in) : sprite(std::move(sprite_in)) {}
   std::string sprite;
};

struct NoDamageComponent {}; //bullets that don't deal damage yet.  for example, laser startup.

struct EnemyBulletComponent {};
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

struct CooldownRangeComponent
{
   CooldownRangeComponent() : ticksMin(0), ticksMax(0) {}
   CooldownRangeComponent(int ticksMin_in, int ticksMax_in) : ticksMin(ticksMin_in), ticksMax(ticksMax_in) {}

   int ticksMin, ticksMax;  //min to max, inclusive
};


struct Shot
{
   Shot() : nextFireTime(0) {}
   Shot(std::string name) : type(std::move(name)), nextFireTime(0) {}
   std::string type;
   Vec2 offset; //physical offset on the character to fire this from.
   float angleOffset = 0.0f; //kind of a hack... sometimes used.

   //HACK - 
   //this should probably be data created with the shot, or something.
   //I'm not sure where to put it so I'm putting it here for now
   uint64_t tickOffset = 0;

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


//health/enemy related

struct InvinciblityComponent {};
struct LivesWithNoHPComponent {};

struct HealthComponent {
   HealthComponent() : hp(1) {}
   HealthComponent(int hp_in) : hp(hp_in) {}
   int hp; //currently just number of hits required to die.  TODO: add an actual damage system, when we have multiple player shottypes.
};