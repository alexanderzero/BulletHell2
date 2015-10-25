//HACKHACK - for testing/prototype/debug purposes.

#include "RunLevelState.hpp"
#include "entity.hpp"
#include "EntitySystemView.hpp"
#include "components.hpp"
#include "constants.hpp"
#include <functional>
#include "NameIndex.hpp"

#include "audio.hpp"

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


static const Vec2 RAN_START = Vec2(constants::cameraSize.x / 2, constants::cameraSize.y - 100);
static const Vec2 RAN_OFFSET = Vec2(200, -200);
static const int RAN_FLY_IN_FRAMES = 30;

Entity createRanYakumo(BulletHellContext* ctxt)
{
   Entity out(ctxt->world);


   //spawn her a bit to the left from center and off the screen.  She "flies in" as the first thing before she starts attacking.
   Vec2 pos = RAN_START;
   pos -= RAN_OFFSET;

   out.create<PositionComponent>(pos);

   out.create<EnemyComponent>();
   out.create<SizeComponent>(64.0f, 64.0f);

   //most importantly....
   out.create<SpriteComponent>("png/stolenRan.png");

   return out;
}

struct SubsectionState
{
   enum e_SubsectionState
   {
      StillProcessing,
      Done
   };
};


typedef SubsectionState::e_SubsectionState e_SubsectionState;
typedef std::function<e_SubsectionState(bool /*first run*/)> LevelSubsection;

class LevelStateQueue
{
public:
   void push(LevelSubsection subsection)
   {
      subsections.push_back(std::move(subsection));
   }
   void update()
   {
      while (!isFinished())
      {
         bool run = isFirstRun;
         isFirstRun = false;
         if (subsections[idx](run) == SubsectionState::StillProcessing) return;
         isFirstRun = true;
         ++idx;
      }
   }
   bool isFinished()
   {
      return idx == subsections.size();
   }

private:
   bool isFirstRun = true;
   size_t idx=0;
   std::vector<LevelSubsection> subsections;
};

bool isAlive(Entity ent)
{
   auto hp = ent.get<HealthComponent>();
   if (!hp) return true;
   return hp->hp > 0;
}

void chasePlayerHorizontally(BulletHellContext* ctxt, Entity ent, uint64_t& nextChaseTime, uint64_t& doneChasingFrame, int chaseFrames, int chaseCooldown)
{
   //handle chasing.
   if (ctxt->currentTick >= nextChaseTime)
   {
      //we want to edge in the general direction of the enemy, moving only horizontally.
      //PCB has slight vertical movements as well, but I don't really care to emulate it perfectly.

      auto playerPos = entityGetByName(ctxt->world, "player").get<PositionComponent>()->pos;
      bool isRight = playerPos.x >= ent.get<PositionComponent>()->pos.x;


      ent.create<AccelerationComponent>(Vec2(isRight ? 0.25f : -0.25f, 0.0f));
      ent.create<VelocityComponent>(Vec2(0.0f, 0.0f));
      ent.create<MaxSpeedComponent>(4);

      doneChasingFrame = ctxt->currentTick + chaseFrames;
      nextChaseTime = ctxt->currentTick + chaseCooldown;
   }
   else if (ctxt->currentTick >= doneChasingFrame)
   {
      ent.remove<MaxSpeedComponent>();
      ent.remove<VelocityComponent>();
      ent.remove<AccelerationComponent>();
      doneChasingFrame = nextChaseTime;
   }

}

//moves to something invincibly for an in-between state
LevelSubsection moveToSubsection(BulletHellContext* ctxt, Entity ent, Vec2 target, float maxSpeed)
{
   uint64_t endFrame;
   return [=](bool isFirstRun) mutable -> e_SubsectionState
   {
      if (isFirstRun)
      {
         //ignore damage during this time.
         ent.create<InvinciblityComponent>();

         //move forward towards the give spot.

         auto offset = target;
         offset -= ent.get<PositionComponent>()->pos;
         float offlen = length(offset);
         int frames = (int) (offlen / maxSpeed);
         
         if (frames > 0)
         {
            offset *= 1.0f / frames;
            ent.create<VelocityComponent>(offset);
         }

         endFrame = ctxt->currentTick + frames;
      }

      if (ctxt->currentTick >= endFrame)
      {
         ent.remove<VelocityComponent>();
         ent.remove<InvinciblityComponent>();
         return SubsectionState::Done;
      }

      return  SubsectionState::StillProcessing;
   };
}


class RanYakumoFromTouhouYouyoumuSection : public LevelSection
{
public:
   RanYakumoFromTouhouYouyoumuSection(BulletHellContext* ctxt_in)
      : ctxt(ctxt_in)
   {

   }

   virtual void onEnter() override
   {
      //spawn ran!   
      auto ran = createRanYakumo(ctxt);

      //ROCKING TUNES OR GTFO
      //playBGM("music/boss.mp3");
      playBGM("music/bsong1.mp3");


      //okay, we have to do a bunch of ordered things.  This state queue lets us pause and resume code, in an easy fashion.  we use pointers to capture state from this section into subsections.
      ran.create<HealthComponent>(80);

      /* //template below
      //--------DESCRIPTION----------
      { //open scope for let-over-lambda.  C++ == LISP now
         stateQueue.push([=](bool isFirstRun) mutable -> e_SubsectionState
         {
            if (isFirstRun)
            {

            }

            return SubsectionState::Done;
         });
      }
      */

      //--------RAN FLIES IN----------
      stateQueue.push(moveToSubsection(ctxt, ran, RAN_START, 8.0f));



      //-----STREAMING CAGE-----
      {
         int HEALTH = 160;
         int chaseCooldown = 100;
         int chaseFrames = 10;
         uint64_t nextChaseTime = ctxt->currentTick + chaseCooldown;
         uint64_t doneChasingFrame = 0;
         uint64_t freezeFamilarTick = 80;
         stateQueue.push([=](bool isFirstRun) mutable -> e_SubsectionState
         {
            if (isFirstRun)
            {
               freezeFamilarTick += ctxt->currentTick;
               ran.create<LivesWithNoHPComponent>();  //don't kill her when she dies
               ran.create<HealthComponent>(HEALTH); //todo: tweak, show a healthbar somehow....

               std::vector<Shot> shots;
               shots.push_back(Shot("StreamingCageFamiliar"));
               shots.push_back(Shot("StreamingCageCage"));
               shots.back().nextFireTime = ctxt->currentTick + 100;
               shots.push_back(Shot("StreamingCageStream"));
               shots.back().nextFireTime = ctxt->currentTick + 81;
               ran.create<ShotComponent>(std::move(shots));
            }

            if (ctxt->currentTick == freezeFamilarTick)
            {
               for (auto shot : ctxt->world->system->entitiesWithComponent<EnemyBulletComponent>()) shot.remove<VelocityComponent>();
            }

            if (isAlive(ran))
            {
               chasePlayerHorizontally(ctxt, ran, nextChaseTime, doneChasingFrame, chaseFrames, chaseCooldown);
               return SubsectionState::StillProcessing;
            }


            for (auto shot : ctxt->world->system->entitiesWithComponent<EnemyBulletComponent>()) shot.destroy();

            //clean-up movement when dead.
            ran.remove<MaxSpeedComponent>();
            ran.remove<VelocityComponent>();
            ran.remove<AccelerationComponent>();

            //no more shoots
            ran.remove<ShotComponent>();

            return  SubsectionState::Done;
         });
      }


      //-----RECENTER-------
      stateQueue.push(moveToSubsection(ctxt, ran, RAN_START, 8.0f));

      //-----BOMBS AWAY-----
      {
         int HEALTH = 150;
         int chaseCooldown = 300;
         int chaseFrames = 45;
         uint64_t nextChaseTime = ctxt->currentTick + chaseCooldown;
         uint64_t doneChasingFrame = 0;
         stateQueue.push([=](bool isFirstRun) mutable -> e_SubsectionState
         {
            if (isFirstRun)
            {
               ran.create<LivesWithNoHPComponent>();  //don't kill her when she dies
               ran.create<HealthComponent>(HEALTH); //todo: tweak, show a healthbar somehow....

               std::vector<Shot> shots;
               shots.push_back(Shot("BombsAway"));
               ran.create<ShotComponent>(std::move(shots));

               //aaaand now we're playing!
            }

            if (isAlive(ran))
            {
               chasePlayerHorizontally(ctxt, ran, nextChaseTime, doneChasingFrame, chaseFrames, chaseCooldown);
               return SubsectionState::StillProcessing;
            }


            for (auto shot : ctxt->world->system->entitiesWithComponent<EnemyBulletComponent>()) shot.destroy();

            //clean-up movement when dead.
            ran.remove<MaxSpeedComponent>();
            ran.remove<VelocityComponent>();
            ran.remove<AccelerationComponent>();

            //no more shoots
            ran.remove<ShotComponent>();

            return  SubsectionState::Done;
         });
      }
      

      //-----RECENTER-------
      stateQueue.push(moveToSubsection(ctxt, ran, RAN_START, 8.0f));


      //-----CURVY HELL-----
      {
         int HEALTH = 300;  //high health because you get LOTS of free hits
         int chaseCooldown = 300;
         int chaseFrames = 20;
         int startRotateTick = 120;
         float anglePerFrame = degToRad(0.1f);
         stateQueue.push([=](bool isFirstRun) mutable -> e_SubsectionState
         {
            if (isFirstRun)
            {
               ran.create<LivesWithNoHPComponent>();  //don't kill her when she dies
               ran.create<HealthComponent>(HEALTH); //todo: tweak, show a healthbar somehow....

               std::vector<Shot> shots;
               shots.push_back(Shot("CurvyBulletRing"));
               shots.push_back(Shot("CurvyBulletBottomFire"));
               shots.back().nextFireTime = ctxt->currentTick + 200;
               ran.create<ShotComponent>(std::move(shots));

               //aaaand now we're playing!
            }

            auto tick = ctxt->currentTick;
            for (auto&& bullet : ctxt->world->system->entitiesWithComponent<CurvyRingTagComponent>())
            {
               if (tick == bullet.get<SpawnTimeComponent>()->spawnTick + startRotateTick)
               {
                  //use linear arc interpolation here...
                  auto vel = bullet.get<VelocityComponent>()->vel;
                  bullet.remove<VelocityComponent>();
                  ArcInterpolationComponent arc;
                  arc.startTick = tick;
                  int maxTicks = 1000;
                  arc.endTick = tick + maxTicks;
                  float speed;
                  arc.startAngle = rectToPolar(vel, &speed);
                  float offset = anglePerFrame;
                  if (bullet.get<RotateLeftComponent>()) offset = -offset;
                  arc.center = ran.get<PositionComponent>()->pos;
                  arc.endAngle = arc.startAngle + offset * maxTicks;
                  arc.startDistance = distance(bullet.get<PositionComponent>()->pos, arc.center);
                  arc.endDistance = arc.startDistance + speed * maxTicks;
                  arc.cosineInterp = false;
                  bullet.set(arc);
               }
            }

            if (isAlive(ran))
            {
               return SubsectionState::StillProcessing;
            }

            for (auto shot : ctxt->world->system->entitiesWithComponent<EnemyBulletComponent>()) shot.destroy();

            //clean-up movement when dead.
            ran.remove<MaxSpeedComponent>();
            ran.remove<VelocityComponent>();
            ran.remove<AccelerationComponent>();

            //no more shoots
            ran.remove<ShotComponent>();

            return  SubsectionState::Done;
         });
      }

      //-----LASER BULLETS-----
      {
         int HEALTH = 200;
         int startTick;
         int firstSwitchTick;
         int switchTickTime;
         int interpFrames;
         int lastSwitch;
         int bulletCount = 20;
         int startTickFrames = 240;
         stateQueue.push([=](bool isFirstRun) mutable -> e_SubsectionState
         {
            if (isFirstRun)
            {
               startTick = ctxt->currentTick;
               firstSwitchTick = startTick + startTickFrames;
               interpFrames = 320;
               switchTickTime = startTickFrames + interpFrames;
               lastSwitch = 0;
               ran.create<LivesWithNoHPComponent>();  //don't kill her when she dies
               ran.create<HealthComponent>(HEALTH);

               std::vector<Shot> shots;
               shots.push_back(Shot("NueLaserBulletRing"));
               shots.push_back(Shot("NueLaserBulletRing"));
               shots.back().angleOffset = 360.0f / (bulletCount * 2);
               shots.back().nextFireTime = ctxt->currentTick + 15;

               ran.create<ShotComponent>(std::move(shots));
            }
            
            auto tick = ctxt->currentTick;
            if (tick == firstSwitchTick || (tick > firstSwitchTick && (tick - firstSwitchTick) % switchTickTime == 0))
            {
               lastSwitch = tick;
               ran.get<ShotComponent>()->shots.clear();
               for (auto&& bullet : ctxt->world->system->entitiesWithComponent<EnemyBulletComponent>())
               {
                  bool rotatesLeft = bullet.get<RotateLeftComponent>() != nullptr;

                  float angleDiff = (TAU / bulletCount) * 2;
                  if (rotatesLeft) angleDiff = -angleDiff;

                  //STOP!
                  bullet.remove<VelocityComponent>();

                  //graphical update.
                  bullet.create<SpriteComponent>("png/fireball_1.png");
                  bullet.remove<AlignToVelocityComponent>();

                  //ok, so the bullets now need to rotate.  we need to circularly interpolate.
                  //each bullet will go two whole sides over.
                  //so interpolation angle = (TAU / count) * 2;
                  //interpolation distance = (distance from enemy)
                  
                  //arc interpolation component!
                  ArcInterpolationComponent arc;
                  arc.center = ran.get<PositionComponent>()->pos;
                  arc.startDistance = arc.endDistance = distance(bullet.get<PositionComponent>()->pos, arc.center);
                  arc.startAngle = angleBetween(arc.center, bullet.get<PositionComponent>()->pos);
                  arc.endAngle = arc.startAngle + angleDiff;
                  arc.startTick = tick + 15;
                  arc.endTick = tick + interpFrames;
                  bullet.set(arc);
               }
            }
            if (tick > firstSwitchTick && (tick == lastSwitch + interpFrames))
            {
               std::vector<Shot> shots;
               shots.push_back(Shot("NueLaserBulletRing"));
               shots.push_back(Shot("NueLaserBulletRing"));
               shots.back().angleOffset = 360.0f / (bulletCount * 2);
               shots.back().nextFireTime = ctxt->currentTick + 15;

               ran.create<ShotComponent>(std::move(shots));

               for (auto&& bullet : ctxt->world->system->entitiesWithComponent<EnemyBulletComponent>())
               {
                  bullet.remove<ArcInterpolationComponent>();
                  bullet.create<VelocityComponent>(polarToRect(angleBetween(ran.get<PositionComponent>()->pos, bullet.get<PositionComponent>()->pos), 5.0f));
                  //bullet.create<SpriteComponent>("png/lozange.png");
                  bullet.create<AlignToVelocityComponent>();
               }
            }

            if (isAlive(ran))
            {
               return SubsectionState::StillProcessing;
            }

            //clean-up movement when dead.
            ran.remove<MaxSpeedComponent>();
            ran.remove<VelocityComponent>();
            ran.remove<AccelerationComponent>();

            //no more shoots
            ran.remove<ShotComponent>();

            //clear bullets, too...
            for (auto&& bullet : ctxt->world->system->entitiesWithComponent<EnemyBulletComponent>()) bullet.destroy();

            return  SubsectionState::Done;
         });
      }

      //-----MESH OF LIGHT AND DARK-----
      {
         int HEALTH = 200;
         int chaseCooldown = 300;
         int chaseFrames = 20;
         uint64_t nextChaseTime = ctxt->currentTick + chaseCooldown;
         uint64_t doneChasingFrame = 0;
         stateQueue.push([=](bool isFirstRun) mutable -> e_SubsectionState
         {
            if (isFirstRun)
            {
               ran.create<LivesWithNoHPComponent>();  //don't kill her when she dies
               ran.create<HealthComponent>(HEALTH); //todo: tweak, show a healthbar somehow....

               std::vector<Shot> shots;
               shots.push_back(Shot("MeshOfLightAndDarkLaser"));
               shots.push_back(Shot("MeshOfLightAndDarkLaser"));
               shots[0].angleOffset = 35;
               shots[1].angleOffset = -35;
               shots.push_back(Shot("MeshOfLightAndDarkSpew"));
               shots.back().nextFireTime = ctxt->currentTick + 55; //offset this a bit to be annoying.
               ran.create<ShotComponent>(std::move(shots));

               //aaaand now we're playing!
            }

            if (isAlive(ran))
            {
               chasePlayerHorizontally(ctxt, ran, nextChaseTime, doneChasingFrame, chaseFrames, chaseCooldown);
               return SubsectionState::StillProcessing;
            }


            for (auto shot : ctxt->world->system->entitiesWithComponent<EnemyBulletComponent>()) shot.destroy();

            //clean-up movement when dead.
            ran.remove<MaxSpeedComponent>();
            ran.remove<VelocityComponent>();
            ran.remove<AccelerationComponent>();

            //no more shoots
            ran.remove<ShotComponent>();

            return  SubsectionState::Done;
         });
      }


      //-----RECENTER-------
      stateQueue.push(moveToSubsection(ctxt, ran, RAN_START, 8.0f));

      //------RAN STARTS SHOOTING-----
      {
         int HEALTH = 100;
         int chaseCooldown = 120;
         int chaseFrames = 30;
         uint64_t nextChaseTime = ctxt->currentTick + chaseCooldown;
         uint64_t doneChasingFrame = 0;
         stateQueue.push([=](bool isFirstRun) mutable -> e_SubsectionState
         {
            if (isFirstRun)
            {
               ran.create<LivesWithNoHPComponent>();  //don't kill her when she dies
               ran.create<HealthComponent>(HEALTH); //todo: tweak, show a healthbar somehow....

               //let's have her fire some shots.  Let's start with her first nonspell...
               std::vector<Shot> shots;
               shots.push_back(Shot("RanNonspell1"));
               ran.create<ShotComponent>(std::move(shots));
               
               //aaaand now we're playing!
            }

            if (isAlive(ran))
            {
               chasePlayerHorizontally(ctxt, ran, nextChaseTime, doneChasingFrame, chaseFrames, chaseCooldown);
               return SubsectionState::StillProcessing;
            }

            for (auto shot : ctxt->world->system->entitiesWithComponent<EnemyBulletComponent>()) shot.destroy();

            //clean-up movement when dead.
            ran.remove<MaxSpeedComponent>();
            ran.remove<VelocityComponent>();
            ran.remove<AccelerationComponent>();

            //no more shoots
            ran.remove<ShotComponent>();

            return  SubsectionState::Done;
         });
      }

      //-----RECENTER-------
      stateQueue.push(moveToSubsection(ctxt, ran, RAN_START, 8.0f));

      //------DEVILS RECITATION!-----
      {
         int STARTING_HEALTH = 300;
         int BUBBLE_HEALTH = 280;
         int AIMED_BUBBLE_HEALTH = 240;
         int GARBAGE_HEALTH = 200;
         int SPEW_HEALTH = 125;
         int FAST_HEALTH = 50;

         int stage = 0;
         stateQueue.push([=](bool isFirstRun) mutable -> e_SubsectionState
         {
            if (isFirstRun)
            {
               ran.create<LivesWithNoHPComponent>();  //don't kill her when she dies
               ran.create<HealthComponent>(STARTING_HEALTH);

                                                 //let's have her fire some shots.  Let's start with her first nonspell...
               std::vector<Shot> shots;

               //add four instances of devil's recitation shots...
               struct Offset
               {
                  Vec2 position;
                  float angle; //fires right by default...
                  uint64_t tickOffset;
               } offsets[]
               {
                  {{100.0f, 50.0f}, 270.0f, 200}, // right, fires down
                  {{200.0f, -100.0f}, 240.0f,0}, // far right
                  {{-100.0f, 50.0f}, 270.0f, 200}, // left, fires down
                  {{-200.0f, -100.0f}, 300.0f,0} // far left
               };

               for (auto&& offset : offsets)
               {
                  Shot shot("DevilsRecitationBulletWave");
                  shot.offset = offset.position;
                  shot.angleOffset = offset.angle;
                  shot.tickOffset = offset.tickOffset;
                  shots.push_back(std::move(shot));
               }
                              
               ran.create<ShotComponent>(std::move(shots));
            }

            if (stage == 0 && ran.get<HealthComponent>()->hp < BUBBLE_HEALTH)
            {
               ++stage;
               //add a new shot type!
               ran.get<ShotComponent>()->shots.push_back(Shot("DevilsRecitationBubbleFall")); //start bubbles falling after she's been shot a few times.
            }

            if (stage == 1 && ran.get<HealthComponent>()->hp < AIMED_BUBBLE_HEALTH)
            {
               ++stage;
               ran.get<ShotComponent>()->shots.push_back(Shot("DevilsRecitationAimedBubble"));
            }

            if (stage == 2 && ran.get<HealthComponent>()->hp < GARBAGE_HEALTH)
            {
               ++stage;
               ran.get<ShotComponent>()->shots.push_back(Shot("DevilsRecitationGarbageFall"));                
            }


            if (stage == 3 && ran.get<HealthComponent>()->hp < SPEW_HEALTH)
            {
               ++stage;
               ran.get<ShotComponent>()->shots.push_back(Shot("DevilsRecitationAngleSpew"));
            }

            if (stage == 4 && ran.get<HealthComponent>()->hp < FAST_HEALTH)
            {
               ++stage;
               ran.get<ShotComponent>()->shots.push_back(Shot("DevilsRecitationQuickAimedBubble"));
            }

            

            if (isAlive(ran))
            {
               return SubsectionState::StillProcessing;
            }

            //clean-up movement when dead.
            ran.remove<MaxSpeedComponent>();
            ran.remove<VelocityComponent>();
            ran.remove<AccelerationComponent>();

            //no more shoots
            ran.remove<ShotComponent>();

            return  SubsectionState::Done;
         });
      }


      //------RAN IS KILL-----
      {
         stateQueue.push([=](bool isFirstRun) mutable -> e_SubsectionState
         {
            //TODO: BIGASS SUPER EXPLOSION
            playSFX("sfx/blast.wav");
            ran.create<MarkedForDeletionComponent>();

            return SubsectionState::Done;
         });
      }
   }

   virtual void onUpdate() override
   {
      stateQueue.update();
   }

   virtual bool finished() override
   {
      return stateQueue.isFinished();
   }
   BulletHellContext* ctxt;
   LevelStateQueue stateQueue;
};

class TrevorSection : public LevelSection
{
public:
   TrevorSection(BulletHellContext* in_ctxt)
      : ctxt(in_ctxt)
   {

   }

   virtual void onEnter() override
   {
      skull = Entity(ctxt->world);


      //spawn her a bit to the left from center and off the screen.  She "flies in" as the first thing before she starts attacking.
      Vec2 pos(constants::cameraSize.x / 2, 900);

      skull.create<PositionComponent>(pos);

      skull.create<EnemyComponent>();
      skull.create<SizeComponent>(128.0f, 128.0f);

      //most importantly....
      skull.create<SpriteComponent>("png/flamingskull.png");

      playBGM("music/bsong1.mp3");

      skull.create<ShotComponent>(Shot("skullshot1"));
      skull.create<HealthComponent>(100);
   }

   virtual void onUpdate() override
   {
      if (finished())
         return;

      if (phase == 0 && skull.get<HealthComponent>()->hp <= 50)
      {
         for (auto shot : ctxt->world->system->entitiesWithComponent<EnemyBulletComponent>()) shot.destroy();
         phase = 1;
         //skull.get<ShotComponent>()->shots.push_back(Shot("skullshot2"));
         skull.create<ShotComponent>(Shot("skullshot2"));
      }
   }

   virtual bool finished() override
   {
      return ctxt->world->system->entitiesWithComponent<EnemyComponent>().empty();
   }

   BulletHellContext* ctxt;
   int phase = 0;
   Entity skull;
};

Level testLevelCreate(BulletHellContext* context)
{
   Level out;

   //out.sections.push_back(std::make_unique<TrevorSection>(context));
   //out.sections.push_back(std::make_unique<CreateSomeTestEnemies>(context));
   out.sections.push_back(std::make_unique<RanYakumoFromTouhouYouyoumuSection>(context));
   out.sections.push_back(std::make_unique<WaitSection>(context, 60 * 60)); //1 minute of waiting

   return out;
}