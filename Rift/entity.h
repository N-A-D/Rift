#pragma once
#ifndef _RIFT_ENTITY_
#define _RIFT_ENTITY_
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
	// a convenience class for an index
	class Entity final {
	public:

		// An Entity::ID is a versionable index
		// The index held by the Entity::ID is invalid if its version is invalid
		class ID {
		public:
			ID() : m_number(0) {}
			ID(const ID&) = default;
			ID(std::uint32_t index, std::uint32_t version)
				: m_number(std::uint64_t(index) | std::uint64_t(version) << 32) {}

			ID& operator=(const ID&) = default;

			std::uint32_t index() const noexcept { return m_number & 0xFFFFFFFFUL; }
			std::uint32_t version() const noexcept { return m_number >> 32; }
			std::uint64_t number() const noexcept { return m_number; }

			bool operator<(const ID& other) const noexcept { return m_number < other.m_number; }
			bool operator>(const ID& other) const noexcept { return other < *this; }
			bool operator==(const ID& other) const noexcept { return !(*this < other) && !(other < *this); }
			bool operator!=(const ID& other) const noexcept { return !(*this == other); }

		private:
			std::uint64_t m_number;
		};

		Entity();
		Entity(const Entity&) = default;
		Entity& operator=(const Entity&) = default;

		// Fetch the entity's index
		Entity::ID id() const noexcept;

		// Checks if the entity's index is valid
		bool valid() const noexcept;

		operator bool() const noexcept;

		// Immediately destroy the entity and all others that have the same Entity::ID
		void destroy() noexcept;

		// Invalidate this handle, disassociating it from the manager that created it
		void invalidate() noexcept;

		// Fetch the entity's ComponentMask
		rift::ComponentMask component_mask() const noexcept;

		// Adds a component to the entity
		// Note: 
		// - An assertion is made that the entity does not own a component of type C
		template <class C, class ...Args>
		void add(Args&& ...args) const noexcept;

		// Removes a component from the entity
		// Note: 
		// - An assertion is made that the entity owns a component of type C
		template <class C>
		void remove() const noexcept;

		// Checks the entity has a component
		template <class C>
		bool has() const noexcept;

		// Fetch the component for the entity
		// Note: 
		// - an assertion is made that the entity owns a component of type C
		template <class C>
		C &get() const noexcept;

		bool operator<(const Entity& other) const noexcept { return m_id < other.m_id; }
		bool operator>(const Entity& other) const noexcept { return other < *this; }
		bool operator==(const Entity& other) const noexcept { return mgr == other.mgr && !(*this < other) && !(other < *this); }
		bool operator!=(const Entity& other) const noexcept { return !(*this == other); }

	private:
		friend class EntityManager;

		static const ID INVALID_ID;

		// Only EntityManagers are permitted to create valid entity handles
		Entity(EntityManager *em, Entity::ID id);

		// The manager that created this entity
		EntityManager* mgr;

		// The entity's index. Only valid while the version is valid
		Entity::ID m_id;

	};

	// The EntityManager class
	// Manages the lifecycle of Entity handles
	class EntityManager final {
		friend class Entity;
	public:

		EntityManager() = default;
		// EntityManagers may not be copied as Entity handles store pointers 
		// to the EntityManager's that created them
		EntityManager(const EntityManager&) = delete;
		EntityManager& operator=(const EntityManager&) = delete;

		// Generate a new Entity handle
		Entity create_entity() noexcept;

		// Returns the number of managed entities
		std::size_t size() const noexcept;

		// Returns the capacity 
		std::size_t capacity() const noexcept;

		// Returns the number of entities that associate with each of the component types
		template <class First, class... Rest>
		std::size_t count_entities_with() const noexcept;

		// Applies the function f onto entities that own an instance of each component type
		// example:
		// EntityManager em;
		// ...
		// em.entities_with<Position, Direction>(...);
		template <class First, class... Rest>
		void entities_with(std::function<void(Entity)> f);

	private:

		/// Internal functions that an Entity interfaces with
		/// In all cases, the entity is verified to be valid
		/// before invoking any of the below functions

		// Enable the component type C in the entity's component mask
		template <class C, class... Args>
		void add_component(const Entity::ID& id, Args&& ...args) noexcept;

		// Disable the component type C in the entity's component mask
		template <class C>
		void remove_component(const Entity::ID& id) noexcept;

		// Test the component type C is enabled in the entity's component mask
		template <class C>
		bool has_component(const Entity::ID& id) noexcept;

		// Fetch the component for the entity
		template <class C>
		C &get_component(const Entity::ID& id) noexcept;

		// Returns the component pool for component type C
		template <class C>
		std::shared_ptr<Cache<C>> component_cache_for(std::size_t id) noexcept;

		// Fetch the ComponentMask for the entity
		ComponentMask component_mask_for(const Entity::ID& id) const noexcept;

		// Check the validity of the entity
		bool valid_id(const Entity::ID& id) const noexcept;

		// Delete all components associated with the entity
		void delete_components_for(const Entity::ID& id) noexcept;

		// Delete the entity from any search caches it may be in
		void delete_any_caches_for(const Entity::ID& id) noexcept;

		// Cleanup resources bound to the entity
		void destroy(const Entity::ID& id) noexcept;

	private:

		// Collection of free indexes
		std::queue<std::uint32_t> free_indexes;

		// Collection of component masks
		std::vector<ComponentMask> masks;

		// Collection of index versions
		std::vector<std::uint32_t> index_versions;

		// The component caches
		std::vector<std::shared_ptr<BaseCache>> component_caches;

		// Entity search caches
		std::unordered_map<ComponentMask, Cache<Entity>> entity_caches;
	};


	template<class C, class ...Args>
	inline void Entity::add(Args && ...args) const noexcept
	{
		assert(valid() && "Cannot add a component to an invalid entity!");
		assert(!has<C>() && "Adding multiple components of the same type to the same entity is not allowed!");
		mgr->add_component<C>(m_id, std::forward<Args>(args)...);
	}

	template<class C>
	inline void Entity::remove() const noexcept
	{
		assert(valid() && "Cannot remove a component from an invalid entity!");
		assert(has<C>() && "The entity does not own a component of the given type!");
		mgr->remove_component<C>(m_id);
	}

	template<class C>
	inline bool Entity::has() const noexcept
	{
		assert(valid() && "Cannot check if an invalid entity has a component!");
		return mgr->has_component<C>(m_id);
	}

	template<class C>
	inline C & Entity::get() const noexcept
	{
		assert(valid() && "Cannot get a compnent for an invalid entity!");
		assert(has<C>() && "The entity does not have a component of the given type!");
		return mgr->get_component<C>(m_id);
	}

	template<class First, class ...Rest>
	inline std::size_t EntityManager::count_entities_with() const noexcept
	{
		auto signature = signature_for<First, Rest...>();

		std::size_t count = 0;
		for (auto& mask : masks) {
			if ((mask & signature) == signature) {
				++count;
			}
		}
		return count;
	}

	template<class First, class ...Rest>
	inline void EntityManager::entities_with(std::function<void(Entity)> f)
	{
		auto signature = signature_for<First, Rest...>();

		if (entity_caches.find(signature) != entity_caches.end()) {
			for (auto entity : entity_caches.at(signature)) {
				f(entity);
			}
		}
		else {
			Cache<Entity> search_cache;
			for (std::size_t i = 0; i < masks.size(); i++) {
				if ((masks[i] & signature) == signature) {
					Entity e(this, Entity::ID(static_cast<std::uint32_t>(i), index_versions[i]));
					search_cache.insert(i, &e);
					f(e);
				}
			}
			entity_caches.emplace(signature, search_cache);
		}
	}

	template<class C, class ...Args>
	inline void EntityManager::add_component(const Entity::ID & id, Args && ...args) noexcept
	{
		auto family_id = C::family();
		auto index = id.index();
		auto mask = masks[index].set(family_id);
		auto component = C(std::forward<Args>(args)...);
		component_cache_for<C>(family_id)->insert(index, &component);

		Entity e(this, id);
		for (auto& search_cache : entity_caches) {
			if (search_cache.first.test(family_id) &&
				(mask & search_cache.first) == search_cache.first) {
				search_cache.second.insert(index, &e);
			}
		}
	}

	template<class C>
	inline void EntityManager::remove_component(const Entity::ID & id) noexcept
	{
		auto family_id = C::family();
		auto index = id.index();
		auto mask = masks[index];

		for (auto& search_cache : entity_caches) {
			if (search_cache.first.test(family_id) &&
				(mask & search_cache.first) == search_cache.first) {
				search_cache.second.erase(index);
			}
		}

		component_cache_for<C>(family_id)->erase(index);
		masks[index].reset(family_id);
	}

	template<class C>
	inline bool EntityManager::has_component(const Entity::ID & id) noexcept
	{
		return masks[id.index()].test(C::family());
	}

	template<class C>
	inline C & EntityManager::get_component(const Entity::ID & id) noexcept
	{
		return *(static_cast<C *>(component_cache_for<C>(C::family())->get(id.index())));
	}

	template<class C>
	inline std::shared_ptr<Cache<C>> EntityManager::component_cache_for(std::size_t id) noexcept
	{
		if (id >= component_caches.size())
			component_caches.resize(id + 1);
		if (!component_caches[id])
			component_caches[id] = std::make_shared<Cache<C>>();
		return std::static_pointer_cast<Cache<C>>(component_caches[id]);
	}

}

#endif // !_RIFT_ENTITY_