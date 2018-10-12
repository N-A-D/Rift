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
		// An EntityManager determines if an Entity's index is valid through the version of the Entity::ID
		class ID {
		public:
			ID() = default;
			ID(std::uint32_t index, std::uint32_t version)
				: m_number(std::uint64_t(index) | std::uint64_t(version) << 32) {}
			std::uint32_t index() const noexcept { return m_number & 0xFFFFFFFFUL; }
			std::uint32_t version() const noexcept { return m_number >> 32; }
			std::uint64_t number() const noexcept { return m_number; }
		private:
			std::uint64_t m_number;
		};


		// Create an invalid entity
		// An invalid Entity is an entity that is not created from an EntityManager
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

		// Adds a component of type C
		// Note: 
		// - An assertion is made that the entity does not own a component of type C
		template <class C, class ...Args>
		void add(Args&& ...args) const noexcept;

		// Removes a component of type C
		// Note: 
		// - An assertion is made that the entity owns a component of type C
		template <class C>
		void remove() const noexcept;

		// Checks if a component of type C is a member of the entity's list of components
		template <class C>
		bool has() const noexcept;

		// Fetches the component of type C from the entity's list of components
		// Note: 
		// - an assertion is made that the entity owns a component of type C
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

	bool operator<(const Entity::ID& a, const Entity::ID& b) noexcept;
	bool operator>(const Entity::ID& a, const Entity::ID& b) noexcept;
	bool operator==(const Entity::ID& a, const Entity::ID& b) noexcept;
	bool operator!=(const Entity::ID& a, const Entity::ID& b) noexcept;

	// The EntityManager class
	// Manages the lifecycle of Entity handles
	class EntityManager final {
	public:

		EntityManager() = default;
		// EntityManagers may not be copied as Entity handles store pointers 
		// to the EntityManager's that created them
		EntityManager(const EntityManager&) = delete;
		EntityManager& operator=(const EntityManager&) = delete;

		// Generate a new Entity handle
		Entity create_entity() noexcept;

		// Returns the number of Entity::IDs that associate with each of the component types
		template <class First, class... Rest>
		std::size_t count_entities_with() const noexcept;

		// Applies the function fun onto entities that own an instance of each component type
		template <class First, class... Rest>
		void entities_with(std::function<void(const Entity&)>&& fun) noexcept;

#ifndef RIFT_DEBUG
	private:
#endif // !RIFT_DEBUG

		friend class Entity;

		// Set the bit for the component type C in the component mask at id's index
		template <class C, class... Args>
		void add(const Entity::ID& id, Args&& ...args) noexcept;

		// Reset the bit for component type C in the component mask at id's index
		template <class C>
		void remove(const Entity::ID& id) noexcept;

		// Test the bit for component type C in the component mask at id's index
		template <class C>
		bool has(const Entity::ID& id) noexcept;

		// Fetch the component at id's index
		template <class C>
		C &get(const Entity::ID& id) noexcept;

		// Check if id's version number if valid
		bool valid_id(const Entity::ID& id) const noexcept;

		// Invalidate all Entity handles whose Entity::ID is equivalent to id
		void invalidate_id(const Entity::ID& id) noexcept;

		// Removes all components associated with the Entity::ID
		void delete_components_for(const Entity::ID& id) noexcept;

		// Remove an Entity from all caches that are relevant
		void delete_caches_for(const Entity::ID& id) noexcept;
		
		// Returns the ComponentMask associated with the master Entity::ID of id
		ComponentMask component_mask_for(const Entity::ID& id) const noexcept;

		// Returns the component pool for component type C
		// Note: 
		// - If the component type does not exist, it is created then returned.
		template <class C>
		std::shared_ptr<Cache<C>> cache_for() noexcept;

		// The collection of ComponentMasks
		std::vector<ComponentMask> masks;

		// The collection of ID versions
		std::vector<std::size_t> id_versions;
		
		// The queue of reusable Entity::IDs
		std::queue<Entity::ID> reusable_ids;

		// A map from query signature to result set
		std::unordered_map<ComponentMask, Cache<Entity>> entity_caches;

		// The component pools
		using ComponentFamily = std::size_t;
		std::unordered_map<ComponentFamily, std::shared_ptr<BaseCache>> component_pools;
	};

	template<class C, class ...Args>
	inline void Entity::add(Args && ...args) const noexcept
	{
		assert(valid() && "Cannot add components to an invalid entity");
		assert(!has<C>() && "The entity already owns a component of type C");
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
		// Generate the component mask for the given component types
		auto signature = signature_for<Rest...>();
		signature.set(First::family());
		std::size_t count = 0;
		// Count the number of masks that match the given signature
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
		// Generate the component mask for the given component types
		ComponentMask signature;
		signature.set(First::family());
		signature |= signature_for<Rest...>();

		// Check for a cached set of entities whose component masks match the signature above
		if (entity_caches.find(signature) != entity_caches.end()) {
			for (auto& entity : entity_caches.at(signature))
				fun(entity);
		}
		// If there is no cached set of entities, look for the entities that match the signature
		// and store them in a cached set for future use while also applying fun
		else {
			Cache<Entity> entity_cache;
			for (std::size_t i = 0; i < masks.size(); i++) {
				if ((masks[i] & signature) == signature) {
					Entity e(this, Entity::ID(i, id_versions[i]));
					// Store a copy into the cache
					entity_cache.insert(e.id().index(), &e);
					fun(e);
				}
			}
			entity_caches.emplace(signature, entity_cache);
		}
	}

	template<class C, class ...Args>
	inline void EntityManager::add(const Entity::ID& id, Args && ...args) noexcept
	{
		// Insert a copy of a newly constructed component in the component cache for C
		auto component = C(std::forward<Args>(args)...);
		// Store a copy of the component in cache
		cache_for<C>()->insert(id.index(), &component);
		auto mask = masks.at(id.index()).set(C::family());

		// As cached search results may already exist, ensure that this entity is in
		// all relevant entity caches so that it isn't missed in a query
		Entity e(this, id);
		for (auto& pair : entity_caches) {
			// Only insert into result sets whose signature includes the bit for type C
			if (pair.first.test(C::family()) && (mask & pair.first) == pair.first)
				// Store a copy of the entity in the cache
				pair.second.insert(id.index(), &e);
		}
	}

	template<class C>
	inline void EntityManager::remove(const Entity::ID& id) noexcept
	{
		// Remove the component that once belonged to the entity from the component
		// cache for type C
		cache_for<C>()->erase(id.index());
		// Disable the bit for type C in the Entity's component mask
		masks.at(id.index()).reset(C::family());

		// When an entity loses a component of type C, all entity caches whose
		// signature includes C & matches the entity's component signature 
		// should remove that entity
		auto mask = masks.at(id.index());
		for (auto& pair : entity_caches) {
			// If the component bit for component type C is set and the result
			// set has the entity then remove the entity
			if (pair.first.test(C::family()) && (mask & pair.first) == pair.first)
				pair.second.erase(id.index());
		}
	}

	template<class C>
	inline bool EntityManager::has(const Entity::ID& id) noexcept
	{
		return masks.at(id.index()).test(C::family());
	}

	template<class C>
	inline C & EntityManager::get(const Entity::ID& id) noexcept
	{
		return *(static_cast<C *>(cache_for<C>()->get(id.index())));
	}

	template<class C>
	inline std::shared_ptr<Cache<C>> EntityManager::cache_for() noexcept
	{
		if (component_pools.find(C::family()) == component_pools.end())
			component_pools.emplace(C::family(), std::make_shared<Cache<C>>());
		return std::static_pointer_cast<Cache<C>>(component_pools.at(C::family()));
	}

}
