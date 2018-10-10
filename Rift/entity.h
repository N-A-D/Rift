#pragma once

#include <queue>
#include <memory>
#include <assert.h>
#include <functional>
#include <unordered_map>
#include "utility/functions.h"
#include "utility/containers.h"

namespace rift {
	class EntityManager;

    // The Entity class
	// a handle for an Entity::ID
	class Entity final {
	public:

		// An Entity::ID is an index into an array
		// An EntityManager determines if an Entity's index is valid
		// through the version of the Entity's id
		class ID {
		public:
			ID() = default;
			ID(std::uint32_t index, std::uint32_t version)
				: m_number(std::uint64_t(index) | std::uint64_t(version) << 32) {}
			operator std::uint64_t() const noexcept { return m_number; }
			std::uint32_t index() const noexcept { return m_number & 0xFFFFFFFFUL; }
			std::uint32_t version() const noexcept { return m_number >> 32; }
			std::uint64_t number() const noexcept { return m_number; }
		private:
			std::uint64_t m_number;
		};


		// Create an invalid entity
		Entity();

		// Fetch the entity's Entity::ID
		Entity::ID id() const noexcept;

		// Checks if the entity's Entity::ID is valid
		bool valid() const noexcept;

		operator bool() const noexcept;
		
		// Invalidate this entity and all other entities that share the same Entity::ID
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
		Entity(EntityManager *em, Entity::ID id);
		// The manager that created this entity
		EntityManager* mgr;
		// The entity's id. Only valid while the version is valid
		Entity::ID m_id;
	
	};

	// The EntityManager class
	// Manages Entity::Records and the creation of Entity handles
	class EntityManager final {
	public:

		EntityManager() = default;
		EntityManager(const EntityManager&) = delete;
		EntityManager& operator=(const EntityManager&) = delete;

		// Generate a new Entity::Record and return an Entity handle to its Entity::ID
		Entity create_entity() noexcept;

		// Returns the number of Entity::IDs that associate with an instance of each component type
		template <class First, class... Rest>
		std::size_t count_entities_with() const noexcept;

		// The function applies 'fun' onto Entities whose Entity::ID associates with an instance of 
		// each component type given as a template parameter.
		template <class First, class... Rest>
		void entities_with(std::function<void(const Entity&)>&& fun) noexcept;

#ifndef RIFT_DEBUG
	private:
#endif // !RIFT_DEBUG

		friend class Entity;
		
		// Assign a component to the master Entity::ID of id
		template <class C, class... Args>
		void add(const Entity::ID& id, Args&& ...args) noexcept;
		
		// Remove a component from the master Entity::ID of id
		template <class C>
		void remove(const Entity::ID& id) noexcept;

		// Check if the master Entity::ID of id owns a component of type C
		template <class C>
		bool has(const Entity::ID& id) noexcept;

		// Returns the component of type C associate with the master Entity::ID
		template <class C>
		C &get(const Entity::ID& id) noexcept;

		// Checks if the id is of the same version as its master Entity::ID
		bool valid_id(const Entity::ID& id) const noexcept;

		// Invalidate id and all copies of it by refreshing the EntityRecord id maps to
		void invalidate_id(const Entity::ID& id) noexcept;

		// Removes all components associated with the Entity::ID
		void delete_components_for(const Entity::ID& id) noexcept;
		
		// Returns the ComponentMask associated with the master Entity::ID of id
		ComponentMask component_mask_for(const Entity::ID& id) const noexcept;

		// Returns the component pool for component type C
		// Note: 
		// - If the component type does not exist, it is created then returned.
		template <class C>
		std::shared_ptr<Pool<C>> pool_for() noexcept;

	private:
		
		std::unordered_map<ComponentMask, Cache<Entity>> result_sets;

		// The collection of ComponentMasks'
		std::vector<ComponentMask> masks;

		// The collection of ID versions
		std::vector<std::size_t> id_versions;
		
		// The queue of reusable Entity::IDs
		std::queue<Entity::ID> reusable_ids;

		// The pools of component pools
		std::unordered_map<ComponentMask, std::shared_ptr<BasePool>> component_pools;
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
		ComponentMask signature;
		signature.set(First::family());
		signature |= signature_for<Rest...>();

		if (result_sets.find(signature) != result_sets.end()) {
			for (auto& entity : result_sets.at(signature))
				fun(entity);
		}
		else {
			Cache<Entity> entity_cache;
			for (std::size_t i = 0; i < masks.size(); i++) {
				if ((masks[i] & signature) == signature) {
					Entity e(this, Entity::ID(i, id_versions[i]));
					entity_cache.insert(e);
					fun(e);
				}
			}
			result_sets.emplace(signature, entity_cache);
		}
	}

	template<class C, class ...Args>
	inline void EntityManager::add(const Entity::ID& id, Args && ...args) noexcept
	{
		/*
		auto pool = pool_for<C>();
		auto size = pool->size();
		if (size <= id.index()) {
			pool->allocate(id.index() - size + 1);
		}
		pool->at(id.index()) = C(std::forward<Args>(args)...);
		auto mask = masks.at(id.index()).set(C::family());	
		*/

		auto pool = pool_for<C>();
		auto component = C(std::forward<Args>(args)...);
		pool->insert(id.index(), &component);
		auto mask = masks.at(id.index()).set(C::family());

		// Insert this entity into any result sets that match 
		Entity e(this, id);
		for (auto& pair : result_sets) {
			if ((mask & pair.first) == pair.first && !pair.second.has(e))
				pair.second.insert(e);
		}
	}

	template<class C>
	inline void EntityManager::remove(const Entity::ID& id) noexcept
	{
		Entity e(this, id);
		auto mask = e.component_mask();
		for (auto& pair : result_sets) {
			if (pair.first.test(C::family()) && ((mask & pair.first) == pair.first))/*&& pair.second.has(e))*/
				pair.second.remove(e);
		}
		pool_for<C>()->remove(id.index());
		masks.at(id.index()).reset(C::family());
	}

	template<class C>
	inline bool EntityManager::has(const Entity::ID& id) noexcept
	{
		return masks.at(id.index()).test(C::family());
	}

	template<class C>
	inline C & EntityManager::get(const Entity::ID& id) noexcept
	{
		//return std::static_pointer_cast<Pool<C>>(component_pools.at(C::family()))->at(id.index());
		return pool_for<C>()->at(id.index());
	}

	template<class C>
	inline std::shared_ptr<Pool<C>> EntityManager::pool_for() noexcept
	{
		if (component_pools.find(C::family()) == component_pools.end())
			component_pools.emplace(C::family(), std::make_shared<Pool<C>>());
		return std::static_pointer_cast<Pool<C>>(component_pools.at(C::family()));
	}

}
