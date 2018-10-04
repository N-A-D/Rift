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
	class Entity {
	public:

		// The concept of an 'entity' is the Entity::ID
		// Notes:
		// - An Entity::ID associates with an EntityManager::EntityRecord which contains the list components owned by the Entity::ID
		// - An Entity::ID determines the lifespan of an Entity handle and its components through its version
		// - Components are arranged as parallel arrays and the Entity::ID serves as an index into those arrays
		class ID {
		public:
			ID();
			ID(std::size_t index, std::size_t version);
			ID& renew() noexcept;
			std::size_t index() const noexcept;
			std::size_t version() const noexcept;
			
		private:
			std::size_t m_index;
			std::size_t m_version;
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
		// Note: The pointer returned must not be destroyed
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
	// Responsible for the management of Entity::IDs
	class EntityManager {
	public:

		EntityManager();
		EntityManager(const EntityManager&) = delete;
		EntityManager& operator=(const EntityManager&) = delete;

		// Handles the book keeping for an Entity::ID (Component management and Entity handle lifespans)
		// Whenever an Entity handle is destroyed, the associated EntityRecord ensures that all copies of 
		// that Entity are also now invalid by refreshing its master Entity::ID value
		class EntityRecord {
		public:
			EntityRecord();
			EntityRecord(rift::Entity::ID id);
			// Returns a new version of the master id
			Entity::ID refresh_id() noexcept;
		private:
			friend class EntityManager;
			// The master id that owns components given in component_list
			rift::Entity::ID entity_id;
			// The bitmask of components currently assigned to the master entity_id
			rift::ComponentMask component_list;
		};

		// Generate a new EntityManager::EntityRecord and return an Entity as a handle to its Entity::ID
		Entity create_entity() noexcept;

		// Returns the number of Entity::IDs that associate with an instance of each component type
		// Note: When Component type parameters are not supplied, the returned value indicates
		// the number of Entity::IDs that do not map to any component types
		template <class ...Components>
		std::size_t count_entities_with() const noexcept;

		// The function applies 'fun' onto Entity(s) whose Entity::ID associates with an instance of each component type
		// given as a template parameter.
		// Note: When Component type parameters are not supplied, the function 'fun' is applied onto Entity(s) whose
		// Entity::ID does not associate with an instance of any existing component types
		template <class ...Components>
		void entities_with(std::function<void(const Entity&)>&& fun) noexcept;

	private:

		friend class Entity;
		
		// Associates a component of type C with the master Entity::ID of id
		template <class C, class... Args>
		void add(Entity::ID id, Args&& ...args) noexcept;
		
		// Disassociates a component of type C with the master Entity::ID of id
		template <class C>
		void remove(Entity::ID id) noexcept;

		// Checks if there is an association between the master Entity::ID and a component of type C
		template <class C>
		bool has(Entity::ID id) noexcept;

		// Returns the component of type C associate with the master Entity::ID
		template <class C>
		C &get(Entity::ID id) noexcept;

		// Checks if the id is of the same version as its master Entity::ID
		bool valid_id(Entity::ID id) const noexcept;

		// Invalidate id and all copies of it by refreshing the EntityRecord id maps to
		void invalidate_id(Entity::ID id) noexcept;

		// Returns the Entity::ID for a newly created EntityManager::EntityRecord object
		Entity::ID allocate_entity_record() noexcept;
		
		// Returns the ComponentMask associated with the master Entity::ID of id
		ComponentMask component_mask(Entity::ID id) const noexcept;
	
		// Checks if this EntityManager has a component pool for C
		template <class C>
		bool has_pool_for() const noexcept;

		// Creates a component pool for type C where size indicates how many components to allocate
		template <class C>
		void create_pool_for(std::size_t size) noexcept;

	private:

		// The collection of EntityRecords
		std::vector<EntityRecord> entity_records;
		
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

	template<class ...Components>
	inline std::size_t EntityManager::count_entities_with() const noexcept
	{
		auto mask = util::mask_for<Components...>();
		std::size_t count = 0;
		if (mask == 0) {
			for (auto entity_record : entity_records) {
				if (entity_record.component_list == 0) {
					++count;
				}
			}
		}
		else {
			for (auto entity_record : entity_records) {
				if (entity_record.component_list != 0 && (entity_record.component_list & mask) == mask) {
					++count;
				}
			}
		}
		return count;
	}

	template<class ...Components>
	inline void EntityManager::entities_with(std::function<void(const Entity&)>&& fun) noexcept
	{
		auto mask = util::mask_for<Components...>();
		if (mask == 0) {
			for (auto entity_record : entity_records) {
				if (entity_record.component_list == 0) {
					fun(Entity(this, entity_record.entity_id));
				}
			}
		}
		else {
			for (auto entity_record : entity_records) {
				if (entity_record.component_list != 0 && (entity_record.component_list & mask) == mask) {
					fun(Entity(this, entity_record.entity_id));
				}
			}
		}
	}

	template<class C, class ...Args>
	inline void EntityManager::add(Entity::ID id, Args && ...args) noexcept
	{
		if (!has_pool_for<C>()) { create_pool_for<C>(entity_records.size()); }
		entity_records.at(id.index()).component_list.set(C::family());
		std::static_pointer_cast<Pool<C>>(component_pools.at(C::family()))->at(id.index()) = C(std::forward<Args>(args)...);
	}

	template<class C>
	inline void EntityManager::remove(Entity::ID id) noexcept
	{
		entity_records.at(id.index()).component_list.reset(C::family());
	}

	template<class C>
	inline bool EntityManager::has(Entity::ID id) noexcept
	{
		return entity_records.at(id.index()).component_list.test(C::family());
	}

	template<class C>
	inline C & EntityManager::get(Entity::ID id) noexcept
	{
		return std::static_pointer_cast<Pool<C>>(component_pools.at(C::family()))->at(id.index());
	}

	template<class C>
	inline bool EntityManager::has_pool_for() const noexcept
	{
		return component_pools.find(C::family()) != component_pools.end();
	}

	template<class C>
	inline void EntityManager::create_pool_for(std::size_t size) noexcept
	{
		component_pools.emplace(C::family(), std::make_shared<Pool<C>>(size));
	}

}
