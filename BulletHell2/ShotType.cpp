#include "ShotType.hpp"

#include "EntitySystemView.hpp"
#include "entity.hpp"
#include "NameIndex.hpp"
#include "components.hpp"


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