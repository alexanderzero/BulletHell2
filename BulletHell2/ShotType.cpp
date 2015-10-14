#include "ShotType.hpp"

#include "EntitySystemView.hpp"
#include "entity.hpp"
#include "NameIndex.hpp"
#include "components.hpp"
#include "BulletHell2.hpp"


ShotType buildPlayerShotType(EntitySystemView* shotTypes)
{
   ShotType playerShot(shotTypes);


   entitySetName(shotTypes, playerShot, "PlayerShot");
   playerShot.create<CooldownComponent>(10);
   playerShot.create<VelocityComponent>(0.0f, 32.0f);
   playerShot.update();

   return playerShot;
}

void hackBuildTestShotTypes(EntitySystemView*& shotTypes)
{
   shotTypes = new EntitySystemView;
   auto db = shotTypes->system = new EntitySystem;
   shotTypes->nameIndex = new NameIndex;


   buildPlayerShotType(shotTypes);
}


class ShotComponentImpl : public IShotType
{
public:
   virtual void fire(Entity ent) override
   {
      //actual entity creation...  note later this might be a fan, or something completely different.  This a placeholder til we figure out requirements of the system a bit more, and also if we want to reuse it directly for enemies

      Entity bullet(ctxt->world);

      //centered on player for now.  more fire types later based on the shot type
      bullet.create<PositionComponent>(*ent.get<PositionComponent>());

      bullet.create<SizeComponent>(4.0f, 32.0f); //hacked, add graphical/collision data to shot type later.

      bullet.create<DieOffscreenComponent>(); //don't leak!  note that some shots of enemies might not have this and be simply timed, or may have bounds expanded to allow for slight offscreen action

                                              //copy velocity component from shot type, if it exists
      if (auto vel = shotType.get<VelocityComponent>()) bullet.create<VelocityComponent>(*vel);

      bullet.create<PlayerBulletComponent>(); //for indexing

      bullet.update();
   }

   BulletHellContext* ctxt;
   ShotType shotType;
};


std::unordered_map<EntityID, IShotType*> g_shotType;

//TODO: this assumes only one context.
IShotType* getShotType(BulletHellContext* context, ShotType shotType)
{
   auto shotID = shotType.getID();
   auto iter = g_shotType.find(shotID);
   if (iter != end(g_shotType)) return iter->second;
   auto out = new ShotComponentImpl;
   out->shotType = shotType;
   out->ctxt = context;
   g_shotType.insert(std::make_pair(shotID, out));
   return out;
}