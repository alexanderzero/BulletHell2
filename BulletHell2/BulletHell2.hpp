#pragma once

//forward decls only!
class EntitySystem;
struct ManagerView;

//struct of forward decls, access what globals you want here
struct BulletHellContext
{
	EntitySystem* eSystem;
	ManagerView* managers;
};

class BulletHell2
{
public:
	void startup();
	void run();
	void shutdown();

	BulletHellContext* context;
};
