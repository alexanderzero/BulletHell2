#pragma once

//what's in a shot type?
//a lot of arbitrary parameters and variable parameters!
//on a player, you also have to keep track of cooldown, which is set by the weapon.
//to be more generic, things work this way - 


class Entity;
typedef Entity ShotType;

//e.g. - shot types are a separate database.  We reference this database as shot types!
//we re-use certain components in shot types to mean different things.


struct EntitySystemView;
void hackBuildTestShotTypes(EntitySystemView*& shotTypes);
