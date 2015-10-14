#include "ShotType.hpp"

#include "EntitySystemView.hpp"
#include "entity.hpp"
#include "NameIndex.hpp"
#include "components.hpp"
#include "BulletHell2.hpp"
#include "random.hpp"


ShotType buildPlayerShotType(EntitySystemView* shotTypes)
{
   ShotType playerShot(shotTypes);

   entitySetName(shotTypes, playerShot, "PlayerShot");
   playerShot.create<CooldownComponent>(10);
   playerShot.create<VelocityComponent>(0.0f, 32.0f);
   playerShot.create<SpriteComponent>("png/needle.png");
   playerShot.update();

   return playerShot;
}

ShotType buildEnemyShotType(EntitySystemView* shotTypes)
{
   ShotType playerShot(shotTypes);

   entitySetName(shotTypes, playerShot, "EnemyShot");
   playerShot.create<CooldownRangeComponent>(10, 30);
   playerShot.create<VelocityComponent>(0.0f, 0.0f);
   playerShot.create<AccelerationComponent>(0.0f, -0.2f);
   playerShot.create<MaxSpeedComponent>(8.0f);
   playerShot.create<SpriteComponent>("png/babble_red.png");
   playerShot.update();

   return playerShot;
}



void hackBuildTestShotTypes(EntitySystemView*& shotTypes)
{
   shotTypes = new EntitySystemView;
   auto db = shotTypes->system = new EntitySystem;
   shotTypes->nameIndex = new NameIndex;


   buildPlayerShotType(shotTypes);
   buildEnemyShotType(shotTypes);
}


class ShotComponentImpl : public IShotType
{
public:
   virtual void fire(Entity ent, Shot* shot) override
   {
      //actual entity creation...  note later this might be a fan, or something completely different.  This a placeholder til we figure out requirements of the system a bit more, and also if we want to reuse it directly for enemies

      Entity bullet(ctxt->world);

      //centered on player for now.  more fire types later based on the shot type
      auto position = ent.get<PositionComponent>()->pos;
      

      if (shot)
      {
         position += shot->offset;
      }

      bullet.create<PositionComponent>(position);

      if (ent.get<PlayerComponent>())
         bullet.create<SizeComponent>(4.0f, 32.0f); //hacked, add graphical/collision data to shot type later.
      else 
         bullet.create<SizeComponent>(4.0f, 4.0f);

      bullet.create<DieOffscreenComponent>(); //don't leak!  note that some shots of enemies might not have this and be simply timed, or may have bounds expanded to allow for slight offscreen action

                                              //copy velocity component from shot type, if it exists
      if (auto vel = shotType.get<VelocityComponent>()) bullet.create<VelocityComponent>(*vel);
      if (auto spd = shotType.get<MaxSpeedComponent>()) bullet.create<MaxSpeedComponent>(*spd);
      if (auto acc = shotType.get<AccelerationComponent>()) bullet.create<AccelerationComponent>(*acc);
      
      //graphics
      if (auto spr = shotType.get<SpriteComponent>()) bullet.create<SpriteComponent>(*spr);

      if (ent.get<PlayerComponent>()) bullet.create<PlayerBulletComponent>(); //for indexing
      if (ent.get<EnemyComponent>()) bullet.create<EnemyBulletComponent>(); //for indexing

      bullet.update();
   }

   BulletHellContext* ctxt;
   ShotType shotType;
};


std::unordered_map<EntityID, IShotType*> g_shotType;

//TODO: this assumes only one context.
int getShotCooldown(ShotType shot)
{
   if (auto cooldown = shot.get<CooldownComponent>()) return cooldown->ticks;
   if (auto cooldown = shot.get<CooldownRangeComponent>()) return randomInt(cooldown->ticksMin, cooldown->ticksMax);   

   return 0;
}

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