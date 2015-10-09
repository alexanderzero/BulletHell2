
//ENTITY.CPP

#include "Entity.hpp"
#include <vector>

Entity nullEntity;

int g_ComponentTypeCounter;

Entity::Entity(EntitySystem* e)
{
	parent = e;
	if (parent) id = parent->allocID();
}
void Entity::destroy()
{
	if (parent) parent->freeID(id);
}
void Entity::update()
{
	parent->updateID(id);
}


class ComponentPool
{
public:
	std::vector<int> indirectionPool;
	void* pool;
	ComponentVTable* vtable;
	~ComponentPool()
	{
		vtable->destroyPool(pool);
	}
	int& indirect(EntityID id)
	{
		if (id >= (EntityID)indirectionPool.size())
		{
			indirectionPool.resize(id + 1, 0);
		}
		return indirectionPool[id];
	}
	void* add(EntityID id)
	{
		auto& idx = indirect(id);
		if (!idx)
		{
			return vtable->addToPool(pool, id, idx);
		}
		else
		{
			return vtable->indexIntoPool(pool, idx);
		}
	}
	void* get(EntityID id)
	{
		auto idx = indirect(id);
		if (!idx) return nullptr;
		return vtable->indexIntoPool(pool, idx);
	}
	void remove(EntityID id)
	{
		auto& idx = indirect(id);
		if (!idx) return;
		vtable->removeFromPool(pool, idx, indirectionPool);
	}
	size_t size()
	{
		return vtable->heldComponents(pool);
	}
	void* data()
	{
		return vtable->rawData(pool);
	}
};

class EntitySystem::Impl
{
public:
	Impl()
	{
		maxID = 0;
	}
	~Impl()
	{
		for (auto&& pool : componentPools)
		{
			if (pool) delete pool;
		}
	}

	std::vector<EntitySystemListener*> listeners;
	std::vector<EntityID> IDFreeList;
	EntityID maxID;

	ComponentPool* getPool(ComponentVTable* vtable)
	{
		auto idx = vtable->componentType;
		if (idx >= (int)componentPools.size()) componentPools.resize(idx + 1, nullptr);
		auto& out = componentPools[idx];
		if (!out)
		{
			out = new ComponentPool;
			out->vtable = vtable;
			out->pool = nullptr;
		}
		return out;
	}
	int allocID()
	{
		if (IDFreeList.empty()) return maxID++;
		auto out = IDFreeList.back();
		IDFreeList.pop_back();
		return out;
	}
	void freeID(EntityID id)
	{
		for (auto&& pool : componentPools)
		{
			if (pool) pool->remove(id);
		}
		IDFreeList.push_back(id);
	}
	std::vector<ComponentPool*> componentPools;
};



EntitySystem::EntitySystem()
	: pImpl(new Impl)
{

}
EntitySystem::~EntitySystem() {}

void EntitySystem::updateID(EntityID id)
{

}

//low-level memory management.
EntityID EntitySystem::allocID()
{
	return pImpl->allocID();
}
void EntitySystem::freeID(EntityID id)
{
	pImpl->freeID(id);
}

void* EntitySystem::addComponent(ComponentVTable* vtable, EntityID id)
{
	auto pool = pImpl->getPool(vtable);
	return pool->add(id);
}
void* EntitySystem::getComponent(ComponentVTable* vtable, EntityID id)
{
	auto pool = pImpl->getPool(vtable);
	return pool->get(id);
}
void EntitySystem::removeComponent(ComponentVTable* vtable, EntityID id)
{
	auto pool = pImpl->getPool(vtable);
	return pool->remove(id);
}

void* EntitySystem::rawComponentData(ComponentVTable* vtable, size_t& outCount)
{
	auto pool = pImpl->getPool(vtable);
	outCount = pool->size();
	return pool->data();
}
void EntitySystem::installListener(EntitySystemListener* listener)
{
	pImpl->listeners.push_back(listener);
}
void EntitySystem::uninstallListener(EntitySystemListener* listener)
{
	auto iter = std::find(begin(pImpl->listeners), end(pImpl->listeners), listener);
	if (iter != end(pImpl->listeners)) pImpl->listeners.erase(iter);
}