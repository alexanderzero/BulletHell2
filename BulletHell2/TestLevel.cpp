//HACKHACK - for testing/prototype/debug purposes.

#include "RunLevelState.hpp"
#include "entity.hpp"
#include "EntitySystemView.hpp"
#include "components.hpp"


//creates a couple enemies which fly from offscreen with a bit of a delay
class CreateSomeTestEnemies : public LevelSection
{
public:
   CreateSomeTestEnemies(BulletHellContext* ctxt_in)
      : ctxt(ctxt_in)
   {

   }
   virtual void onEnter() override
   {
      startTick = ctxt->currentTick;
      nSpawned = 0;
   }

   void spawnEnemies()
   {
      //spawn these near to the top of the screen, heading in an X pattern with a velocity slightly towards the player.
      //they won't shoot for now.

      auto& eSys = ctxt->world->system;
      for (int i = 0; i < 2; ++i)
      {
         bool left = i != 0;

         Entity e(eSys);

         float xOffset = 100.0f;
         float xVel = 16.0f;

         //todo: better constant handling...
         e.create<PositionComponent>(left ? -xOffset : 1920 + xOffset, 800.0f);
         e.create<VelocityComponent>(left ? xVel : -xVel, xVel / -8);

         e.create<EnemyComponent>();
         e.create<SizeComponent>(64.0f, 64.0f);
         
         e.create<ShotComponent>(Shot("EnemyShot"));

         e.update();
      }
   }

   virtual void onUpdate() override
   {
      auto tick = ctxt->currentTick - startTick;
      if (nSpawned != enemyCount && !(tick % enemyTickGap))
      {
         spawnEnemies();
         ++nSpawned;
      }
   }

   virtual bool finished() override
   {
      auto tick = ctxt->currentTick - startTick;
      return tick >= totalTicks;
   }

   static const int enemyCount = 30;
   static const int enemyTickGap = 20; //20 ticks per enemy spawn

   static const int totalTicks = enemyCount * enemyTickGap + 240; //some buffer time.
  

   int nSpawned;
   BulletHellContext* ctxt;
   uint64_t startTick;
};

class WaitSection : public LevelSection
{
public:
   WaitSection(BulletHellContext* ctxt_in, uint64_t nTicks)
      : ctxt(ctxt_in), tickCount(nTicks)
   {
   }
   virtual void onEnter() override
   {
      lastTick = ctxt->currentTick + tickCount;
   }

   
   virtual void onUpdate() override
   {
   }

   virtual bool finished() override
   {
      return ctxt->currentTick >= lastTick;
   }
   BulletHellContext* ctxt;
   uint64_t tickCount, lastTick;
};

Level testLevelCreate(BulletHellContext* context)
{
   Level out;

   out.sections.push_back(std::make_unique<CreateSomeTestEnemies>(context));
   out.sections.push_back(std::make_unique<WaitSection>(context, 60 * 60)); //1 minute of waiting

   return out;
}