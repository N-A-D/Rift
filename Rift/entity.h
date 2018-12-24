#pragma once

#include <queue>
#include <memory>
#include <cassert>
#include <iostream>
#include <execution>
#include <algorithm>
#include <functional>
#include <unordered_map>
#include "internal/pool.h"
#include "internal/sparse_set.h"
#include "internal/rift_traits.h"
#include "internal/noncopyable.h"

namespace rift {

	class EntityManager;

	// The Entity class
	// A proxy class for an index to a set of components.
	class Entity final {
	public:

		// The Entity::ID class
		// A versionable index.
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

		// Fetches the entity's id.
		Entity::ID id() const noexcept;

		// Checks if the entity is valid.
		bool valid() const noexcept;

		operator bool() const noexcept;

		// Checks if this entity will be invalid next frame.
		bool pending_invalidation() const noexcept;

		// Signals the manager to recycle this entity.
		void destroy() const noexcept;

		// Fetches the entity's ComponentMask.
		rift::ComponentMask component_mask() const noexcept;

		// Adds a component to the entity.
		// Note: 
		// - Asserts the entity does not own an instance of the component type.
		template <class C, class ...Args>
		void add(Args&& ...args) const noexcept;

		// Replaces the entity's existing component.
		// Note:
		// - Asserts the entity owns an instance of the component type.
		template <class C, class ...Args>
		void replace(Args&& ...args) const noexcept;

		// Removes the entity's component.
		// Note: 
		// - Asserts the entity owns an instance of the component type.
		template <class C>
		void remove() const noexcept;

		// Checks if the entity has a component.
		template <class C>
		bool has() const noexcept;

		// Fetches the entity's component.
		// Note: 
		// - Asserts the entity owns an instance of the component type.
		template <class C>
		C &get() const noexcept;

		bool operator<(const Entity& other) const noexcept { return uid < other.uid; }
		bool operator>(const Entity& other) const noexcept { return other < *this; }
		bool operator==(const Entity& other) const noexcept { return manager == other.manager && !(*this < other) && !(other < *this); }
		bool operator!=(const Entity& other) const noexcept { return !(*this == other); }

	private:
		friend class EntityManager;

		// Only EntityManagers are permitted to create valid entities.
		Entity(EntityManager *manager, Entity::ID uid) noexcept;

		// The manager that created this entity.
		EntityManager* manager = nullptr;

		// The entity's unique id.
		Entity::ID uid = INVALID_ID;

	};

	inline std::ostream& operator<<(std::ostream& os, const Entity::ID& id) {
		os << "Entity::ID(index=" << id.index() << ", version=" << id.version() << ")";
		return os;
	}

	inline std::ostream& operator<<(std::ostream& os, const Entity& entity) {
		os << "rift::Entity(" << entity.id() << ")";
		return os;
	}

	// The EntityManager class
	// Manages the lifecycle of entities.
	class EntityManager final : rift::impl::NonCopyable {
		friend class Entity;
	public:

		EntityManager() = default;

		// Creates a new Entity.
		Entity create_entity() noexcept;

		// Returns the number of managed entities.
		std::size_t size() const noexcept;

		// Returns the capacity (number of managed entities + those that can reused).
		std::size_t capacity() const noexcept;

		// Returns the number of entities that can be reused
		std::size_t number_of_reusable_entities() const noexcept;

		// Returns the number of entities waiting to be destroyed.
		std::size_t number_of_entities_to_destroy() const noexcept;

		// Returns the number of entities whose component mask includes each component type.
		template <class First, class... Rest>
		std::size_t number_of_entities_with() const noexcept;

		// Applies the function f on every entity whose component mask includes each component type.
		// Example:
		// EntityManager em;
		// em.for_entities_with<A, B>([](Entity e, A& a, B& b){ // Do something with the entity & its components });
		template <class First, class... Rest>
		void for_entities_with(rift::impl::identity_t<std::function<void(Entity, First& first, Rest&... rest)>> f);

		// Applies the function f on every entity whose component mask includes each component type.
		// Note:
		// - Application of the function is done in parallel
		// Example:
		// em.par_for_entities_with<A, B>([](A& a, B& b){ // Do something with the entity's components });
		template <class First, class... Rest>
		void par_for_entities_with(rift::impl::identity_t<std::function<void(First& first, Rest&... rest)>> f);

		// Recycles destroyed entities.
		// Note:
		// - This function must be called at the end of every frame.
		void update() noexcept;

		// Clears the manager of all entities.
		void clear() noexcept;

	private:

		// The following are internal functions every entity interface with.
		// An entity ensures that its Entity::ID is valid before invoking any of these.

		// Adds a component to an entity.
		// Note:
		// - Builds a new component pool if one does not exist for the component type.
		template <class C, class... Args>
		void add_component(std::uint32_t index, Args&& ...args) noexcept;

		// Replaces an entity's already existing component.
		// Note:
		// - Assumes there exists a component pool for the component type.
		template <class C, class ...Args>
		void replace_component(std::uint32_t index, Args&& ...args) noexcept;

		// Removes a component from an entity.
		// Note:
		// - Assumes there exists a component pool for the component type.
		template <class C>
		void remove_component(std::uint32_t index) noexcept;

		// Checks if an entity has a component.
		template <class C>
		bool has_component(std::uint32_t index) noexcept;

		// Fetches a component for an entity.
		template <class C>
		C &get_component(std::uint32_t index) noexcept;

		// Fetches the ComponentMask for an entity.
		ComponentMask component_mask_for(std::uint32_t index) const noexcept;

		// Checks if an entity will be recycled.
		bool pending_invalidation(std::uint32_t index) const noexcept;

		// Checks if an entity is valid.
		bool valid_id(const Entity::ID& id) const noexcept;

		// Queue an entity for recycling.
		void destroy(std::uint32_t index) noexcept;

		// The following are internal functions an entity manager only uses.

		// Given a template parameter pack of Component types, this function returns the ComponentMask for those types
		// Note:
		// - The order of the types does not matter, the function will still return the same component mask. That is, if
		//   classes A and B are any two subclasses of rift::Component, signature_for<A, B>() == signature_for<B, A>()
		// Example: 
		// ComponentMask mask = signature_for<Position, Direction>();
		template <class ...Components>
		static ComponentMask signature_for() noexcept;
		
		// Removes an index from any caches that contain it.
		void erase_caches_for(std::uint32_t index);

		// Checks if the manager is caching indices for the given signature.
		bool contains_cache_for(const ComponentMask& sig) const;

		// Creates a cache of indices for the given signature.
		void create_cache_for(const ComponentMask& sig);
		
	private:

		// Collection of indices to invalidate before the next frame.
		rift::impl::SparseSet invalid_indices;

		// Queue of indices to reuse.
		std::queue<std::uint32_t> free_indices;

		// Collection of entity component masks.
		std::vector<ComponentMask> masks;

		// Collection of index versions.
		std::vector<std::uint32_t> index_versions;

		// Collection of component pools.
		std::vector<std::unique_ptr<rift::impl::BasePool>> component_pools;

		// Collection of cached indices for faster system queries.
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
		static_assert(std::is_base_of_v<BaseComponent, C>, 
			"The component type does not inherit from rift::Component!");
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
		if (contains_cache_for(sig)) {
			return index_caches.at(sig).size();
		}
		else {
			return std::count_if(std::execution::par_unseq, masks.begin(), masks.end(), 
			[&sig](ComponentMask mask) {
				return (mask & sig) == sig;
			});
		}
	}

	template<class First, class ...Rest>
	inline void EntityManager::for_entities_with(rift::impl::identity_t<std::function<void(Entity, First& first, Rest&...rest)>> f)
	{
		auto sig = signature_for<First, Rest...>();
		if (!contains_cache_for(sig))
			create_cache_for(sig);
		for (auto index : index_caches.at(sig))
			f(Entity(this, Entity::ID(index, index_versions[index])),      // The entity
			  get_component<First>(index), get_component<Rest>(index)...); // The entity's components
	}

	template<class First, class ...Rest>
	inline void EntityManager::par_for_entities_with(rift::impl::identity_t<std::function<void(First&first, Rest&...rest)>> f)
	{
		auto sig = signature_for<First, Rest...>();
		if (!contains_cache_for(sig))
			create_cache_for(sig);
		
		auto& indices = index_caches.at(sig);
		// indices.sort(); // Might be needed... Not sure atm.
		
		// Apply the system transformation in parallel
		std::for_each(std::execution::par_unseq, indices.begin(), indices.end(), 
		[this, f](std::uint32_t index) {
			f(get_component<First>(index), get_component<Rest>(index)...);
		});

	}

	template<class C, class ...Args>
	inline void EntityManager::add_component(std::uint32_t index, Args && ...args) noexcept
	{
		auto family_id = C::family();
		auto mask = masks[index].set(family_id);

		// Build new component pool if necessary
		if (family_id >= component_pools.size())
			component_pools.resize(family_id + 1);
		if (!component_pools[family_id])
			component_pools[family_id] = std::make_unique<rift::impl::Pool<C>>();

		// Build a new component and insert it into the component pool
		component_pools[family_id]->insert(index, C(std::forward<Args>(args)...));

		// Add the entity into every existing index cache whose signature 
		// includes the component type and matches the entity's component mask
		for (auto& index_cache : index_caches) {
			if (index_cache.first.test(family_id) && (mask & index_cache.first) == index_cache.first)
				index_cache.second.insert(index);
		}
	}

	template<class C, class ...Args>
	inline void EntityManager::replace_component(std::uint32_t index, Args && ...args) noexcept
	{
		component_pools[C::family()]->insert(index, C(std::forward<Args>(args)...));
	}

	template<class C>
	inline void EntityManager::remove_component(std::uint32_t index) noexcept
	{
		auto family_id = C::family();
		auto mask = masks[index];

		// Remove the entity from every existing index cache whose signature 
		// includes the component type and matches the entity's component mask
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
		return static_cast<C&>(component_pools[C::family()]->at(index));
	}

	template<class ...Components>
	inline ComponentMask EntityManager::signature_for() noexcept
	{
		static_assert(rift::impl::all_of_v<std::is_base_of_v<BaseComponent, Components>...>,
			"All components must inherit from rift::Component!");
		ComponentMask mask;
		[](...) {}((mask.set(Components::family()))...);
		return mask;
	}

} // namespace rift

namespace std {
	template <> struct hash<rift::Entity>
	{
		std::size_t operator()(const rift::Entity& entity) const noexcept 
		{
			return entity.id().index() ^ entity.id().version();
		}
	};
	template <> struct hash<const rift::Entity>
	{
		std::size_t operator()(const rift::Entity& entity) const noexcept
		{
			return entity.id().index() ^ entity.id().version();
		}
	};
}