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

void playerFireShot(BulletHellContext* ctxt, Entity player, Shot& shot)
{
   //get the actual shot from the context.
   auto shotType = entityGetByName(ctxt->shotTypes, shot.type);
   if (!shotType) return; //not a valid shot type?

   //manage cooldowns.
   shot.nextFireTime = ctxt->currentTick;
   if (auto cooldown = shotType.get<CooldownComponent>()) shot.nextFireTime += cooldown->ticks;

   //fire!
   if (auto shot = getShotType(ctxt, shotType)) shot->fire(player);
}
void playerFire(BulletHellContext* ctxt, Entity player)
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
      playerFireShot(ctxt, player, shot);
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

   std::vector<Entity> offscreenEntities;
   for (auto ent : ctxt->world->system->entitiesWithComponent<DieOffscreenComponent>())
   {
      auto pos = ent.get<PositionComponent>();
      if (!pos) continue;


      if (pos->pos.x < 0 || pos->pos.x > constants::cameraSize.x ||
         pos->pos.y < 0 || pos->pos.y > constants::cameraSize.y)
      {
         offscreenEntities.push_back(ent); //do this in two passes to be nice with iteration
      }
   }
   for (auto&& ent : offscreenEntities) ent.destroy();
}

void updatePhysics(BulletHellContext* ctxt)
{
   updatePlayerVelocity(ctxt);

   for (auto ePair : ctxt->world->system->componentsOfType<VelocityComponent>())
   {
      Entity e(ePair.entity, ctxt->world->system);

      auto pos = e.get<PositionComponent>();
      auto vel = e.get<VelocityComponent>();
      
      if (!pos) continue;

      pos->pos += vel->vel;
   }
   
   enforceWorldBoundaries(ctxt);
   
   //todo - handle collision cases here.
}

template <typename T>
void drawSpritesWithComponent(BulletHellContext* ctxt)
{
   for (auto enemy : ctxt->world->system->entitiesWithComponent<T>())
   {
      auto pos = enemy.get<PositionComponent>();
      auto sz = enemy.get<SizeComponent>();
      if (!pos || !sz) continue;

      ctxt->window->drawSprite(pos->pos.x, pos->pos.y, 0, 0);
   }
}

void drawSpritesHacked(BulletHellContext* ctxt)
{
   //doing this gives us a rough z-order.  Might want a dedicated Z-order component, though.
   drawSpritesWithComponent<EnemyComponent>(ctxt);
   drawSpritesWithComponent<PlayerComponent>(ctxt);
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
      for (auto&& player : players)
      {
         auto pc = player.get<PlayerComponent>();
         if (pc->firing) playerFire(ctxt, player);
      }
   }

   void drawLevel()
   {
      updateInput();

      updatePhysics(ctxt);

      //todo - this should probably be in a nicer, shared spot... hack it in here for now.
      ctxt->audio->update();
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