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

#include "RunLevelState.hpp"

#include "audio.hpp"

#include "ShotType.hpp"


Entity createPlayer(BulletHellContext* ctxt)
{
	Entity out(ctxt->world);
	entitySetName(ctxt->world, out, "player");
	out.create<PositionComponent>(constants::cameraSize.x/2, 0.0f);
   out.create<RadiusComponent>(6.0f);
   out.create<PlayerComponent>();
   out.create<WorldBoundedComponent>();


   std::vector<Shot> shots;
   Shot shot1("PlayerShot");
   Shot shot2("PlayerShot");
   shot1.offset = Vec2(8.0f, 0.0f);
   shot2.offset = Vec2(-8.0f, 0.0f);
   shots.push_back(shot1);
   shots.push_back(shot2);
   out.create<ShotComponent>(std::move(shots));  


   out.create<SpriteComponent>("png/player_temp.png");
   
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


void startGame(BulletHellContext* ctxt)
{
   //for now, let's jump straight into a test level.
   Level level = testLevelCreate(ctxt);
   ctxt->currentState = runLevelStateCreate(ctxt, std::move(level));

   //we need a player to control in the level.

   createPlayer(ctxt);
}

void BulletHell2::startup()
{
	//game initialization, create everything here and hook it all up.

   //global systems
   timeStart();
  

   //context stuff
	context = new BulletHellContext;   


   //static data
   hackBuildTestShotTypes(context->shotTypes);


   //audio
   context->audio = new Audio;

   
   //game tick
   context->currentTick = 0;
   context->gameRunning = true;

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
      
   context->playingBGM = new Sound(Sound::createSong(*context->audio, "music/test.mp3"));
   context->playingBGM->play();

	//run game "main loop" here.
	while(context->window->isOpen() && context->gameRunning)
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
   
   

}
void BulletHell2::shutdown()
{
	//final thing called, do cleanup here.
	//kill esys before managers...
	delete context->world->system;
	delete context->world->nameIndex;
	delete context->world;
	
   delete context->window;

   delete context->audio;

	delete context;
}

void BulletHell2::update()
{
	//update.
   ++context->currentTick;


   //update the input...
   context->window->updateInput();

   //update the simulation state.
   if (!context->currentState) startGame(context);
   context->currentState->update();

}
