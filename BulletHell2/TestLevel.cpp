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

//todo: lift this function somerwhere sane
void playBGM(BulletHellContext* ctxt, std::string const& path)
{
   if (ctxt->playingBGM) delete ctxt->playingBGM;
   ctxt->playingBGM = new Sound(Sound::createSong(*ctxt->audio, path));
   ctxt->playingBGM->play();
}
void playSFX(BulletHellContext* ctxt, std::string const& path)
{
   auto sfx = Sound::createSample(*ctxt->audio, path.c_str());
   sfx.play();
   sfx.detach();
}
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
      playBGM(ctxt, "music/boss.mp3");


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
         int GARBAGE_HEALTH = 200;

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
               stage = 1;
               //add a new shot type!
               ran.get<ShotComponent>()->shots.push_back(Shot("DevilsRecitationBubbleFall")); //start bubbles falling after she's been shot a few times.
            }

            if (stage == 1 && ran.get<HealthComponent>()->hp < GARBAGE_HEALTH)
            {
               stage = 2;
               ran.get<ShotComponent>()->shots.push_back(Shot("DevilsRecitationGarbageFall"));                
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
            playSFX(ctxt, "sfx/blast.wav");
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

Level testLevelCreate(BulletHellContext* context)
{
   Level out;

   out.sections.push_back(std::make_unique<CreateSomeTestEnemies>(context));
   out.sections.push_back(std::make_unique<RanYakumoFromTouhouYouyoumuSection>(context));
   out.sections.push_back(std::make_unique<WaitSection>(context, 60 * 60)); //1 minute of waiting

   return out;
}