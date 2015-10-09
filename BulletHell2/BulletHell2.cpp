//this will include pretty much everything, a central app coordination point

#include "BulletHell2.hpp"
#include "ManagerView.hpp"
#include "entity.hpp"
#include "components.hpp"

Entity createPlayer(BulletHellContext* ctxt)
{
	Entity out(ctxt->eSystem);
	out.create<NameComponent>("player");
	out.create<PositionComponent>(0.0f, 0.0f);
	return out;
}

void BulletHell2::startup()
{
	//game initialization, create everything here and hook it all up.
	context = new BulletHellContext;
	context->managers = new ManagerView;
	context->eSystem = new EntitySystem;

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
	delete context->eSystem;
	delete context->managers;
	delete context;
}

