#pragma once

#include <queue>
#include <memory>
#include <numeric>
#include <cassert>
#include <functional>
#include <unordered_map>
#include "internal/cache.h"
#include "internal/sparse_set.h"
#include "internal/rift_traits.h"
#include "internal/noncopyable.h"

namespace rift {

	class EntityManager;

	// The Entity class
	// a convenience class for an index
	class Entity final {
	public:

		// An Entity::ID is a versionable index
		// The index is valid as long as the version is valid
		class ID {
		public:
			ID() = default;
			ID(const ID&) = default;
			ID(std::uint32_t index, std::uint32_t version) noexcept
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
			std::uint64_t m_number = 0;
		};

		static const ID INVALID_ID;

		Entity() = default;
		Entity(const Entity&) = default;
		Entity& operator=(const Entity&) = default;

		// Fetch the entity's index
		Entity::ID id() const noexcept;

		// Checks if the entity's index is valid
		bool valid() const noexcept;

		operator bool() const noexcept;

		// Checks if this entity will be invalid next frame
		bool pending_invalidation() const noexcept;

		// Signal the entity's manager to recycle this entity's index
		void destroy() const noexcept;

		// Fetch the entity's ComponentMask
		rift::ComponentMask component_mask() const noexcept;

		// Adds a component to the entity
		// Note: 
		// - Asserts the entity does not own a component of type C
		template <class C, class ...Args>
		void add(Args&& ...args) const noexcept;

		// Replaces the entity's component
		// Note:
		// - Asserts the entity owns a component of type C
		template <class C, class ...Args>
		void replace(Args&& ...args) const noexcept;

		// Removes a component from the entity
		// Note: 
		// - Asserts the entity owns a component of type C
		template <class C>
		void remove() const noexcept;

		// Checks if the entity has a component
		template <class C>
		bool has() const noexcept;

		// Fetch the component for the entity
		// Note: 
		// - Asserts the entity owns a component of type C
		template <class C>
		C &get() const noexcept;

		bool operator<(const Entity& other) const noexcept { return uid < other.uid; }
		bool operator>(const Entity& other) const noexcept { return other < *this; }
		bool operator==(const Entity& other) const noexcept { return manager == other.manager && !(*this < other) && !(other < *this); }
		bool operator!=(const Entity& other) const noexcept { return !(*this == other); }

	private:
		friend class EntityManager;

		// Only EntityManagers are permitted to create valid entity handles
		Entity(EntityManager *manager, Entity::ID uid) noexcept;

		// The manager that created this entity
		EntityManager* manager = nullptr;

		// The entity's unique id
		Entity::ID uid = INVALID_ID;

	};

	// The EntityManager class
	// Manages the lifecycle of entities and their components
	class EntityManager final : rift::impl::NonCopyable {
		friend class Entity;
	public:

		EntityManager() = default;

		// Generate a new Entity handle
		Entity create_entity() noexcept;

		// Returns the number of managed entities
		std::size_t size() const noexcept;

		// Returns the capacity (number of managed entities + those that can reused)
		std::size_t capacity() const noexcept;

		// Returns the number of entities that can be reused
		std::size_t number_of_reusable_entities() const noexcept;

		// Returns the number of entities waiting to be destroyed
		std::size_t number_of_entities_to_destroy() const noexcept;

		// Returns the number of entities whose component mask includes each component type
		template <class First, class... Rest>
		std::size_t number_of_entities_with() const noexcept;

		// Applies the function f on entities whose component mask includes each component type
		// example:
		// EntityManager em;
		// em.for_entities_with<Position, Direction>([](rift::Entity entity, Position& p, Direction& d){ *do something with the entity and its components* });
		template <class First, class... Rest>
		void for_entities_with(typename rift::impl::identity_t<std::function<void(Entity, First& first, Rest&... rest)>> f);

		// Cleanup the resources for every entity that was destroyed in the current frame
		void update() noexcept;

		// Clear the manager of all entities and their components
		void clear() noexcept;

	private:

		/*
		 *
		 * The following are internal functions that an Entity interfaces with.
		 * In all cases an entity is asserts it is valid before using any of them.
		 *
		 */
		
		// Include the component into the entity's component mask
		// Note:
		// - Creates a component cache for type C if it doesn't already exist
		template <class C, class... Args>
		void add_component(std::uint32_t index, Args&& ...args) noexcept;

		// Replace the entity's already existing component
		template <class C, class ...Args>
		void replace_component(std::uint32_t index, Args&& ...args) noexcept;

		// Remove the component from the entity's component mask
		// Note:
		// - Assumes there exists a component cache for the component type
		template <class C>
		void remove_component(std::uint32_t index) noexcept;

		// Check if the component is a part of the entity's component mask
		template <class C>
		bool has_component(std::uint32_t index) noexcept;

		// Fetch the component for the entity
		template <class C>
		C &get_component(std::uint32_t index) noexcept;

		// Fetch the ComponentMask for the entity
		ComponentMask component_mask_for(std::uint32_t index) const noexcept;

		// Checks if the entity is pending deletion
		bool pending_invalidation(std::uint32_t index) const noexcept;

		// Checks the validity of the entity
		bool valid_id(const Entity::ID& id) const noexcept;

		// Queue the id for recycling
		void destroy(std::uint32_t index) noexcept;

		/*
		 *
		 * The following are internal functions that the entity manager only uses
		 *
		 */

		 // Given a template parameter pack of Component types, this function returns the ComponentMask for those types
		 // example: ComponentMask mask = signature_for<Position, Direction>();
		 // Note:
		 // - The order of the types does not matter, the function will still return the same component mask. That is, if
		 //   classes A and B are any two subclasses of rift::Component, signature_for<A, B>() == signature_for<B, A>()
		template <class ...Components>
		static ComponentMask signature_for() noexcept;
		
		// Accommodates a new component
		// Note:
		// - Creates a new cache for type C if there does not exist one already
		template <class C, class ...Args>
		void accommodate_component(std::uint32_t index, std::size_t family_id, Args&& ...args) noexcept;

		// Remove the index from any search caches
		void erase_all_index_caches_for(std::uint32_t index);

		// Check if the manager is caching indices for the given signature
		bool contains_index_cache_for(const ComponentMask& sig) const;

		// Create a cache of indices for the given signature
		void create_index_cache_for(const ComponentMask& sig);
		
	private:

		// Collection of indices to invalidate before the next frame
		rift::impl::SparseSet invalid_indices;

		// Queue of indices to reuse
		std::queue<std::uint32_t> free_indices;

		// Collection of entity component masks 
		std::vector<ComponentMask> masks;

		// Collection of index versions
		std::vector<std::uint32_t> index_versions;

		// Collection of component caches
		std::vector<std::shared_ptr<rift::impl::BaseCache>> component_caches;

		// Collection of cached indices for faster queries
		std::unordered_map<ComponentMask, rift::impl::SparseSet> index_caches;
		
	};

	template<class C, class ...Args>
	inline void Entity::add(Args && ...args) const noexcept
	{
		assert(!has<C>());
		manager->add_component<C>(uid.index(), std::forward<Args>(args)...);
	}

	template<class C, class ...Args>
	inline void Entity::replace(Args && ...args) const noexcept
	{
		assert(has<C>());
		manager->replace_component<C>(uid.index(), std::forward<Args>(args)...);
	}

	template<class C>
	inline void Entity::remove() const noexcept
	{
		assert(has<C>());
		manager->remove_component<C>(uid.index());
	}

	template<class C>
	inline bool Entity::has() const noexcept
	{
		static_assert(std::is_base_of_v<BaseComponent, C>, "The component type does not inherit from rift::Component!");
		assert(valid() && "Cannot check if an invalid entity has a component type!");
		return manager->has_component<C>(uid.index());
	}

	template<class C>
	inline C & Entity::get() const noexcept
	{
		assert(has<C>());
		return manager->get_component<C>(uid.index());
	}

	template<class First, class ...Rest>
	inline std::size_t EntityManager::number_of_entities_with() const noexcept
	{
		auto sig = signature_for<First, Rest...>();
		if (contains_index_cache_for(sig)) {
			return index_caches.at(sig).size();
		}
		else {
			return std::accumulate(masks.begin(), masks.end(), std::size_t(0), 
			[&sig](std::size_t n, ComponentMask mask) {
				return (mask & sig) == sig ? ++n : n;
			});
		}
	}

	template<class First, class ...Rest>
	inline void EntityManager::for_entities_with(typename rift::impl::identity_t<std::function<void(Entity, First& first, Rest&...rest)>> f)
	{
		auto sig = signature_for<First, Rest...>();
		if (!contains_index_cache_for(sig))
			create_index_cache_for(sig);
		for (auto index : index_caches.at(sig))
			f(Entity(this, Entity::ID(index, index_versions[index])), get_component<First>(index), get_component<Rest>(index)...);
	}

	template<class C, class ...Args>
	inline void EntityManager::add_component(std::uint32_t index, Args && ...args) noexcept
	{
		auto family_id = C::family();
		auto mask = masks[index].set(family_id);

		// insert the component into the appropriate cache in the appropriate location
		accommodate_component<C>(index, family_id, std::forward<Args>(args)...);

		for (auto& index_cache : index_caches) {
			if (index_cache.first.test(family_id) && (mask & index_cache.first) == index_cache.first)
				index_cache.second.insert(index);
		}
	}

	template<class C, class ...Args>
	inline void EntityManager::replace_component(std::uint32_t index, Args && ...args) noexcept
	{
		component_caches[C::family()]->insert(index, C(std::forward<Args>(args)...));
	}

	template<class C>
	inline void EntityManager::remove_component(std::uint32_t index) noexcept
	{
		auto family_id = C::family();
		auto mask = masks[index];

		for (auto& index_cache : index_caches) {
			if (index_cache.first.test(family_id) && (mask & index_cache.first) == index_cache.first)
				index_cache.second.erase(index);
		}

		masks[index].reset(family_id);
	}

	template<class C>
	inline bool EntityManager::has_component(std::uint32_t index) noexcept
	{
		return masks[index].test(C::family());
	}

	template<class C>
	inline C & EntityManager::get_component(std::uint32_t index) noexcept
	{
		return static_cast<C&>(component_caches[C::family()]->at(index));
	}

	template<class ...Components>
	inline ComponentMask EntityManager::signature_for() noexcept
	{
		static_assert(rift::impl::all_of_v<std::is_base_of_v<BaseComponent, Components>...>,
			"Not all components inherit from rift::Component!");
		ComponentMask mask;
		[](...) {}((mask.set(Components::family()))...);
		return mask;
	}

	template<class C, class ...Args>
	inline void EntityManager::accommodate_component(std::uint32_t index, std::size_t family_id, Args&& ...args) noexcept
	{
		if (family_id >= component_caches.size())
			component_caches.resize(family_id + 1);
		if (!component_caches[family_id])
			component_caches[family_id] = std::make_shared<rift::impl::Cache<C>>();
		component_caches[family_id]->insert(index, C(std::forward<Args>(args)...));
	}

}
