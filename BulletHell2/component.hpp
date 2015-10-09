#pragma once


#include <vector>

typedef int EntityID;

extern int g_ComponentTypeCounter;

template <typename T>
struct ComponentPair
{
	ComponentPair() {}
	ComponentPair(EntityID id, T&& comp) : entity(id), component(std::move(comp)) {}
	EntityID entity;
	T component;
};

class ComponentVTable
{
public:
	ComponentVTable()
	{
		componentType = g_ComponentTypeCounter++;
	}
	virtual void destroyPool(void* pool) = 0;
	virtual void* indexIntoPool(void*& pool, int idx) = 0;
	virtual void* addToPool(void*& pool, EntityID id, int& out) = 0;
	virtual void  removeFromPool(void*& pool, int idx, std::vector<int>& indirection) = 0;
	virtual void setEntity(void*& pool, int idx, EntityID entity) = 0;
	virtual EntityID getEntity(void*& pool, int idx) = 0;
	virtual void* rawData(void*& pool) = 0;
	virtual size_t heldComponents(void*& pool) = 0;

	int componentType;
};


template <typename T>
class ClassComponentVTable : public ComponentVTable
{
public:
	static ClassComponentVTable* get()
	{
		static ClassComponentVTable instance;
		return &instance;
	}
	typedef std::vector<ComponentPair<T>> PoolType;
	PoolType& getPool(void* pool)
	{
		return *(PoolType*)(pool);
	}
	virtual void destroyPool(void* pool)
	{
		if (pool) delete &getPool(pool);
	}
	virtual void* indexIntoPool(void*& pool, int idx)
	{
		if (!pool || !idx) return nullptr;
		--idx;
		auto& p = getPool(pool);
		return &p[idx].component;
	}
	virtual void* addToPool(void*& pool, EntityID id, int& idxOut)
	{
		if (!pool) pool = new PoolType;
		auto& p = getPool(pool);
		p.push_back(ComponentPair<T>(id, T()));
		idxOut = (int)p.size();
		return &p.back().component;
	}
	virtual void removeFromPool(void*& pool, int idx, std::vector<int>& indirection)
	{
		if (!pool || !idx) return;
		--idx;
		auto& p = getPool(pool);
		indirection[p[idx].entity] = 0; //kill this indirection.
		if (idx != p.size() - 1)
		{
			p[idx] = std::move(p.back());
			//update indirection...
			indirection[p[idx].entity] = idx;
		}
		p.pop_back();
	}
	virtual void setEntity(void*& pool, int idx, EntityID entity)
	{
		if (!pool || !idx) return;
		--idx;
		auto& p = getPool(pool);
		p[idx].entity = entity;
	}
	virtual EntityID getEntity(void*& pool, int idx)
	{
		if (!pool || !idx) return 0;
		--idx;
		auto& p = getPool(pool);
		return p[idx].entity;
	}
	virtual void* rawData(void*& pool) //return as std::pair<EntityID, T>
	{
		if (!pool) return nullptr;
		auto& p = getPool(pool);
		return p.data();
	}
	virtual size_t heldComponents(void*& pool)
	{
		if (!pool) return 0;
		auto& p = getPool(pool);
		return p.size();
	}
};