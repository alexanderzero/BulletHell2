#include "RunLevelState.hpp"

#include "window.hpp"
#include "entity.hpp"
#include "entitySystemView.hpp"
#include "components.hpp"

#include "audio.hpp"
#include "input.hpp"

#include "constants.hpp"

#include "NameIndex.hpp"

#include "ShotType.hpp"
#include "sprite.h"
#include "random.hpp"

void destroyMarkedForDeletion(EntitySystemView* systemView)
{
   auto vec = systemView->system->entitiesWithComponent<MarkedForDeletionComponent>();
   for (auto&& ent : vec) ent.destroy();  //note: if this destroy can do callbacks this might have issues with a child and a parent being marked for deletion, so this algorithm might have to be "smarter" about recursively deleting.  This isn't an issue as of writing
}

void fireShot(BulletHellContext* ctxt, Entity player, Shot& shot)
{
   auto iface = getShotType(ctxt, shot.type);
   if (!iface) return; //uhhhh
   
                       
   //manage cooldowns.
   shot.nextFireTime = ctxt->currentTick;
   shot.nextFireTime += iface->getCooldown();

   //fire!
   iface->fire(ctxt, player, &shot);
}
void fireAllWeapons(BulletHellContext* ctxt, Entity player)
{
   //ok, we're firing, but what do we fire?
   //two parts to this - what active data do we have (e.g. current weapon cooldown)
   //and what static data do we have (e.g. cooldown to reset after firing, how much damage we do on hit, etc)
   //and then, where are they stored?

   //a single player may also have multiple weapons (main firing gun with homing bullets that fire at a different cooldown rate.)

   auto shotComp = player.get<ShotComponent>();
   if (!shotComp) return; // nothing to fire...
   for (auto&& shot : shotComp->shots)
   {
      if (shot.nextFireTime > ctxt->currentTick) continue; //on cooldown
      fireShot(ctxt, player, shot);
   }
}

void updatePlayerVelocity(BulletHellContext* ctxt)
{
   for (auto player : ctxt->world->system->entitiesWithComponent<PlayerComponent>())
   {
      Vec2 velocity;
      auto pc = player.get<PlayerComponent>();

      if (pc->left) velocity.x -= 1.0f;
      if (pc->right) velocity.x += 1.0f;
      if (pc->down) velocity.y -= 1.0f;
      if (pc->up) velocity.y += 1.0f;

      //if there's a velocity, normalize it, and scale it to match the player's speed
      //we can directly compare against 0.0 here, it's exact math above
      if (velocity.x != 0.0f || velocity.y != 0.0f)
      {
         normalize(velocity);
         velocity *= pc->focused ? constants::playerFocusedSpeed : constants::playerSpeed;
      }

      player.create<VelocityComponent>(velocity); //overwrites the old velocity value.
   }
}
void updateLasers()
{
   auto tick = g_context.currentTick;
   for (auto ent : g_context.world->system->entitiesWithComponent<ResizingLaserComponent>())
   {
      auto rc = ent.get<ResizingLaserComponent>();
      if (tick >= rc->endTick)
      {
         ent.create<MarkedForDeletionComponent>();
         continue;
      }
      float t = 1.0f;
      if (tick >= rc->startFadeTick)
      {
         t = 1.0f-((tick - rc->startFadeTick) / ((float)rc->endTick - rc->startFadeTick));
         ent.create<NoDamageComponent>();
      }
      else if (tick >= rc->fullWidthTick)
      {
         //... manage components to deal damage?
         ent.remove<NoDamageComponent>();
      }
      else
      {
         t = (tick - rc->startTick) / ((float)rc->fullWidthTick- rc->startTick);
         ent.create<NoDamageComponent>();
      }
      if (auto laser = ent.get<LaserComponent>())
      {
         laser->width = t * rc->maxWidth;
      }
   }

   destroyMarkedForDeletion(g_context.world);
}


void enforceWorldBoundaries(BulletHellContext* ctxt)
{
   for (auto ent : ctxt->world->system->entitiesWithComponent<WorldBoundedComponent>())
   {
      //do this a kinda dumb way for now - just only do the position component.  Later we should handle sizes etc.  But that's not standardized yet so I'm not going to go nuts with it
      auto pos = ent.get<PositionComponent>();
      if (!pos) continue;

      if (pos->pos.x < 0) pos->pos.x = 0;
      else if (pos->pos.x > constants::cameraSize.x) pos->pos.x = constants::cameraSize.x;

      if (pos->pos.y < 0) pos->pos.y = 0;
      else if (pos->pos.y > constants::cameraSize.y) pos->pos.y = constants::cameraSize.y;
   }

   for (auto ent : ctxt->world->system->entitiesWithComponent<DieOffscreenComponent>())
   {
      auto pos = ent.get<PositionComponent>();
      if (!pos) continue;


      if (pos->pos.x < 0 || pos->pos.x > constants::cameraSize.x ||
         pos->pos.y < 0 || pos->pos.y > constants::cameraSize.y)
      {
         ent.create<MarkedForDeletionComponent>();
      }
   }


   float padX = constants::cameraSize.x * 0.25f;
   float padY = constants::cameraSize.y * 0.25f;
   for (auto ent : ctxt->world->system->entitiesWithComponent<DiePaddedOffscreenComponent>())
   {
      auto pos = ent.get<PositionComponent>();
      if (!pos) continue;


      if (pos->pos.x < -padX || pos->pos.x > constants::cameraSize.x + padX ||
          pos->pos.y < -padY || pos->pos.y > constants::cameraSize.y + padY)
      {
         ent.create<MarkedForDeletionComponent>();
      }
   }

   for (auto ent : ctxt->world->system->entitiesWithComponent<TimedDeathComponent>())
   {
      if (ctxt->currentTick == ent.get<TimedDeathComponent>()->deleteTick)
      {
         ent.create<MarkedForDeletionComponent>();
      }
   }
   
   destroyMarkedForDeletion(ctxt->world);
}

bool entitiesCollide(Entity e1, Entity e2)
{
   //just do simple, centered box collision for now....
   auto p1 = e1.get<PositionComponent>();
   auto p2 = e2.get<PositionComponent>();

   if (!p1 || !p2) return false;  //unhandled case?


   auto s1 = e1.get<SizeComponent>();
   auto s2 = e2.get<SizeComponent>();
   auto r1 = e1.get<RadiusComponent>();
   auto r2 = e2.get<RadiusComponent>();
   auto l1 = e1.get<LaserComponent>();
   auto l2 = e2.get<LaserComponent>();

   //do all combinations of checks (circles, boxes, points.)
   if (r1)
   {
      auto c1 = Circle{ p1->pos, r1->radius };
      if (r2) 
      {
         auto c2 = Circle{ p2->pos, r2->radius };

         return isColliding(c1, c2);
      }
      else if (s2)
      {
         auto b2 = Box::fromCenterExtents(p2->pos, s2->sz);

         return isColliding(b2, c1);
      }
      else if (l2)
      {
         Line laserLine;
         laserLine.start = p2->pos;
         laserLine.end = l2->direction;
         laserLine.end *= 2300;
         laserLine.end += laserLine.start;
         return distance(laserLine, p1->pos) < r1->radius;
      }
      else //just a point
      {
         return isColliding(p2->pos, c1);
      }
   }
   else if (s1)
   {
      if (l2) return false;
      auto b1 = Box::fromCenterExtents(p1->pos, s1->sz);
      if (r2)
      {
         auto c2 = Circle{ p2->pos, r2->radius };

         return isColliding(b1, c2);
      }
      else if (s2)
      {
         auto b2 = Box::fromCenterExtents(p2->pos, s2->sz);

         return isColliding(b1, b2);
      }
      else //just a point
      {
         return isColliding(p2->pos, b1);
      }
   }
   else if (l1)
   {
      //only laser vs. radius handled for now...
      if (!r2) return false;
      Line laserLine;
      laserLine.start = p1->pos;
      laserLine.end = l1->direction;
      laserLine.end *= 2300;
      laserLine.end += laserLine.start;
      return distance(laserLine, p2->pos) < r2->radius;
   }
   
   
   return false;
}

void handleCollisions(BulletHellContext* ctxt)
{
   //get enemies that collide with bullets
   auto enemies = ctxt->world->system->entitiesWithComponent<EnemyComponent>();
   auto bullets = ctxt->world->system->entitiesWithComponent<PlayerBulletComponent>();
    
   //just do a really stupid brute force n x m implementation right now   
   for (auto&& enemy : enemies)
   {
      for (auto&& bullet : bullets)
      {
         if (entitiesCollide(enemy, bullet))
         {
            bullet.create<MarkedForDeletionComponent>();

            if (enemy.get<InvinciblityComponent>()) continue; //ignore getting shot if invicible.

            if (auto hp = enemy.get<HealthComponent>())
            {
               if ((hp->hp -= 1) <= 0)
               {
                  //ENEMY IS DEAD
                  if (!enemy.get<LivesWithNoHPComponent>())
                  {
                     enemy.create<MarkedForDeletionComponent>();
                  }                  
               }
            }
            else //single-shot baddie got shot
            {
               enemy.create<MarkedForDeletionComponent>();
            }          
            
         }
      }
   }

   //player hit?
   auto enemyBullets = ctxt->world->system->entitiesWithComponent<EnemyBulletComponent>();
   auto players = ctxt->world->system->entitiesWithComponent<PlayerComponent>();

   for (auto&& player : players)
   {
      for (auto&& bullet : enemyBullets)
      {
         if (bullet.get<NoDamageComponent>()) continue;
         if (entitiesCollide(player, bullet))
         {
            //YOU GOT HIT YOU SUCK
            Sound::createSample("sfx/blast.wav").play().detach();

            //delete, but only if not a laser.
            //if (!bullet.get<LaserComponent>())
            {
               bullet.create<MarkedForDeletionComponent>();
            }
            // global bullet cancel will happen later....
         }
      }
   }

   destroyMarkedForDeletion(ctxt->world);
}

/* some speciality updates...*/

void updateMeshOfLightAndDark(BulletHellContext* ctxt)
{
   for (auto ent : ctxt->world->system->entitiesWithComponent<MeshOfLightAndDarkSpawnComponent>())
   {
      auto spawnTime = ent.get<SpawnTimeComponent>();
      if (!spawnTime)
      {
         ent.create<SpawnTimeComponent>(ctxt->currentTick);
         return;
      }
      auto time = ctxt->currentTick - spawnTime->spawnTick;

      if (!(time % 6))
      {
         bool mirrored = (time / 6) % 2 == 1;
         auto angle = rectToPolar(ent.get<VelocityComponent>()->vel);
         float offset = randomFloat(75, 105);
         if (mirrored)
         {
            offset = -offset;
         }
         angle += degToRad(offset);

         //fire a laser from this bullet in this direction.
         Entity laser(ctxt->world);

         laser.create<PositionComponent>(ent.get<PositionComponent>()->pos);
         laser.create<LaserComponent>(polarToRect(angle), 0);

         ResizingLaserComponent rescomp;
         rescomp.startTick = ctxt->currentTick;
         rescomp.fullWidthTick = ctxt->currentTick + 40;
         rescomp.startFadeTick = ctxt->currentTick + 120;
         rescomp.endTick = ctxt->currentTick + 135;

         rescomp.maxWidth = 16;

         laser.set(std::move(rescomp));
         laser.create<SpriteComponent>("png/babble_red.png");
         laser.create<EnemyBulletComponent>();
      }
   }
}


void setInterpolationVelocities(BulletHellContext* ctxt)
{
   for (auto ent : ctxt->world->system->entitiesWithComponent<ArcInterpolationComponent>())
   {
      //find the prev position, and the new position, subtract and set as velocity.
      auto interp = ent.get<ArcInterpolationComponent>();
      auto tick = ctxt->currentTick;
      if (tick < interp->startTick || tick > interp->endTick)
      {
         ent.remove<VelocityComponent>(); //invalid!
         continue;
      }
      //auto prevTick = tick-1;
      //float t1 = ((prevTick - (interp->startTick - 1)))/(float)(interp->endTick - interp->startTick + 1);
      float t=  ((tick - (interp->startTick - 1)))/(float)(interp->endTick - interp->startTick + 1);

      //auto p1 = polarToRect(cosInterp(interp->startAngle, interp->endAngle, t1), lerp(interp->startDistance, interp->endDistance, t1));
      auto p = polarToRect(cosInterp(interp->startAngle, interp->endAngle, t), lerp(interp->startDistance, interp->endDistance, t));
      p += interp->center;

      auto vel = p;
      vel -= ent.get<PositionComponent>()->pos;

      ent.create<VelocityComponent>(vel); //ez!
   }
}

void updatePhysics(BulletHellContext* ctxt)
{
   updateMeshOfLightAndDark(ctxt);
   updatePlayerVelocity(ctxt);

   setInterpolationVelocities(ctxt);

   float physTime = 1.0f / constants::physicsTicksPerTick;

   for (int i = 0; i < constants::physicsTicksPerTick; ++i)
   {
      //apply acceleration (crappy euler, todo: RK4 or whatever)
      for (auto ent : ctxt->world->system->entitiesWithComponent<AccelerationComponent>())
      {
         auto vel = ent.get<VelocityComponent>();
         if (!vel) vel = ent.create<VelocityComponent>();
         auto acc = ent.get<AccelerationComponent>()->accel;
         acc *= physTime;
         vel->vel += acc;
      }

      //cap speed, for everything 
      for (auto ent : ctxt->world->system->entitiesWithComponent<MaxSpeedComponent>())
      {
         auto vel = ent.get<VelocityComponent>();
         if (!vel) continue;
         auto maxSpeed = ent.get<MaxSpeedComponent>()->maxSpeed;
         auto curSpeed = length(vel->vel);
         if (curSpeed > maxSpeed)
         {
            vel->vel *= (maxSpeed / curSpeed);
         }
      }

      for (auto ePair : ctxt->world->system->componentsOfType<VelocityComponent>())
      {
         Entity e(ePair.entity, ctxt->world->system);

         auto pos = e.get<PositionComponent>();
         auto vel = e.get<VelocityComponent>();

         if (!pos) continue;
         auto velT = vel->vel;
         velT *= physTime;

         pos->pos += velT;
      }

      enforceWorldBoundaries(ctxt);

      handleCollisions(ctxt);
   }
}

template <typename T>
void drawSpritesWithComponent(BulletHellContext* ctxt)
{
   auto window = ctxt->window;
   for (auto enemy : ctxt->world->system->entitiesWithComponent<T>())
   {
      auto pos = enemy.get<PositionComponent>();

      if (!pos) continue;

      Sprite* sprite = nullptr;
      if (auto sprComp = enemy.get<SpriteComponent>())
      {
         sprite = GetSprite(sprComp->sprite);
      }
      else
      {
         continue; //sprite is required now
      }

      if (auto laser = enemy.get<LaserComponent>())
      {
         float rotation = radToDeg(rectToPolar(laser->direction));
         float length = 2300; //longer than sqrtf(1920*1920 + 1080*1080);
         float width = laser->width;

         Vec2 center = laser->direction;
         center *= length / 2;
         center += pos->pos;

         window->drawSpriteStretched(center.x, center.y, length, width, 0, 0, rotation, sprite);
      }
      else
      {
         float angle = 0.0f;

         if (enemy.get<AlignToVelocityComponent>())
         {
            if (auto vel = enemy.get<VelocityComponent>())
            {
               angle = radToDeg(rectToPolar(vel->vel));
            }            
         }
         window->drawSprite(pos->pos.x, pos->pos.y, 0, 0, angle, sprite);
      }
   }
}

void drawSpritesHacked(BulletHellContext* ctxt)
{
   //doing this gives us a rough z-order.  Might want a dedicated Z-order component, though.
   drawSpritesWithComponent<EnemyComponent>(ctxt);
   drawSpritesWithComponent<PlayerComponent>(ctxt);
   drawSpritesWithComponent<EnemyBulletComponent>(ctxt);
   drawSpritesWithComponent<PlayerBulletComponent>(ctxt);
}

class LevelRun : public BulletHellState
{
public:
   LevelRun(BulletHellContext* ctxt_in, Level level)
      : ctxt(ctxt_in), m_level(std::move(level))
   {

   }

   bool checkWin()
   {
      return (m_level.currentSection == m_level.sections.size());
   }
   void handleWin()
   {
      int* exitGracefully = nullptr;
      *exitGracefully = 'LOL!';
      //todo: transition to a score screen or a next level or literally anything else
   }

   void updateInput()
   {
      auto players = ctxt->world->system->entitiesWithComponent<PlayerComponent>();
      for (auto&& ev : inputFlushQueue())
      {
         if (ev.type == KeyPressType::Repeated) continue; //don't care
         bool isDown = ev.type == KeyPressType::Down;

         //more bad programming!
         //needs to be generalized for remapping, joysticks, etc
         for (auto&& player : players)
         {
            auto pc = player.get<PlayerComponent>();
            switch (ev.key)
            {
            case KeyType::Left: pc->left = isDown; break;
            case KeyType::Right: pc->right = isDown; break;
            case KeyType::Down: pc->down = isDown; break;
            case KeyType::Up: pc->up = isDown; break;

            case KeyType::LeftShift: case KeyType::RightShift: pc->focused = isDown; break;  //kind of awkward - the shifts affect each other.  refcount?

            case 'Z': pc->firing = isDown; break;
            }
         }
      }

      //TODO: move this somewhere nicer?

   }

   void fireWeapons()
   {
      for (auto&& player : ctxt->world->system->entitiesWithComponent<PlayerComponent>())
      {
         auto pc = player.get<PlayerComponent>();
         if (pc->firing) fireAllWeapons(ctxt, player);
      }
      for (auto&& enemy : ctxt->world->system->entitiesWithComponent<EnemyComponent>())
      {
         fireAllWeapons(ctxt, enemy);
      }
   }

   //todo - this should probably be in a nicer, shared spot... hack it in here for now.
   void gameTick()
   {
      updateInput();
      fireWeapons();
      updateLasers();
      updatePhysics(ctxt);
            
      ctxt->audio->update();
   }
   void drawLevel()
   {      
      ctxt->window->startDraw();

      drawSpritesHacked(ctxt);

      ctxt->window->endDraw();
   }

   virtual void update() override
   {
      //update the sections...
      for (;;)
      {
         if (m_level.currentSection >= 0) //have we actually started the level?
         { 
            auto& section = m_level.sections[m_level.currentSection];
            section->onUpdate();
            if (!section->finished())
            {
               gameTick();
               drawLevel();
               return;  //more to do laterererer
            }
         }
         ++m_level.currentSection;
         if (checkWin()) break;
         m_level.sections[m_level.currentSection]->onEnter();
      }

      //you won
      handleWin();
      return;
   }
private:
   
   BulletHellContext* ctxt;
   Level m_level;
};


std::unique_ptr<BulletHellState> runLevelStateCreate(BulletHellContext* ctxt, Level level)
{
   return std::unique_ptr<BulletHellState>(new LevelRun(ctxt, std::move(level)));
}