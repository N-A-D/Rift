#pragma once

#include <queue>
#include <memory>
#include <vector>
#include <assert.h>
#include <functional>
#include "util/pool.h"
#include "component.h"
#include "details/config.h"

namespace rift {

	class EntityManager;

	// An Entity is a handle to an Entity::ID
	class Entity {
	public:

		// The core of an Entity is the Entity::ID
		// The Entity::ID maps to an EntityManager::BitMask that indicates which components are owned by the Entity
		// The Entity::ID determines the lifespan of an Entity and its components through its version
		// The different component types are arranged in contiguous pools in parallel fashion. The Entity::ID is used
		// to index into these pools to find the components that belong to the Entity.
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
		
		// Primes the entity's Entity::ID for invalidation along with any other Entity that happens to have the same Entity::ID as this Entity
		// Note: This entity, and any other identical entity, is still valid to use in the current frame but will not be in the next
		void invalidate() const noexcept;

		// Checks if this entity's Entity::ID is pending invalidation
		bool is_pending_invalidation() const noexcept;

		// Fetch the entity's ComponentMask
		rift::ComponentMask component_mask() const noexcept;

		// Adds a component of type C to the entity's list of components
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

		friend bool operator==(const rift::Entity& a, const rift::Entity& b) noexcept;
		friend bool operator!=(const rift::Entity& a, const rift::Entity& b) noexcept;

	private:
		friend class EntityManager;
		// Only EntityManagers are permitted to create valid entity handles
		Entity(EntityManager *em, Entity::ID id);
	
		// A copy of the master Entity::ID managed by an EntityManager::BitMask
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

	class EntityManager {
	public:

		// Maps assigned components to a master Entity::ID
		class BitMask {
		public:
			BitMask();
			BitMask(rift::Entity::ID id);
			// Returns a new version of the master id
			Entity::ID refresh() noexcept;
		private:
			friend class EntityManager;
			// An indicator as to whether or not the id is to be refreshed
			bool pending_refresh;
			// The master id that owns components defined in component_list
			rift::Entity::ID id;
			// The bitmask of components currently assigned to the master entity_id
			rift::ComponentMask component_list;
		};

		// Generate a new entity
		Entity create_entity() noexcept;

		// Invalidates all Entity::IDs whose master id has been refreshed
		void update() noexcept;

		// Returns the number of Entity::IDs that associate with an instance of each component type
		// Note: When Component type parameters are not supplied, the returned value indicates
		// the number of Entity::IDs that do not map to any component types
		template <class ...Components>
		std::size_t count_entities_with() const noexcept;

		// The function is applies 'fun' onto Entity(s) whose Entity::ID associates with an instance of each component type
		// Note: When the Component type parameters are not supplied, the function 'fun' is applied onto Entity(s) whose
		// Entity::ID does not associate with an instance of any of the component types
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

		// Mark the id's master as pending for refresh
		void mark_for_refresh(Entity::ID id) noexcept;

		// Checks if id's master is pending refresh
		bool is_pending_refresh(Entity::ID id) noexcept;

		// Allocates a new BitMask and return a copy of the master Entity::ID contained in it
		Entity::ID accommodate_entity() noexcept;
		
		// Returns the ComponentMask associated with the master Entity::ID of id
		ComponentMask component_mask(Entity::ID id) const noexcept;
	
		// Checks if this EntityManager has a component pool for C
		template <class C>
		bool has_pool_for() const noexcept;

		// Creates a component pool for type C where size indicates how many components to allocate
		template <class C>
		void create_pool_for(std::size_t size) noexcept;

	private:

		// The collection of BitMasks
		std::vector<BitMask> bitmasks;
		
		// The queue of reusable Entity::IDs
		std::queue<Entity::ID> reusable_ids;

		// The pools of components
		std::vector<std::shared_ptr<BasePool>> component_pools;
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
			for (auto bitmask : bitmasks) {
				if (bitmask.component_list == 0) {
					++count;
				}
			}
		}
		else {
			for (auto bitmask : bitmasks) {
				if (bitmask.component_list != 0 && (bitmask.component_list & mask) == mask) {
					++count;
				}
			}
		}
	}

	template<class ...Components>
	inline void EntityManager::entities_with(std::function<void(const Entity&)>&& fun) noexcept
	{
		auto mask = util::mask_for<Components...>();
		if (mask == 0) {
			for (auto bitmask : bitmasks) {
				if (bitmask.component_list == 0) {
					fun(Entity(this, bitmask.id));
				}
			}
		}
		else {
			for (auto bitmask : bitmasks) {
				if (bitmask.component_list != 0 && (bitmask.component_list & mask) == mask) {
					fun(Entity(this, bitmask.id));
				}
			}
		}
	}

	template<class C, class ...Args>
	inline void EntityManager::add(Entity::ID id, Args && ...args) noexcept
	{
		if (!has_pool_for<C>()) { create_pool_for<C>(bitmasks.size()); }
		bitmasks[id.index()].component_list.set(C::family());
		auto pool_ptr = std::static_pointer_cast<Pool<C>>(component_pools.at(C::family()));
		pool_ptr->at(id.index()) = C(std::forward<Args>(args)...);
	}

	template<class C>
	inline void EntityManager::remove(Entity::ID id) noexcept
	{
		bitmasks[id.index()].component_list.reset(C::family());
	}

	template<class C>
	inline bool EntityManager::has(Entity::ID id) noexcept
	{
		return bitmasks[id.index()].component_list.test(C::family());
	}

	template<class C>
	inline C & EntityManager::get(Entity::ID id) noexcept
	{
		return std::static_pointer_cast<Pool<C>>(component_pools.at(C::family()))->at(id.index());
	}

	template<class C>
	inline bool EntityManager::has_pool_for() const noexcept
	{
		return C::family() < component_pools.size();
	}

	template<class C>
	inline void EntityManager::create_pool_for(std::size_t size) noexcept
	{
		component_pools.resize(C::family() + 1);
		component_pools.at(C::family()) = std::make_shared<Pool<C>>(size);
	}
}