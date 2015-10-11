#pragma once

#include "entity.hpp"
#include <string>
#include <unordered_map>

class BulletHellContext;

class NameIndex
{
public:
	Entity getEntity(std::string characterName);
private:
	std::unordered_map<std::string, Entity> m_entityLookup;
};

//use this to update the name index.
void entitySetName(Entity e, std::string const& name);
Entity entityGetByName();