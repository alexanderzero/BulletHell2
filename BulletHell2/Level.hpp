#pragma once

#include <vector>
#include <memory>

//levels are made up of "sections"
//each section has a "pass criteria" which lets you move onto the next section.
//clear all sections, and you've cleared the level!
//currently this has no branching...  some branches might instantly return done, though, depending on game state.

//essentially, this is a state machine.
class LevelSection
{
public:
   ~LevelSection() {}
   virtual void onEnter() = 0;
   virtual void onUpdate() = 0;
   virtual bool finished() = 0;
};


//note - this is NOT a flyweight.  this needs to be built from a level factory, this has real-time data!
struct Level
{
   std::vector<std::unique_ptr<LevelSection> > sections;
   int currentSection=-1; //-1 == not run yet.
};


//hack, remove later
struct BulletHellContext;
Level testLevelCreate(BulletHellContext* context);