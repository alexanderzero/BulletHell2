//this will include pretty much everything, a central app coordination point

#include "BulletHell2.hpp"
#include "ManagerView.hpp"
#include "entity.hpp"
#include "components.hpp"
#include "NameIndex.hpp"
#include "constants.hpp"

#include "Time.hpp"
#include "EntitySystemView.hpp"

#include "window.hpp"


//what's in a player?
//what's in a level?
//what's in an enemy? a bullet?
//DEFINE DATA HERE!




EntitySystem* eSystem;

Entity createPlayer(BulletHellContext* ctxt)
{
	Entity out(ctxt->world);
	entitySetName(ctxt->world, out, "player");
	out.create<PositionComponent>(10.0f, 10.0f);

	out.update();
	return out;
}

Entity createCamera(BulletHellContext* ctxt)
{
	Entity out(ctxt->world);
	entitySetName(ctxt->world, out, "camera");
	out.create<PositionComponent>(constants::cameraSize.x/2, 0.0f);
	out.create<SizeComponent>(constants::cameraSize);
	out.create<CameraComponent>();

	out.update();
	return out;
}

void BulletHell2::startup()
{
	//game initialization, create everything here and hook it all up.

   //global systems
   timeStart();
  

   //context stuff
	context = new BulletHellContext;   


   //game tick
   context->currentTick = 0;

   //game window
   WindowInit windowInitData;
   windowInitData.width = constants::defaultResolutionWidth;
   windowInitData.height = constants::defaultResolutionHeight;
   windowInitData.title = constants::windowTitle;
   context->window = new Window(windowInitData);

   //entity system stuff
	auto eSystemView = context->world = new EntitySystemView;
   auto eSystem = eSystemView->system = new EntitySystem;
   eSystemView->nameIndex = new NameIndex;

	//link things together that need to know about each other.
   eSystem->installListener(eSystemView->nameIndex);
}
void BulletHell2::run()
{
   auto prevTime = timeCurrentMicroseconds();


   //we need an idea of a "level"
   auto ent = createPlayer(context);
   auto cam = createCamera(context);

	
	//run game "main loop" here.
	while(context->window->isOpen())
	{
      auto currentTime = timeCurrentMicroseconds();
      auto elapsedTime = currentTime - prevTime;
				
      if (elapsedTime >= constants::microsecondsPerFrame)
      {
         prevTime = currentTime;
         update();
      }
      else if (constants::microsecondsPerFrame - elapsedTime > 3000)
      {
         timeSleep();
      }
	}
   
	//auto player = entityGetByName(context, "player");
	//printf("player position (%f, %f)\n", player.get<PositionComponent>()->pos.x, player.get<PositionComponent>()->pos.y);

	ent.destroy();
	cam.destroy();
}
void BulletHell2::shutdown()
{
	//final thing called, do cleanup here.
	//kill esys before managers...
	delete context->world->system;
	delete context->world->nameIndex;
	delete context->world;
	
   delete context->window;

	delete context;
}

void BulletHell2::update()
{
	//update.
   ++context->currentTick;
   context->window->update();
}
