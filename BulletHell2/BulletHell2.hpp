#pragma once

#include "stdint.h"
#include <memory>

//forward decls only!
struct EntitySystemView;
class Window;

class BulletHellState
{
public:
   virtual ~BulletHellState() {}
   virtual void update() = 0;
};

//struct of forward decls, access what globals you want here
struct BulletHellContext
{
   EntitySystemView* world;
   Window* window;
   uint64_t currentTick;
   bool gameRunning;
   std::unique_ptr<BulletHellState> currentState;//game FSM
};


class BulletHell2
{
public:
	void startup();
	void run();
	void shutdown();
	void update();

	BulletHellContext* context;
};
