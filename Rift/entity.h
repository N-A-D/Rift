#pragma once

#include <queue>
#include <memory>
#include <assert.h>
#include <functional>
#include "util/pool.h"
#include "component.h"
#include <unordered_map>
#include "details/config.h"

namespace rift {
	class EntityManager;

    // The Entity class
	// a handle to an Entity::ID
	class Entity final {
	public:

		// The Entity::ID class
		// The concept of an 'entity' is the Entity::ID
		// Notes:
		// - An Entity::ID associates with an EntityRecord which contains the list components owned by the Entity::ID
		// - An Entity::ID determines the lifespan of an Entity handle and its components through its version
		// - Components are arranged as parallel arrays and the Entity::ID serves as an index into those arrays
		class ID final {
		public:
			ID();
			ID(std::size_t index, std::size_t version);
			// Return this Entity::ID with an incremented version number
			ID& renew() noexcept;
			// Return the index portion of the Entity::ID 
			// Note: Used for indexing into arrays
			std::size_t index() const noexcept;
			// Return the version portion of the Entity::ID
			// Note: Used for Entity::ID equality comparisons
			std::size_t version() const noexcept;
		private:
			std::size_t m_index;
			std::size_t m_version;
		};

		// The Entity::Record class
		// An Entity::Record handles the book keeping for an Entity::ID, that is, component management.
		// Notes:
		// - Whenever an Entity handle is created, an associated Entity::Record is created. The newly created
		//   Entity handle is then given a copy of the Entity::ID created in the Entity::Record.
		// - Whenever an Entity handle is destroyed, the associated Entity::Record ensures that the destroyed 
		//   Entity and all copies of it are invalid by renewing its Enity::ID
		class Record final {
			using ComponentFamily = std::size_t;
		public:
			Record();
			Record(rift::Entity::ID id);
			// Renews the record's Entity::ID and returns a copy of it
			Entity::ID renew_master_id() noexcept;
			// Returns a copy of the record's Entity::ID
			Entity::ID master_id_copy() const noexcept;
			// Returns the ComponentMask for the Entity::ID
			ComponentMask components() const noexcept;
			// Sets the bit associated with the component
			void set_component(ComponentFamily family) noexcept { component_list.set(family); }
			// Resets the bit associated with the component
			void reset_component(ComponentFamily family) noexcept { component_list.reset(family); }
			// Tests the bit associated witht the component
			bool test_component(ComponentFamily family) noexcept { return component_list.test(family); }
		private:
			// The master id that owns components given in component_list
			rift::Entity::ID entity_id;
			// The bitmask of components currently assigned to the master entity_id
			rift::ComponentMask component_list;
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

		// Returns a pointer to this entity's EntityManager
		// Note: You must not delete the pointer!
		EntityManager* manager() const noexcept;

	private:
		friend class EntityManager;
		// Only EntityManagers are permitted to create valid entity handles
		Entity(EntityManager *em, Entity::ID id);
	
		// A copy of the master Entity::ID owned by the associated EntityManager::EntityRecord
		Entity::ID m_id;
		
		// The manager responsible for creating this Entity
		EntityManager* mgr;
	};

	bool operator<(const rift::Entity::ID& a, const rift::Entity::ID& b) noexcept;
	bool operator>(const rift::Entity::ID& a, const rift::Entity::ID& b) noexcept;
	bool operator==(const rift::Entity::ID& a, const rift::Entity::ID& b) noexcept;
	bool operator!=(const rift::Entity::ID& a, const rift::Entity::ID& b) noexcept;

	bool operator<(const rift::Entity& a, const rift::Entity& b) noexcept;
	bool operator>(const rift::Entity& a, const rift::Entity& b) noexcept;
	bool operator==(const rift::Entity& a, const rift::Entity& b) noexcept;
	bool operator!=(const rift::Entity& a, const rift::Entity& b) noexcept;

	// The EntityManager class
	// Responsible for the management of Entities and their EntityRecords
	class EntityManager final {
	public:

		EntityManager();
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
		
		// Associates a component of type C with the master Entity::ID of id
		template <class C, class... Args>
		void add(const Entity::ID& id, Args&& ...args) noexcept;
		
		// Disassociates a component of type C with the master Entity::ID of id
		template <class C>
		void remove(const Entity::ID& id) noexcept;

		// Checks if there is an association between the master Entity::ID and a component of type C
		template <class C>
		bool has(const Entity::ID& id) noexcept;

		// Returns the component of type C associate with the master Entity::ID
		template <class C>
		C &get(const Entity::ID& id) noexcept;

		// Checks if the id is of the same version as its master Entity::ID
		bool valid_id(const Entity::ID& id) const noexcept;

		// Invalidate id and all copies of it by refreshing the EntityRecord id maps to
		void invalidate_id(const Entity::ID& id) noexcept;
		
		// Returns the ComponentMask associated with the master Entity::ID of id
		ComponentMask component_mask(const Entity::ID& id) const noexcept;

		template <class C>
		std::shared_ptr<Pool<C>> pool_for() noexcept;

	private:

		// Entities with cache
		// std::unordered_map<ComponentMask, SparseSet> entities_with_cache;

		// The collection of EntityRecords
		std::vector<Entity::Record> entity_records;
		
		// The queue of reusable Entity::IDs
		std::queue<Entity::ID> reusable_ids;

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
		auto mask = util::mask_for<Rest...>();
		mask.set(First::family());
		std::size_t count = 0;
		for (auto entity_record : entity_records) {
			if ((entity_record.components() & mask) == mask) {
				++count;
			}
		}
		return count;
	}

	template <class First, class... Rest>
	inline void EntityManager::entities_with(std::function<void(const Entity&)>&& fun) noexcept
	{
		auto mask = util::mask_for<Rest...>();
		mask.set(First::family());
		for (auto entity_record : entity_records) {
			if ((entity_record.components() & mask) == mask) {
				fun(Entity(this, entity_record.master_id_copy()));
			}
		}
	}

	template<class C, class ...Args>
	inline void EntityManager::add(const Entity::ID& id, Args && ...args) noexcept
	{
		auto pool = pool_for<C>();
		auto size = pool->size();
		if (size <= id.index()) {
			pool->allocate(id.index() - size + 1);
		}
		pool->at(id.index()) = C(std::forward<Args>(args)...);
		entity_records.at(id.index()).set_component(C::family());
	}

	template<class C>
	inline void EntityManager::remove(const Entity::ID& id) noexcept
	{
		entity_records.at(id.index()).reset_component(C::family());
	}

	template<class C>
	inline bool EntityManager::has(const Entity::ID& id) noexcept
	{
		return entity_records.at(id.index()).test_component(C::family());
	}

	template<class C>
	inline C & EntityManager::get(const Entity::ID& id) noexcept
	{
		return (*(std::static_pointer_cast<Pool<C>>(component_pools.at(C::family()))))[id.index()];
	}

	template<class C>
	inline std::shared_ptr<Pool<C>> EntityManager::pool_for() noexcept
	{
		if (component_pools.find(C::family()) == component_pools.end())
			component_pools.emplace(C::family(), std::make_shared<Pool<C>>(entity_records.size()));
		return std::static_pointer_cast<Pool<C>>(component_pools.at(C::family()));
	}

}
