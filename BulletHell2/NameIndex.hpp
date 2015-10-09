#pragma once

#include "entity.hpp"
#include <string>
#include <unordered_map>

struct BulletHellContext;

class NameIndex : public EntitySystemListener
{
public:
	void onNewEntity(Entity entity);
	void onDestroyEntity(Entity entity);
	Entity getEntity(std::string characterName);
private:
	std::unordered_map<std::string, Entity> m_entityLookup;
};

//use this to update the name index.
void entitySetName(BulletHellContext* ctxt, Entity e, std::string const& name);
Entity entityGetByName(BulletHellContext* ctxt, std::string const& name);