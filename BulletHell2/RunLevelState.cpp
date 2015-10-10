#include "RunLevelState.hpp"

#include "window.hpp"
#include "entity.hpp"
#include "entitySystemView.hpp"
#include "components.hpp"


//placeholder...
void updatePhysics(BulletHellContext* ctxt)
{
   for (auto ePair : ctxt->world->system->componentsOfType<VelocityComponent>())
   {
      Entity e(ePair.entity, ctxt->world->system);

      auto pos = e.get<PositionComponent>();
      auto vel = e.get<VelocityComponent>();
      
      if (!pos) continue;

      pos->pos += vel->vel;
   }
}

//HASKC AKLHC GAC HACK HACK  TODO: ANYTHING ELSE
void drawSpritesHacked(BulletHellContext* ctxt)
{
   for (auto enemyPair : ctxt->world->system->componentsOfType<EnemyComponent>())
   {
      Entity enemy(enemyPair.entity, ctxt->world->system);

      auto pos = enemy.get<PositionComponent>();
      auto sz = enemy.get<SizeComponent>();
      if (!pos || !sz) continue;

      ctxt->window->drawSpriteHACK(pos->pos, sz->sz);
   }
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

   void drawLevel()
   {
      updatePhysics(ctxt);

      //todo - this should probably be in a nicer, shared spot... hack it in here for now.
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