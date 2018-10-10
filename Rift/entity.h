#pragma once

#include <queue>
#include <memory>
#include <assert.h>
#include <functional>
#include <unordered_map>
#include "utility/functions.h"
#include "utility/containers.h"
#include "utility/identification.h"

namespace rift {
	class EntityManager;

    // The Entity class
	// a handle for an rift::ID
	class Entity final {
	public:

		// Create an invalid entity
		Entity();

		// Fetch the entity's rift::ID
		rift::ID id() const noexcept;

		// Checks if the entity's rift::ID is valid
		bool valid() const noexcept;

		operator bool() const noexcept;
		
		// Invalidate this entity and all other entities that share the same rift::ID
		void destroy() const noexcept;

		// Fetch the entity's ComponentMask
		rift::ComponentMask component_mask() const noexcept;

		// Adds a component of type C to the entity's list of components
		// Note: if the entity already owns a component of type C, 
		// then that component is overwritten by the new component created in this function call.
		template <class C, class ...Args>
		void add(Args&& ...args) const noexcept;

		// Removes a component of type C from the entity's list of components
		// Note: an assertion is made that the entity owns a component of type C
		template <class C>
		void remove() const noexcept;

		// Checks if a component of type C is a member of the entity's list of components
		template <class C>
		bool has() const noexcept;

		// Fetches the component of type C from the entity's list of components
		// Note: an assertion is made that the entity owns a component of type C
		template <class C>
		C &get() const noexcept;

		friend bool operator<(const rift::Entity& a, const rift::Entity& b) noexcept;
		friend bool operator>(const rift::Entity& a, const rift::Entity& b) noexcept;
		friend bool operator==(const rift::Entity& a, const rift::Entity& b) noexcept;
		friend bool operator!=(const rift::Entity& a, const rift::Entity& b) noexcept;

	private:
		friend class EntityManager;
		// Only EntityManagers are permitted to create valid entity handles
		Entity(EntityManager *em, rift::ID id);
		// The manager that created this entity
		EntityManager* mgr;
		// The entity's id. Only valid while the version is valid
		rift::ID m_id;
	
	};

	// The EntityManager class
	// Manages Entity::Records and the creation of Entity handles
	class EntityManager final {
	public:

		EntityManager() = default;
		EntityManager(const EntityManager&) = delete;
		EntityManager& operator=(const EntityManager&) = delete;

		// Generate a new Entity::Record and return an Entity handle to its rift::ID
		Entity create_entity() noexcept;

		// Returns the number of rift::IDs that associate with an instance of each component type
		template <class First, class... Rest>
		std::size_t count_entities_with() const noexcept;

		// The function applies 'fun' onto Entities whose rift::ID associates with an instance of 
		// each component type given as a template parameter.
		template <class First, class... Rest>
		void entities_with(std::function<void(const Entity&)>&& fun) noexcept;

#ifndef RIFT_DEBUG
	private:
#endif // !RIFT_DEBUG

		friend class Entity;
		
		// Assign a component to the master rift::ID of id
		template <class C, class... Args>
		void add(const rift::ID& id, Args&& ...args) noexcept;
		
		// Remove a component from the master rift::ID of id
		template <class C>
		void remove(const rift::ID& id) noexcept;

		// Check if the master rift::ID of id owns a component of type C
		template <class C>
		bool has(const rift::ID& id) noexcept;

		// Returns the component of type C associate with the master rift::ID
		template <class C>
		C &get(const rift::ID& id) noexcept;

		// Checks if the id is of the same version as its master rift::ID
		bool valid_id(const rift::ID& id) const noexcept;

		// Invalidate id and all copies of it by refreshing the EntityRecord id maps to
		void invalidate_id(const rift::ID& id) noexcept;
		
		// Returns the ComponentMask associated with the master rift::ID of id
		ComponentMask component_mask(const rift::ID& id) const noexcept;

		// Returns the component pool for component type C
		// Note: 
		// - If the component type does not exist, it is created then returned.
		template <class C>
		std::shared_ptr<Pool<C>> pool_for() noexcept;

	private:
	
		// The collection of ComponentMasks'
		std::vector<ComponentMask> masks;

		// The collection of ID versions
		std::vector<std::size_t> id_versions;
		
		// The queue of reusable rift::IDs
		std::queue<rift::ID> reusable_ids;

		using ComponentFamily = std::size_t;

		// The pools of components
		std::unordered_map<ComponentFamily, std::shared_ptr<BasePool>> component_pools;
	};

	template<class C, class ...Args>
	inline void Entity::add(Args && ...args) const noexcept
	{
		assert(valid() && "Cannot add components to an invalid entity");
		mgr->add<C>(m_id, std::forward<Args>(args)...);
	}

	template<class C>
	inline void Entity::remove() const noexcept
	{
		assert(valid() && "Cannot remove components from an invalid entity");
		assert(has<C>() && "Cannot remove a component from an entity if it does not own it");
		mgr->remove<C>(m_id);
	}

	template<class C>
	inline bool Entity::has() const noexcept
	{
		assert(valid() && "Cannot check if an invalid entity owns a component");
		return mgr->has<C>(m_id);
	}

	template<class C>
	inline C & Entity::get() const noexcept
	{
		assert(valid() && "Cannot fetch components for an invalid entity");
		assert(has<C>() && "Cannot fetch a component for an entity if it does not own it");
		return mgr->get<C>(m_id);
	}

	template <class First, class... Rest>
	inline std::size_t EntityManager::count_entities_with() const noexcept
	{
		auto signature = signature_for<Rest...>();
		signature.set(First::family());
		std::size_t count = 0;
		for (auto& mask : masks) {
			if ((mask & signature) == signature) {
				++count;
			}
		}
		return count;
	}

	template <class First, class... Rest>
	inline void EntityManager::entities_with(std::function<void(const Entity&)>&& fun) noexcept
	{
		auto signature = signature_for<Rest...>();
		signature.set(First::family());
		for (std::size_t i = 0; i < masks.size(); i++) {
			if ((masks[i] & signature) == signature) {
				fun(Entity(this, rift::ID(i, id_versions[i])));
			}
		}
		
	}

	template<class C, class ...Args>
	inline void EntityManager::add(const rift::ID& id, Args && ...args) noexcept
	{
		auto pool = pool_for<C>();
		auto size = pool->size();
		if (size <= id.index()) {
			pool->allocate(id.index() - size + 1);
		}
		pool->at(id.index()) = C(std::forward<Args>(args)...);
		masks.at(id.index()).set(C::family());	
	}

	template<class C>
	inline void EntityManager::remove(const rift::ID& id) noexcept
	{
		masks.at(id.index()).reset(C::family());
	}

	template<class C>
	inline bool EntityManager::has(const rift::ID& id) noexcept
	{
		return masks.at(id.index()).test(C::family());
	}

	template<class C>
	inline C & EntityManager::get(const rift::ID& id) noexcept
	{
		return std::static_pointer_cast<Pool<C>>(component_pools.at(C::family()))->at(id.index());
	}

	template<class C>
	inline std::shared_ptr<Pool<C>> EntityManager::pool_for() noexcept
	{
		if (component_pools.find(C::family()) == component_pools.end())
			component_pools.emplace(C::family(), std::make_shared<Pool<C>>(masks.size()));
		return std::static_pointer_cast<Pool<C>>(component_pools.at(C::family()));
	}

}
