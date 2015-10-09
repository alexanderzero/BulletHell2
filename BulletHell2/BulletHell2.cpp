//this will include pretty much everything, a central app coordination point

#include "BulletHell2.hpp"
#include "ManagerView.hpp"
#include "entity.hpp"
#include "components.hpp"
#include "NameIndex.hpp"

Entity createPlayer(BulletHellContext* ctxt)
{
	Entity out(ctxt->eSystem);
	entitySetName(out, "Player");
	out.create<PositionComponent>(0.0f, 0.0f);


	return out;
}

Entity createCamera(BulletHellContext* ctxt)
{
	Entity out(ctxt->eSystem);
	entitySetName(out, "camera");
	out.create<PositionComponent>(0.0f, 0.0f);
	out.create<SizeComponent>();

	return out;
}

void BulletHell2::startup()
{
	//game initialization, create everything here and hook it all up.
	context = new BulletHellContext;
	context->eSystem = new EntitySystem;
	context->managers = new ManagerView;
	context->managers->nameIndex = new NameIndex;

	//link things together that need to know about each other.
	
}
void BulletHell2::run()
{
	//run game "main loop" here.

	//we need an idea of a "level"

	auto ent = createPlayer(context);

}
void BulletHell2::shutdown()
{
	//final thing called, do cleanup here.
	delete context->managers->nameIndex;
	delete context->managers;
	delete context->eSystem;
	delete context;
}

