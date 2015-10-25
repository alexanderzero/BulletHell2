#include "ShotType.hpp"

#include "EntitySystemView.hpp"
#include "entity.hpp"
#include "NameIndex.hpp"
#include "components.hpp"
#include "BulletHell2.hpp"
#include "random.hpp"
#include "constants.hpp"

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


std::unordered_map<std::string, IShotType*> g_rawNonSpells; //simply implemented interfaces.

//----------------------RAN YAKUMO SHOT TYPES----------------------


//nonspell 1 - fires circles of three shots
//1. fast, aimed bullets.  fires in a circle to hide the fact that they're playing tracking
//2. slower, aimed bullets.  creates "wave" feel.
//3. mid-speed, aimed, with an angle offset from the player position.
//all of these fire at the same cooldown so this is a single "shot", which is nice for our purposes :)

static const int RAN_NON1_BULLET_COUNT = 48;

Entity getPlayer(BulletHellContext* context)
{
   return entityGetByName(context->world, "player");
}

float angleToPlayer(BulletHellContext* context, Entity ent)
{
   Vec2 playerPosition;
   if (auto player = entityGetByName(context->world, "player"))
   {
      if (auto pos = player.get<PositionComponent>()) playerPosition = pos->pos;
   }

   return rectToPolar(playerPosition -= ent.get<PositionComponent>()->pos);
}

class RanNonSpell1 : public IShotType
{

   virtual void fire(BulletHellContext* context, Entity ent, Shot* shot) override
   { 
	  float playerAngle = angleToPlayer(context, ent);
	  float attackAngle = playerAngle;
      float angleStep = TAU / (RAN_NON1_BULLET_COUNT - 1);

      
      float baseSpeed = 2.0f;
      float speedStep = 2.0f;


      auto pos = ent.get<PositionComponent>()->pos;
      for (int i = 0; i < RAN_NON1_BULLET_COUNT; ++i)
      {
         //the 3 speeds
         for (int j = 0; j < 3; ++j)
         {
            Entity bullet(context->world);

            bullet.create<PositionComponent>(pos);
            bullet.create<SizeComponent>(4.0f, 4.0f);

            bullet.create<DieOffscreenComponent>();
            float angle = j == 1 ? attackAngle + angleStep / 2 : attackAngle; //the middle speed shot is offset
            bullet.create<VelocityComponent>(polarToRect(angle, baseSpeed + speedStep * j));
            
            bullet.create<SpriteComponent>("png/fireball_1.png");

            //always an enemy bullet
            bullet.create<EnemyBulletComponent>();

            bullet.update();
         }

         attackAngle += angleStep;
      }
	  //create some lasers, just for fun.
	  attackAngle = playerAngle;
	  for (int i = 0; i < 16; ++i)
	  {
        Entity bullet(context->world);

        bullet.create<PositionComponent>(pos);
        bullet.create<LaserComponent>(polarToRect(attackAngle), 0.0f);
        ResizingLaserComponent laser;
        laser.startTick = g_context.currentTick;
        laser.fullWidthTick = laser.startTick + 30;
        laser.startFadeTick = laser.fullWidthTick + 20;
        laser.endTick = laser.startFadeTick + 10;
        laser.maxWidth = 12.0f;
        bullet.set(std::move(laser));

        bullet.create<SpriteComponent>("png/babble_red.png");

        //always an enemy bullet
        bullet.create<EnemyBulletComponent>();

        bullet.update();

        attackAngle += TAU / 16;
	  }


      //aaand we fired!
   }
   virtual int getCooldown()
   {
      //fire the barrage once every this many ticks....
      return 60;
   }
}; 


//----------------------DEVIL'S RECITATION----------------------

//this is a famous attack in the touhou/seihou circles.
//it's an attack that adds more and more shots, the lower HP the enemy has.
//this has some of the major elements of the attack and tries to capture its spirit.

//Stage 1 - bullet waves.
//four ocillating waves of 3 streams on each.
//we define one wave, then it can be added as four shot types.

//it's probably going to be the case where shot types require state, eh?  well perhaps it should be jammed in a component.... ugh

static const float devilsRecitationWaveOffset = 55.0f; //degrees between each shot
static const int devilsRecitationWaveTicksPerLoop = 200;
static const float devilsRecitationLoopDistance = 15.0f;
static const float devilsRecitationBulletSpeed = 8.0f;

class DevilsRecitationBulletWave : public IShotType
{
public:

   virtual void fire(BulletHellContext* context, Entity ent, Shot* shot) override
   {
      //create 3 streams, which slowly go back and forth.
      float angles[3];

      //base angles
      angles[0] = -devilsRecitationWaveOffset;
      angles[1] = 0.0f;
      angles[2] = devilsRecitationWaveOffset;

      //time offset
      auto tick = shot->tickOffset + context->currentTick;
      bool mirrored = (tick / devilsRecitationWaveTicksPerLoop) % 2 == 0;
      float t = (tick % devilsRecitationWaveTicksPerLoop) / (float)(devilsRecitationWaveTicksPerLoop);
      if (mirrored) t = 1.0f - t;
      float timeAngle = (-0.5f + t) * (devilsRecitationLoopDistance);

      //create a bullet at each angle.
      for (auto&& angle : angles)
      {
         float radAng = degToRad(angle + timeAngle + shot->angleOffset);

         Entity bullet(context->world);
         Vec2 pos = ent.get<PositionComponent>()->pos;
         pos += shot->offset;

         bullet.create<PositionComponent>(pos);
         bullet.create<SizeComponent>(4.0f, 4.0f);

         bullet.create<DieOffscreenComponent>();
         bullet.create<VelocityComponent>(polarToRect(radAng, devilsRecitationBulletSpeed));

         bullet.create<SpriteComponent>("png/fireball_1.png");

         //always an enemy bullet
         bullet.create<EnemyBulletComponent>();

         bullet.update();
      }
   }
   virtual int getCooldown()
   {
      return 3;
   }
};

//Stage 2 - bubble fall.
//randomly falling bubbles.  created totally at random on x with some slight random offsets from the player on y.  pretty much a normal accelerating shot.  create one a frame.

class DevilsRecitationBubbleFall : public IShotType
{
public:

   virtual void fire(BulletHellContext* context, Entity ent, Shot* shot) override
   {
      Vec2 pos = ent.get<PositionComponent>()->pos;
      pos.x = randomFloat(0, constants::cameraSize.x);
      pos.y += randomFloat(-20.0f, 20.0f);

      Entity bullet(context->world);
      
      pos += shot->offset;

      bullet.create<PositionComponent>(pos);
      bullet.create<SizeComponent>(6.0f, 6.0f);

      bullet.create<DieOffscreenComponent>();
      bullet.create<AccelerationComponent>(0.0f, randomFloat(-0.25f, -0.15f));
      bullet.create<MaxSpeedComponent>(randomFloat(6.0f, 8.0f));

      bullet.create<SpriteComponent>("png/babble_red.png");

      //always an enemy bullet
      bullet.create<EnemyBulletComponent>();

      bullet.update();
      

   }
   virtual int getCooldown()
   {
      return 3;
   }
};


//Stage 3 - aimed bubbles.
//3 bubbles aimed around the player every once in a while

class DevilsRecitationAimedBubble : public IShotType
{
public:

   virtual void fire(BulletHellContext* context, Entity ent, Shot* shot) override
   {
      Vec2 pos = ent.get<PositionComponent>()->pos;
      float attackAngle = angleToPlayer(context, ent);

      float angles[] =
      {
         -35.0f, 0.0f, 35.0f
      };

      for (auto&& angle : angles)
      {

         Entity bullet(context->world);

         pos += shot->offset;

         bullet.create<PositionComponent>(pos);
         bullet.create<SizeComponent>(6.0f, 6.0f);

         bullet.create<DieOffscreenComponent>();
         bullet.create<VelocityComponent>(polarToRect(attackAngle + degToRad(angle), 5.0f));

         bullet.create<SpriteComponent>("png/babble_red.png"); //todo - different color

         //always an enemy bullet
         bullet.create<EnemyBulletComponent>();

         bullet.update();

      }


   }
   virtual int getCooldown()
   {
      return 50;
   }
};



//Stage 4 - garbage fall.
//randomly falling garbage.  falls slower than bubbles, can have a random slight x direciton.

class DevilsRecitationGarbageFall : public IShotType
{
public:

   virtual void fire(BulletHellContext* context, Entity ent, Shot* shot) override
   {
      Vec2 pos = ent.get<PositionComponent>()->pos;
      pos.x = randomFloat(0, constants::cameraSize.x);
      pos.y += randomFloat(-20.0f, 20.0f);

      Entity bullet(context->world);


      pos += shot->offset;

      bullet.create<PositionComponent>(pos);
      bullet.create<SizeComponent>(4.0f, 4.0f);

      bullet.create<DieOffscreenComponent>();
      bullet.create<AccelerationComponent>(randomFloat(-0.025, 0.025f), -0.1f);
      bullet.create<MaxSpeedComponent>(4.0f);

      bullet.create<SpriteComponent>("png/fireball_1.png");

      //always an enemy bullet
      bullet.create<EnemyBulletComponent>();

      bullet.update();


   }
   virtual int getCooldown()
   {
      return 2;
   }
};


//Stage 5 - angle spew
//fires back and forth at an angle.

static const float devilsRecitationAngleSpewAngle = 75.0f; //difference from straight-down.  so the full fan is this * 2
static const int devilsRecitationSpewTicksPerLoop = 100;  //this should be at odds with the firing rate
static const float devilsRecitationSpewBulletSpeed = 5.0f;

class DevilsRecitationAngleSpew : public IShotType
{
public:

   virtual void fire(BulletHellContext* context, Entity ent, Shot* shot) override
   {
      Vec2 pos = ent.get<PositionComponent>()->pos;


      //time offset
      auto tick = context->currentTick;
      bool mirrored = (tick / devilsRecitationSpewTicksPerLoop) % 2 == 0;
      float t = (tick % devilsRecitationSpewTicksPerLoop) / (float)(devilsRecitationSpewTicksPerLoop);
      if (mirrored) t = 1.0f - t;
      float timeAngle = (-0.5f + t) * (devilsRecitationAngleSpewAngle * 2);

      Entity bullet(context->world);

      bullet.create<PositionComponent>(pos);
      bullet.create<SizeComponent>(4.0f, 4.0f);

      bullet.create<DieOffscreenComponent>();
      bullet.create<AccelerationComponent>(polarToRect(degToRad(270 + timeAngle), 0.8f));
      bullet.create<MaxSpeedComponent>(devilsRecitationSpewBulletSpeed);

      bullet.create<SpriteComponent>("png/fireball_1.png");

      //always an enemy bullet
      bullet.create<EnemyBulletComponent>();

      bullet.update();
   }
   virtual int getCooldown()
   {
      return 3;
   }
};




//Stage 6 - quick aimed bubbles.

class DevilsRecitationQuickAimedBubble : public IShotType
{
public:

   virtual void fire(BulletHellContext* context, Entity ent, Shot* shot) override
   {
      Vec2 pos = ent.get<PositionComponent>()->pos;
      float attackAngle = angleToPlayer(context, ent);

      float angles[] =
      {
         -100.0, -80.0, -60.0, -40.0, -20.0f, 0.0f, 20.0f, 40.0f, 60.0, 80.0, 100.0
      };

      for (auto&& angle : angles)
      {

         Entity bullet(context->world);

         pos += shot->offset;

         bullet.create<PositionComponent>(pos);
         bullet.create<SizeComponent>(6.0f, 6.0f);

         bullet.create<DieOffscreenComponent>();
         bullet.create<VelocityComponent>(polarToRect(attackAngle + degToRad(angle), 16.0f));

         bullet.create<SpriteComponent>("png/babble_red.png"); //todo - different color again

                                                               //always an enemy bullet
         bullet.create<EnemyBulletComponent>();

         bullet.update();

      }


   }
   virtual int getCooldown()
   {
      return 75;
   }
};

//skullshot1

class SkullShot1 : public IShotType
{
public:
   float angle;
   SkullShot1()
   {
      angle = 0.0;
   }
   virtual void fire(BulletHellContext* context, Entity ent, Shot* shot)
   {
      for (size_t i = 0; i < 4; i++)
      {
         Entity bullet(context->world);
         Vec2 position = ent.get<PositionComponent>()->pos;

         float shot_angle = angle;
         
         shot_angle += 90.0*i;
       

         bullet.create<PositionComponent>(position);
         bullet.create<VelocityComponent>(polarToRect(degToRad(shot_angle), 4.0));
         bullet.create<RadiusComponent>(4.0);
         bullet.create<SpriteComponent>("png/fireball_1.png");
         bullet.create<EnemyBulletComponent>();

         bullet.update();
      }
      for (size_t i = 0; i < 4; i++)
      {
         Entity bullet(context->world);
         Vec2 position = ent.get<PositionComponent>()->pos;

         float shot_angle = angle;

         shot_angle += 90.0*i;


         bullet.create<PositionComponent>(position);
         bullet.create<VelocityComponent>(polarToRect(degToRad(-shot_angle), 2.0));
         bullet.create<RadiusComponent>(4.0);
         bullet.create<SpriteComponent>("png/fireball_1.png");
         bullet.create<EnemyBulletComponent>();

         bullet.update();
      }


      angle += 7;
   }
   virtual int getCooldown()
   {
      return 3;
   }
};

class SkullShot2 : public IShotType
{
public:
   virtual void fire(BulletHellContext* context, Entity ent, Shot* shot)
   {
      Vec2 start = polarToRect(randomFloat(0,TAU),randomFloat(0, 128));
      Vec2 end = polarToRect(randomFloat(0, TAU), randomFloat(0, 8));

      start += ent.get<PositionComponent>()->pos;
      end += getPlayer(context).get<PositionComponent>()->pos;

      Entity laser(context->world);

      laser.create<PositionComponent>(start);
      laser.create<LaserComponent>(polarToRect(angleBetween(start, end)), 0);
      ResizingLaserComponent rescomp;
      rescomp.startTick = context->currentTick;
      rescomp.fullWidthTick = context->currentTick + 30;
      rescomp.maxWidth = 8;
      rescomp.startFadeTick = context->currentTick + 60;
      rescomp.endTick = context->currentTick + 90;
      laser.set(std::move(rescomp));
      laser.create<SpriteComponent>("png/babble_red.png");
      laser.create<EnemyBulletComponent>();

      laser.update();
   }
   virtual int getCooldown()
   {
      return 20;
   }
};


template <typename ShotFn, typename CooldownFn>
void buildShotType(std::string name, ShotFn&& fn, CooldownFn&& cdFn)
{
   class ShotTypeImpl : public IShotType
   {
   public:
      ShotTypeImpl(ShotFn&& fn, CooldownFn&& cdFn)
         : s(std::forward<ShotFn>(fn)), c(std::forward<CooldownFn>(cdFn))
      {

      }

      virtual void fire(BulletHellContext* context, Entity ent, Shot* shot)
      {
         s(ent, shot);
      }
      virtual int getCooldown()
      {
         return c();
      }

      typename std::decay<ShotFn>::type s;
      typename std::decay<CooldownFn>::type c;
   };


   g_rawNonSpells.insert(std::make_pair(std::move(name), new ShotTypeImpl(std::forward<ShotFn>(fn), std::forward<CooldownFn>(cdFn))));  
}

void buildMeshOfLightAndDark()
{
   int meshOfLightAndDarkShotTime = 150;

   auto ctxt = &g_context;
   buildShotType("MeshOfLightAndDarkLaser",
   [=](Entity ent, Shot* shot)
   {
      float angle = angleToPlayer(ctxt, ent) + degToRad(shot->angleOffset);

      Entity laser(ctxt->world);

      laser.create<PositionComponent>(ent.get<PositionComponent>()->pos);
      laser.create<LaserComponent>(polarToRect(angle), 0);

      ResizingLaserComponent rescomp;
      rescomp.startTick = ctxt->currentTick;
      rescomp.fullWidthTick = ctxt->currentTick + 30;
      rescomp.startFadeTick = ctxt->currentTick + 150;
      rescomp.endTick = ctxt->currentTick + 165;

      rescomp.maxWidth = 32;

      laser.set(std::move(rescomp));
      laser.create<SpriteComponent>("png/babble_red.png");
      laser.create<EnemyBulletComponent>();

      //we also create bullets that fly down this path as well.  These bullets are INVISIBLE!

      Entity spawner(ctxt->world);

      //no need for a size or a sprite.
      spawner.create<PositionComponent>(ent.get<PositionComponent>()->pos);
      spawner.create<VelocityComponent>(polarToRect(angle, 12.0f));
      spawner.create<EnemyBulletComponent>();
      spawner.create<DieOffscreenComponent>();
      //create a component to handle the mesh of light and dark spawning. 
      spawner.create<MeshOfLightAndDarkSpawnComponent>();
      
   },
   [=]()
   {
      return meshOfLightAndDarkShotTime;
   });


   
   buildShotType("MeshOfLightAndDarkSpew",
      [=](Entity ent, Shot* shot)
   {

      int shotCount = randomInt(48, 65);
      for (int i = 0; i < shotCount; ++i)
      { 
         //shot in the general area of the player
         //basically a "shotgun"
         float angle = angleToPlayer(ctxt, ent) + degToRad(randomFloat(-35.0f, 35.0f));

         Entity bullet(ctxt->world);

         bullet.create<PositionComponent>(ent.get<PositionComponent>()->pos);
         bullet.create<VelocityComponent>(polarToRect(angle, randomFloat(3.0f, 8.0f)));
         bullet.create<RadiusComponent>(4.0f);
         bullet.create<SpriteComponent>("png/fireball_1.png");
         bullet.create<EnemyBulletComponent>();
         bullet.create<DieOffscreenComponent>();
      }
   },
      [=]()
   {
      return meshOfLightAndDarkShotTime;
   });
}

void buildNueLaserBullets()
{
   int cooldown = 40;
   int count = 20;
   int trainCount = 8;

   auto ctxt = &g_context;
   buildShotType("NueLaserBulletRing",
      [=](Entity ent, Shot* shot)
   {
      
      for (int i = 0; i < count; ++i)
      {
         float angle = (TAU / count) * i + degToRad(shot->angleOffset);

         for (int j = 0; j < trainCount; ++j)
         {
            auto vel = polarToRect(angle, 5.0f);
            auto pos = ent.get<PositionComponent>()->pos;
            auto offset = vel;
            offset *= -j * 4.0f;
            pos += offset;


            Entity bullet(ctxt->world);

            bullet.create<PositionComponent>(pos);
            bullet.create<VelocityComponent>(vel);
            bullet.create<RadiusComponent>(4.0);
            bullet.create<SpriteComponent>("png/fireball_1.png");
            bullet.create<EnemyBulletComponent>();
            bullet.create<AlignToVelocityComponent>();
            bullet.create<DiePaddedOffscreenComponent>();

            if (!(j % 2)) bullet.create<RotateLeftComponent>();

            bullet.update();
         }
      }
   },
      [=]()
   {
      return cooldown;
   });
}


//time to try and be clever and come up with some of my own attacks rather than just copy 2hu

void buildCurvyBulletHell()
{
   //this one does curvy bullets which you'll want to dodge at the screen bottom....
   int cooldown = 50;
   int count = 125;

   auto ctxt = &g_context;
   buildShotType("CurvyBulletRing",
      [=](Entity ent, Shot* shot)
   {
      float offset = randomFloat(0, TAU);
      for (int i = 0; i < count; ++i)
      {
         float angle = (TAU / count) * i + degToRad(shot->angleOffset) + offset;

         auto vel = polarToRect(angle, 1.5f);
         auto pos = ent.get<PositionComponent>()->pos;
         
         Entity bullet(ctxt->world);

         bullet.create<PositionComponent>(pos);
         bullet.create<VelocityComponent>(vel);
         bullet.create<RadiusComponent>(4.0);
         bullet.create<SpriteComponent>("png/fireball_1.png");
         bullet.create<EnemyBulletComponent>();
         bullet.create<AlignToVelocityComponent>();
         bullet.create<DiePaddedOffscreenComponent>();
         bullet.create<CurvyRingTagComponent>();

         if (!(i % 2)) bullet.create<RotateLeftComponent>();

         //apply effect using this...  TODO: link this less awkwardly?  manager-ize?
         bullet.create<SpawnTimeComponent>(ctxt->currentTick);
         

         bullet.update();
         
      }
   },
      [=]()
   {
      return cooldown;
   });


   //so to mess with you, we randomly spawn bullets at the buttom of the screen which slowly move up :)
   cooldown = 12;
   
   buildShotType("CurvyBulletBottomFire",
      [=](Entity ent, Shot* shot)
   {
      auto pos = Vec2(randomFloat(0.0f, constants::cameraSize.x), 0.0f);

      Entity bullet(ctxt->world);

      bullet.create<PositionComponent>(pos);
      bullet.create<AccelerationComponent>(Vec2(0.0f, randomFloat(0.08f, 0.16f)));
      bullet.create<MaxSpeedComponent>(randomFloat(4.0f, 6.0f));
      bullet.create<RadiusComponent>(6.0);
      bullet.create<SpriteComponent>("png/babble_red.png");
      bullet.create<EnemyBulletComponent>();
      bullet.create<AlignToVelocityComponent>();
      bullet.create<DieOffscreenComponent>();
         
      bullet.update();
   },
      [=]()
   {
      return cooldown;
   });
}

void buildBombsAway()
{
   //this one does curvy bullets which you'll want to dodge at the screen bottom....
   int cooldown = 140;
   int lifeTime = 190;
   int count = 16;
   int startDistance = 400;
   float anglePerFrame = degToRad(0.6f);
   
   auto ctxt = &g_context;
   buildShotType("BombsAway",
      [=](Entity ent, Shot* shot)
   {
      float offset = randomFloat(0, TAU);

      Circle spawnCircle;
      spawnCircle.center = getPlayer(ctxt).get<PositionComponent>()->pos;
      spawnCircle.radius = 275.0f;

      auto center = randomPointInCircle(spawnCircle);

      int direction = randomInt(0, 2);

      for (int i = 0; i < count; ++i)
      {
         float angle = (TAU / count) * i + degToRad(shot->angleOffset) + offset;

         auto vel = polarToRect(angle, 1.5f);
         
         Entity bullet(ctxt->world);
         
         bullet.create<RadiusComponent>(4.0);
         bullet.create<SpriteComponent>("png/fireball_1.png");
         bullet.create<EnemyBulletComponent>();
         bullet.create<AlignToVelocityComponent>();
         bullet.create<TimedDeathComponent>(ctxt->currentTick + lifeTime);


         if (!i) //create just one...
         {
            bullet.create<ShotComponentOnDeathComponent>(Shot("BombsAwayBomb"));
         }

         ArcInterpolationComponent arc;

         arc.startTick = ctxt->currentTick;
         arc.endTick = ctxt->currentTick + lifeTime;

         arc.center = center;

         arc.startDistance = startDistance;
         arc.endDistance = 0;
         
         float angleOffset = anglePerFrame;
         if (direction == 1)
         {
            angleOffset = -angleOffset;
         }
         else if (direction == 2)
         {
            if (i % 2)
            {
               angleOffset = -angleOffset;
            }
         }

         arc.startAngle = angle;
         arc.endAngle = angle + angleOffset * lifeTime;
         
         arc.cosineInterp = false;

         bullet.set(arc);

         auto pos = arc.center;
         pos += polarToRect(arc.startAngle, arc.startDistance);
         bullet.create<PositionComponent>(pos);

         bullet.update();
      }
   },
      [=]()
   {
      return cooldown;
   });


   buildShotType("BombsAwayBomb",
      [=](Entity ent, Shot* shot)
   {
      for (int i = 0; i < count; ++i)
      {
         float angle = randomFloat(0, TAU);

         Entity bullet(ctxt->world);

         bullet.create<PositionComponent>(ent.get<PositionComponent>()->pos);
         bullet.create<VelocityComponent>(polarToRect(angle, randomFloat(1.0f, 5.0f)));
         bullet.create<RadiusComponent>(4.0);
         bullet.create<SpriteComponent>("png/fireball_2.png");
         bullet.create<EnemyBulletComponent>();
         bullet.create<AlignToVelocityComponent>();
         bullet.create<DiePaddedOffscreenComponent>();
         

         bullet.update();
      }
   },
      [=]()
   {
      return 0; //unused
   });
}



void buildStreamingCage()
{
   int buildEveryX = 65;
   int count = constants::cameraSize.x / buildEveryX;
   float speed = 5.0f;
   int cooldown = 25;
   int familiarTicks = 80;

   auto ctxt = &g_context;

   buildShotType("StreamingCageFamiliar",
      [=](Entity ent, Shot* shot)
   {

      for (int i = 0; i < count; ++i)
      {
         float x = i * buildEveryX;
         bool top = (i % 2) == 0;
         //bool top = true;
         float y = top ? constants::cameraSize.y : 0;


         
         Vec2 pos = ent.get<PositionComponent>()->pos;
         Vec2 target(x, y);
         
         Vec2 vel = target;
         vel -= pos;
         vel *= 1.0f/familiarTicks;

         Entity bullet(ctxt->world);

         bullet.create<PositionComponent>(pos);
         bullet.create<VelocityComponent>(vel);
         bullet.create<RadiusComponent>(4.0);
         bullet.create<SpriteComponent>("png/babble_red.png");
         bullet.create<EnemyBulletComponent>();
         bullet.create<DiePaddedOffscreenComponent>();

         bullet.update();
      }
   },
      [=]()
   {
      return 1000000000;
   });

   buildShotType("StreamingCageCage",
      [=](Entity ent, Shot* shot)
   {

      for (int i = 0; i < count; ++i)
      {
         float x = i * buildEveryX;
         bool top = (i % 2) == 0;
         //bool top = true;
         float y = top ? constants::cameraSize.y : 0;

         Vec2 pos(x, y);
         Vec2 vel(0.0f, top ? -speed : speed);

         Entity bullet(ctxt->world);
         
         bullet.create<PositionComponent>(pos);
         bullet.create<VelocityComponent>(vel);
         bullet.create<RadiusComponent>(4.0);
         bullet.create<SpriteComponent>("png/fireball_2.png");
         bullet.create<EnemyBulletComponent>();
         bullet.create<AlignToVelocityComponent>();
         bullet.create<DieOffscreenComponent>();

         bullet.update();
      }
   },
      [=]()
   {
      return cooldown;
   });

   buildShotType("StreamingCageStream",
      [=](Entity ent, Shot* shot)
   {

      Entity bullet(ctxt->world);

      bullet.create<PositionComponent>(ent.get<PositionComponent>()->pos);
      bullet.create<VelocityComponent>(polarToRect(angleToPlayer(ctxt, ent), 8.0f));
      bullet.create<RadiusComponent>(4.0);
      bullet.create<SpriteComponent>("png/fireball_1.png");
      bullet.create<EnemyBulletComponent>();
      bullet.create<AlignToVelocityComponent>();
      bullet.create<DieOffscreenComponent>();

      bullet.update();
      
   },
      [=]()
   {
      return 12;
   });
}

void hackBuildTestShotTypes(EntitySystemView*& shotTypes)
{
   shotTypes = new EntitySystemView;
   auto db = shotTypes->system = new EntitySystem;
   shotTypes->nameIndex = new NameIndex;


   buildPlayerShotType(shotTypes);
   buildEnemyShotType(shotTypes);

   g_rawNonSpells.insert(std::make_pair("RanNonspell1", new RanNonSpell1));
   g_rawNonSpells.insert(std::make_pair("DevilsRecitationBulletWave", new DevilsRecitationBulletWave));   
   g_rawNonSpells.insert(std::make_pair("DevilsRecitationBubbleFall", new DevilsRecitationBubbleFall));
   g_rawNonSpells.insert(std::make_pair("DevilsRecitationAimedBubble", new DevilsRecitationAimedBubble));   
   g_rawNonSpells.insert(std::make_pair("DevilsRecitationGarbageFall", new DevilsRecitationGarbageFall));
   g_rawNonSpells.insert(std::make_pair("DevilsRecitationAngleSpew", new DevilsRecitationAngleSpew));
   g_rawNonSpells.insert(std::make_pair("DevilsRecitationQuickAimedBubble", new DevilsRecitationQuickAimedBubble));   
   
   g_rawNonSpells.insert(std::make_pair("skullshot1", new SkullShot1));
   g_rawNonSpells.insert(std::make_pair("skullshot2", new SkullShot2));

   buildMeshOfLightAndDark();
   buildNueLaserBullets();
   buildCurvyBulletHell();
   buildBombsAway();
   buildStreamingCage();
}


class ShotComponentImpl : public IShotType
{
public:
   virtual void fire(BulletHellContext* context, Entity ent, Shot* shot) override
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
   virtual int getCooldown()
   {
      return ::getShotCooldown(shotType);
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

IShotType* getShotType(BulletHellContext* context, std::string const& shotType)
{
   if (auto shotEnt = context->shotTypes->nameIndex->getEntity(shotType)) return getShotType(context, shotEnt);

   //is it a raw type...?
   auto iter = g_rawNonSpells.find(shotType);
   if (iter == g_rawNonSpells.end()) return nullptr; //uhhhh

   return iter->second;
}