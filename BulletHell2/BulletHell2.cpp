//this will include pretty much everything, a central app coordination point

#include "BulletHell2.hpp"
#include "ManagerView.hpp"
#include "entity.hpp"
#include "components.hpp"
#include "NameIndex.hpp"
#include "constants.hpp"

//what's in a player?
//what's in a level?
//what's in an enemy? a bullet?
//DEFINE DATA HERE!



Entity createPlayer(BulletHellContext* ctxt)
{
	Entity out(ctxt->eSystem);
	entitySetName(ctxt, out, "player");
	out.create<PositionComponent>(10.0f, 10.0f);

	out.update();
	return out;
}

Entity createCamera(BulletHellContext* ctxt)
{
	Entity out(ctxt->eSystem);
	entitySetName(ctxt, out, "camera");
	out.create<PositionComponent>(constants::cameraSize.x/2, 0.0f);
	out.create<SizeComponent>(constants::cameraSize);
	out.create<CameraComponent>();

	out.update();
	return out;
}

void BulletHell2::startup()
{
	//game initialization, create everything here and hook it all up.
	context = new BulletHellContext;
	auto eSystem = context->eSystem = new EntitySystem;
	auto managers = context->managers = new ManagerView;
	managers->nameIndex = new NameIndex;

	//link things together that need to know about each other.
	eSystem->installListener(managers->nameIndex);
}
void BulletHell2::run()
{
	float target_fps = 60; //should be set as a constant later.
	float millis_per_frame = 1000/target_fps;
	
	LARGE_INTEGER StartingTime, EndingTime, ElapsedMiilliseconds;
	LARGE_INTEGER Frequency;
	
	QueryPerformanceFrequency(&Frequency); 
	
	QueryPerformanceCounter(&StartingTime);
	
	//run game "main loop" here.
	while(true)
	{
		QueryPerformanceCounter(&EndingTime);
		ElapsedMilliseconds.QuadPart = EndingTime.QuadPart - StartingTime.QuadPart;
		
		//
		// We now have the elapsed number of ticks, along with the
		// number of ticks-per-second. We use these values
		// to convert to the number of elapsed microseconds.
		// To guard against loss-of-precision, we convert
		// to microseconds before dividing by ticks-per-second.
		//
		
		ElapsedMilliseconds.QuadPart *= 1000;
		ElapsedMilliseconds.QuadPart /= Frequency.QuadPart;
		
		update((float)ElapsedMilliseconds.QuardPart/millis_per_frame);
		
		QueryPerformanceCounter(&StartingTime);
	}

	//we need an idea of a "level"
	auto ent = createPlayer(context);
	auto cam = createCamera(context);

	//auto player = entityGetByName(context, "player");
	//printf("player position (%f, %f)\n", player.get<PositionComponent>()->pos.x, player.get<PositionComponent>()->pos.y);

	ent.destroy();
	cam.destroy();
}
void BulletHell2::shutdown()
{
	//final thing called, do cleanup here.
	//kill esys before managers...
	delete context->eSystem;

	delete context->managers->nameIndex;
	delete context->managers;
	
	delete context;
}

//delta is the fraction of time relative to a single frame at your target framerate
//If the delta value is 0.5, half a frame of your target framerate is passed
//If the delta value is 2.0, two frames of your target framerate is passed
void BulletHell2::update(float delta)
{
	
}
