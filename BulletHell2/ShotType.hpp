#pragma once

#include <string>

//what's in a shot type?
//a lot of arbitrary parameters and variable parameters!
//on a player, you also have to keep track of cooldown, which is set by the weapon.
//to be more generic, things work this way - 


class Entity;
typedef Entity ShotType;

//e.g. - shot types are a separate database.  We reference this database as shot types!
//we re-use certain components in shot types to mean different things.


struct BulletHellContext;
struct EntitySystemView;
void hackBuildTestShotTypes(EntitySystemView*& shotTypes);

struct Shot;

class IShotType
{
public:
   virtual ~IShotType() {}
   virtual void fire(BulletHellContext* context, Entity ent, Shot* shot) = 0;
   virtual int getCooldown() = 0;
};

int getShotCooldown(ShotType shot);
//IShotType* getShotType(BulletHellContext* context, ShotType shotType);
IShotType* getShotType(BulletHellContext* context, std::string const& shotType);
