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

	// The EntityID class
	// The concept of an 'entity' is the EntityID
	// Notes:
	// - An EntityID associates with an EntityRecord which contains the list components owned by the EntityID
	// - An EntityID determines the lifespan of an Entity handle and its components through its version
	// - Components are arranged as parallel arrays and the EntityID serves as an index into those arrays
	class EntityID final {
	public:
		EntityID();
		EntityID(std::size_t index, std::size_t version);
		// Return this EntityID with an incremented version number
		EntityID& renew() noexcept;
		// Return the index portion of the EntityID 
		// Note: Used for indexing into arrays
		std::size_t index() const noexcept;
		// Return the version portion of the EntityID
		// Note: Used for EntityID equality comparisons
		std::size_t version() const noexcept;
	private:
		std::size_t m_index;
		std::size_t m_version;
	};

	bool operator<(const rift::EntityID& a, const rift::EntityID& b) noexcept;
	bool operator>(const rift::EntityID& a, const rift::EntityID& b) noexcept;
	bool operator==(const rift::EntityID& a, const rift::EntityID& b) noexcept;
	bool operator!=(const rift::EntityID& a, const rift::EntityID& b) noexcept;

	class EntityManager;

    // The Entity class
	// a handle to an EntityID
	class Entity final {
	public:

		// Create an invalid entity
		Entity();

		// Fetch the entity's EntityID
		EntityID id() const noexcept;

		// Checks if the entity's EntityID is valid
		bool valid() const noexcept;
		operator bool() const noexcept;
		
		// Invalidate this entity and all other entities that share the same EntityID
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
		Entity(EntityManager *em, EntityID id);
	
		// A copy of the master EntityID owned by the associated EntityManager::EntityRecord
		EntityID m_id;
		
		// The manager responsible for creating this Entity
		EntityManager* mgr;
	};

	bool operator<(const rift::Entity& a, const rift::Entity& b) noexcept;
	bool operator>(const rift::Entity& a, const rift::Entity& b) noexcept;
	bool operator==(const rift::Entity& a, const rift::Entity& b) noexcept;
	bool operator!=(const rift::Entity& a, const rift::Entity& b) noexcept;

	// The EntityRecord class
	// An EntityRecord handles the book keeping for an EntityID (Component management and Entity handle lifespans)
	// Whenever an Entity handle is destroyed, the associated EntityRecord ensures that all copies of 
	// that Entity are also invalid by renewing its EntityID value
	class EntityRecord final {
		using ComponentFamily = std::size_t;
	public:
		EntityRecord();
		EntityRecord(rift::EntityID id);
		// Renews the record's EntityID and returns a copy of it
		EntityID renew_master_id() noexcept;
		// Returns a copy of the record's EntityID
		EntityID master_id_copy() const noexcept;
		// Returns the ComponentMask for the EntityID
		ComponentMask components() const noexcept;
		// Sets the bit associated with the component
		void set_component(ComponentFamily family) noexcept { component_list.set(family); }
		// Resets the bit associated with the component
		void reset_component(ComponentFamily family) noexcept { component_list.reset(family); }
		// Tests the bit associated witht the component
		bool test_component(ComponentFamily family) noexcept { return component_list.test(family); }
	private:
		// The master id that owns components given in component_list
		rift::EntityID entity_id;
		// The bitmask of components currently assigned to the master entity_id
		rift::ComponentMask component_list;
	};

	// The EntityManager class
	// Responsible for the management of Entities and their EntityRecords
	class EntityManager final {
	public:

		EntityManager();
		EntityManager(const EntityManager&) = delete;
		EntityManager& operator=(const EntityManager&) = delete;

		// Generate a new EntityManager::EntityRecord and return an Entity as a handle to its EntityID
		Entity create_entity() noexcept;

		// Returns the number of EntityIDs that associate with an instance of each component type
		// Note: When Component type parameters are not supplied, the returned value indicates
		// the number of EntityIDs that do not map to any component types
		template <class ...Components>
		std::size_t count_entities_with() const noexcept;

		// The function applies 'fun' onto Entity(s) whose EntityID associates with an instance of each component type
		// given as a template parameter.
		// Note: When Component type parameters are not supplied, the function 'fun' is applied onto Entity(s) whose
		// EntityID does not associate with an instance of any existing component types
		template <class ...Components>
		void entities_with(std::function<void(const Entity&)>&& fun) noexcept;

	private:

		friend class Entity;
		
		// Associates a component of type C with the master EntityID of id
		template <class C, class... Args>
		void add(EntityID id, Args&& ...args) noexcept;
		
		// Disassociates a component of type C with the master EntityID of id
		template <class C>
		void remove(EntityID id) noexcept;

		// Checks if there is an association between the master EntityID and a component of type C
		template <class C>
		bool has(EntityID id) noexcept;

		// Returns the component of type C associate with the master EntityID
		template <class C>
		C &get(EntityID id) noexcept;

		// Checks if the id is of the same version as its master EntityID
		bool valid_id(EntityID id) const noexcept;

		// Invalidate id and all copies of it by refreshing the EntityRecord id maps to
		void invalidate_id(EntityID id) noexcept;
		
		// Returns the ComponentMask associated with the master EntityID of id
		ComponentMask component_mask(EntityID id) const noexcept;
	
		// Checks if this EntityManager has a component pool for C
		template <class C>
		bool has_pool_for() const noexcept;

		// Creates a component pool for type C where size indicates how many components to allocate
		template <class C>
		void create_pool_for(std::size_t size) noexcept;

	private:

		// The collection of EntityRecords
		std::vector<EntityRecord> entity_records;
		
		// The queue of reusable EntityIDs
		std::queue<EntityID> reusable_ids;

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
				if (entity_record.components() == 0) {
					++count;
				}
			}
		}
		else {
			for (auto entity_record : entity_records) {
				if ((entity_record.components() & mask) == mask) {
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
				if (entity_record.components() == 0) {
					fun(Entity(this, entity_record.master_id_copy()));
				}
			}
		}
		else {
			for (auto entity_record : entity_records) {
				if ((entity_record.components() & mask) == mask) {
					fun(Entity(this, entity_record.master_id_copy()));
				}
			}
		}
	}

	template<class C, class ...Args>
	inline void EntityManager::add(EntityID id, Args && ...args) noexcept
	{
		if (!has_pool_for<C>()) 
			create_pool_for<C>(entity_records.size());
		auto pool = std::static_pointer_cast<Pool<C>>(component_pools.at(C::family()));
		auto size = pool->size() - 1;
		if (size < id.index()) {
			pool->allocate(id.index() - size + 1);
		}
		pool->at(id.index()) = C(std::forward<Args>(args)...);
		entity_records.at(id.index()).set_component(C::family());
	}

	template<class C>
	inline void EntityManager::remove(EntityID id) noexcept
	{
		entity_records.at(id.index()).reset_component(C::family());
	}

	template<class C>
	inline bool EntityManager::has(EntityID id) noexcept
	{
		return entity_records.at(id.index()).test_component(C::family());
	}

	template<class C>
	inline C & EntityManager::get(EntityID id) noexcept
	{
		return (*(std::static_pointer_cast<Pool<C>>(component_pools.at(C::family()))))[id.index()];
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
