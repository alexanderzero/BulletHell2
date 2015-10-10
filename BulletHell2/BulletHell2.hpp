#pragma once

#include "stdint.h"

//forward decls only!
struct EntitySystemView;
class Window;

//struct of forward decls, access what globals you want here
struct BulletHellContext
{
   EntitySystemView* world;
   Window* window;
   uint64_t currentTick;
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
