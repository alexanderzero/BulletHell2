#pragma once

#include <memory>
#include "Component.hpp"

class EntitySystem;
struct EntitySystemView;

//entities are value types.  copying does a shallow copy.  kill them with .destroy().
class Entity
{
public:
	Entity(EntitySystem* e = nullptr);  //if not entity system, nothing works, but you can re-assign as a different entity later.
   Entity(EntitySystemView* eView);
   Entity(EntityID id_in, EntitySystem* parent_in);
	void destroy(); //it's easy to leak in this engine by not destroying... do careful!
	void update();

	template <typename T>
	T* get()
	{
		if (!parent) return nullptr;
		return (T*)parent->getComponent(ClassComponentVTable<T>::get(), id);
	}

	template <typename T>
	void set(T&& component)
	{
      typedef typename std::decay<T>::type BaseType;
		if (!parent) return;
		*(BaseType*)parent->addComponent(ClassComponentVTable<BaseType>::get(), id) = std::forward<T>(component);
	}

	template <typename T, typename... Args>
	T* create(Args&&... args)
	{
		if (!parent) return nullptr;
		return &(*(T*)parent->addComponent(ClassComponentVTable<T>::get(), id) = T(std::forward<Args>(args)...));
	}

	template <typename T>
	void remove()
	{
		if (!parent) return;
		parent->removeComponent(ClassComponentVTable<T>::get(), id);
	}

   bool operator==(Entity const& rhs) const
   {
      return parent == rhs.parent && id == rhs.id;
   }
   bool operator!=(Entity const& rhs) const
   {
      return !(*this == rhs);
   }
   bool operator<(Entity const& rhs) const
   {
      if (parent < rhs.parent) return true;
      if (parent > rhs.parent) return false;
      return id < rhs.id;
   }

	explicit operator bool() const {

		return parent != nullptr;
	}
   EntityID getID()
   {
      return id;
   }
private:
	EntityID id;
	EntitySystem* parent;
};

extern Entity nullEntity; //special entity marking it as "false"


class EntitySystemListener
{
public:
	virtual void onNewEntity(Entity e) = 0;
	virtual void onDestroyEntity(Entity e) = 0;
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

	void installListener(EntitySystemListener* listener);
	void uninstallListener(EntitySystemListener* listener);

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

   template <typename T>
   std::vector<Entity> entitiesWithComponent()
   {
      std::vector<Entity> out;
      auto search = componentsOfType<T>();
      out.reserve(search.last - search.first);
      for (auto&& ePair : search)
      {
         out.push_back(Entity(ePair.entity, this));
      }
      return out;
   }

private:
	class Impl;
	std::unique_ptr<Impl> pImpl;
};

