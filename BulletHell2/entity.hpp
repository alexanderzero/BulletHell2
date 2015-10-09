#pragma once

#include <memory>
#include "Component.hpp"

class EntitySystem;

//entities are value types.  copying does a shallow copy.  kill them with .destroy().
class Entity
{
public:
	Entity(EntitySystem* e = nullptr);  //if not entity system, nothing works, but you can re-assign as a different entity later.
	void destroy(); //it's easy to leak in this engine by not destroying... do careful!
	void sendUpdate();

	template <typename T>
	T* get()
	{
		return (T*)parent->getComponent(ClassComponentVTable<T>::get(), id);
	}

	template <typename T>
	void set(T&& component)
	{
		*(T*)parent->addComponent(ClassComponentVTable<typename std::decay<T>::type>::get(), id) = std::forward<T>(component);
	}

	template <typename T>
	T* create()
	{
		return (T*)parent->addComponent(ClassComponentVTable<T>::get(), id);
	}

	template <typename T>
	void remove()
	{
		parent->removeComponent(ClassComponentVTable<T>::get(), id);
	}

private:
	EntityID id;
	EntitySystem* parent;
};

class EntitySystem
{
public:

	EntitySystem();
	~EntitySystem();

	void updateID(EntityID id);

	//low-level memory management.
	EntityID allocID();
	void freeID(EntityID id);

	void* addComponent(ComponentVTable* vtable, EntityID id);
	void* getComponent(ComponentVTable* vtable, EntityID id);
	void removeComponent(ComponentVTable* vtable, EntityID id);

	void* rawComponentData(ComponentVTable* vtable, size_t& outCount);


	//high-level helpers.
	template <typename T>
	struct ComponentView
	{
		ComponentPair<T>* first, *last;
		ComponentPair<T>* begin() const { return first; }
		ComponentPair<T>* end() const { return last; }
	};

	template <typename T>
	ComponentView<T> componentsOfType()
	{
		ComponentView<T> out;
		size_t count;
		out.first = (ComponentPair<T>*)rawComponentData(ClassComponentVTable<T>::get(), count);
		out.last = out.first + count;
		return out;
	}

private:
	class Impl;
	std::unique_ptr<Impl> pImpl;
};
