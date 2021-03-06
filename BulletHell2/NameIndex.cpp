#include "NameIndex.hpp"
#include "Components.hpp"
#include "BulletHell2.hpp"
#include "ManagerView.hpp"
#include "EntitySystemView.hpp"

void NameIndex::onNewEntity(Entity entity)
{
	if (auto name = entity.get<NameComponent>()) m_entityLookup[name->name] = entity;
}
void NameIndex::onDestroyEntity(Entity entity)
{
	if (auto name = entity.get<NameComponent>()) m_entityLookup.erase(name->name);
}
Entity NameIndex::getEntity(std::string characterName)
{
	auto iter = m_entityLookup.find(characterName);
	if (iter != m_entityLookup.end()) return iter->second;
	return nullEntity;
}
 

void entitySetName(EntitySystemView* ctxt, Entity e, std::string const& name)
{
	e.create<NameComponent>(name);
	ctxt->nameIndex->onNewEntity(e);
}
Entity entityGetByName(EntitySystemView* ctxt, std::string const& name)
{
	return ctxt->nameIndex->getEntity(name);
}
