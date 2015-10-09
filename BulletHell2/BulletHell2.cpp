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
	//run game "main loop" here.

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

