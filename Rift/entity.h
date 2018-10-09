#pragma once

#include <queue>
#include <memory>
#include <assert.h>
#include <functional>
#include <unordered_map>
#include "details/functions.h"
#include "details/containers.h"

namespace rift {
	class EntityManager;

    // The Entity class
	// a handle for an Entity::ID
	class Entity final {
	public:

		// The Entity::ID class
		// The concept of an 'entity' is the Entity::ID
		// Notes:
		// - An Entity::ID associates with an Entity::Record which contains the list components owned by the Entity::ID
		// - An Entity::ID determines the lifespan of Entity handles and the current component list through its version
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
		// An Entity::Record records which components belong and do not belong to an Entity::ID
		// Notes:
		// - Whenever an Entity handle is created, an associated Entity::Record is created. The newly created
		//   Entity handle is then given a copy of the Entity::ID created in the Entity::Record.
		// - Whenever an Entity handle is destroyed, the associated Entity::Record ensures that the destroyed 
		//   Entity and all copies of it are invalid by renewing its Entity::ID
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
			// Insert the bit associated with the component
			void insert_component(ComponentFamily family) noexcept;
			// Remove the bit associated with the component
			void remove_component(ComponentFamily family) noexcept;
			// Check if the bit associated with the component is active
			bool check_component(ComponentFamily family) noexcept;
		private:
			// The master Entity::ID that owns the component_list
			rift::Entity::ID id;
			// The bitmask of components currently assigned to the master Entity::ID
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
	
		// A copy of the master Entity::ID owned by the associated Entity::Record
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
	// Manages Entity::Records and the creation of Entity handles
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

		class Cache {
		public:
			using iterator = std::vector<Entity>::iterator;
			using const_iterator = std::vector<Entity>::const_iterator;
				
			Cache();

			iterator begin();
			iterator end();
			const_iterator begin() const;
			const_iterator end() const;

			// Return true if the entity is found, false otherwise
			bool search(const Entity& e);
			// Insert the entity into the set unless it already exists within it
			void insert(const Entity& e);
			// Remove the entity from the set unless it does not exist within it
			void remove(const Entity& e);
			// Clear the set
			void clear();
			// Check if the set is empty
			bool empty() const;

		private:
			// The current number of entities
			std::size_t n;
			// The container full of entities that match a search criteria (bitmask)
			std::vector<Entity> dense;
			// The container of indexes to entities
			std::vector<std::size_t> sparse;
		};

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
		
		// Returns the ComponentMask associated with the master Entity::ID of id
		ComponentMask component_mask(const Entity::ID& id) const noexcept;

		// Returns the component pool for component type C
		// Note: 
		// - If the component type does not exist, it is created then returned.
		template <class C>
		std::shared_ptr<Pool<C>> pool_for() noexcept;

	private:

		// Query caches
		std::unordered_map<ComponentMask, Cache> search_caches;
	
		// The collection of Entity::Records
		std::vector<Entity::Record> entity_records;
		
		// The queue of reusable Entity::IDs
		std::queue<Entity::ID> reusable_ids;

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
		auto mask = signature_for<Rest...>();
		mask.set(First::family());
		std::size_t count = 0;
		for (auto& entity_record : entity_records) {
			if ((entity_record.components() & mask) == mask) {
				++count;
			}
		}
		return count;
	}

	template <class First, class... Rest>
	inline void EntityManager::entities_with(std::function<void(const Entity&)>&& fun) noexcept
	{
		auto mask = signature_for<Rest...>();
		mask.set(First::family());
		
		if (search_caches.find(mask) != search_caches.end()) {
			for (auto& entity : search_caches.at(mask)) {
				fun(entity);
			}
		}
		else {
			search_caches.emplace(mask, Cache());
			for (auto& entity_record : entity_records) {
				if ((entity_record.components() & mask) == mask) {
					auto e = Entity(this, entity_record.master_id_copy());
					fun(e);
					search_caches.at(mask).insert(e);
				}
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
		entity_records.at(id.index()).insert_component(C::family());
		
		auto mask = component_mask(id);
		for (auto& pair : search_caches) {
			if ((mask & pair.first) == pair.first) {
				pair.second.insert(Entity(this, id));
			}
		}	
	}

	template<class C>
	inline void EntityManager::remove(const Entity::ID& id) noexcept
	{
		for (auto& pair : search_caches) {
			if (pair.first.test(C::family())) {
				pair.second.remove(Entity(this, id));
			}
		}
		entity_records.at(id.index()).remove_component(C::family());
	}

	template<class C>
	inline bool EntityManager::has(const Entity::ID& id) noexcept
	{
		return entity_records.at(id.index()).check_component(C::family());
	}

	template<class C>
	inline C & EntityManager::get(const Entity::ID& id) noexcept
	{
		return std::static_pointer_cast<Pool<C>>(component_pools.at(C::family()))->at(id.index());
	}

	template<class C>
	inline std::shared_ptr<Pool<C>> EntityManager::pool_for() noexcept
	{
		if (component_pools.find(C::family()) == component_pools.end())
			component_pools.emplace(C::family(), std::make_shared<Pool<C>>(entity_records.size()));
		return std::static_pointer_cast<Pool<C>>(component_pools.at(C::family()));
	}

}
