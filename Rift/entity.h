#pragma once

#include <stack>
#include <vector>
#include <memory>
#include <bitset>
#include <cassert>
#include <iostream>
#include <execution>
#include <algorithm>
#include <functional>
#include <unordered_map>
#include "config/config.h"
#include "internal/pool.h"
#include "internal/component.h"
#include "internal/sparse_set.h"
#include "internal/noncopyable.h"

namespace rift {

	using ComponentMask = std::bitset<MAX_COMPONENTS>;

	class EntityManager;

	// The Entity class
	// A proxy class for an index to a set of components.
	class Entity final {
	public:

		// The Entity::ID class
		// A versionable index.
		class ID final {
		public:
			ID() = default;
			ID(const ID&) = default;
			ID(std::uint32_t index, std::uint32_t version) noexcept
				: num(std::uint64_t(index) << 0x20UL | std::uint64_t(version)) {}

			ID& operator=(const ID&) = default;

			std::uint32_t index() const noexcept { return num >> 0x20UL; }
			std::uint32_t version() const noexcept { return num & 0xFFFFFFFFUL; }
			std::uint64_t number() const noexcept { return num; }

			bool operator<(const ID& other) const noexcept { return num < other.num; }
			bool operator>(const ID& other) const noexcept { return other < *this; }
			bool operator==(const ID& other) const noexcept { return !(*this < other) && !(other < *this); }
			bool operator!=(const ID& other) const noexcept { return !(*this == other); }

		private:
			std::uint64_t num = 0;
		};

		inline static Entity::ID INVALID_ID;

		Entity() = default;
		Entity(const Entity&) = default;
		Entity& operator=(const Entity&) = default;

		// Fetches the entity's id.
		ID id() const noexcept;

		// Returns the entity's hash code.
		std::size_t hash() const noexcept;

		// Checks if the entity is valid.
		bool valid() const noexcept;

		operator bool() const noexcept;

		// Checks if the entity will be destroyed on the next manager update.
		bool marked_for_destruction() const noexcept;

		// Signals the manager to destroy this entity.
		// Note:
		// - The entity remains valid UNTIL its EntityManager updates.
		// - Not thread safe.
		void destroy() const noexcept;

		// Fetches the entity's ComponentMask.
		ComponentMask component_mask() const noexcept;

		// Adds a component to the entity.
		// Note: 
		// - Asserts the entity does not own an instance of the component type.
		// - Not thread safe.
		template <class C, class ...Args>
		void add(Args&& ...args) const noexcept;

		// Replaces the entity's existing component.
		// Note:
		// - Asserts the entity owns an instance of the component type.
		// - Not thread safe.
		template <class C, class ...Args>
		void replace(Args&& ...args) const noexcept;

		// Removes the entity's component.
		// Note: 
		// - Asserts the entity owns an instance of the component type.
		// - Not thread safe.
		template <class C>
		void remove() const noexcept;

		// Checks if the entity has a component.
		// Note:
		// - Asserts the entity is valid.
		template <class C>
		bool has() const noexcept;

		// Fetches the entity's component.
		// Note: 
		// - Asserts the entity owns an instance of the component type.
		// - Multiple writers is not thread safe.
		template <class C>
		C &get() const noexcept;

		bool operator<(const Entity& other) const noexcept { return uid < other.uid; }
		bool operator>(const Entity& other) const noexcept { return other < *this; }
		bool operator==(const Entity& other) const noexcept { 
			return manager == other.manager && !(*this < other) && !(other < *this); 
		}
		bool operator!=(const Entity& other) const noexcept { return !(*this == other); }

	private:
		friend class EntityManager;

		// Only EntityManagers are permitted to create valid entities.
		Entity(EntityManager *manager, Entity::ID uid) noexcept : manager(manager), uid(uid) {}

		// The manager that created this entity.
		EntityManager* manager = nullptr;

		// The entity's unique id.
		Entity::ID uid = INVALID_ID;

	};
	
	namespace internal {

		// Wrapper type to support lamba to std::function conversion
		template <class T> struct identity { using type = T; };
		template <class T> using identity_t = typename identity<T>::type;

	}

	// The EntityManager class
	// Manages entities and their components.
	class EntityManager final : internal::NonCopyable {
	public:

		EntityManager() = default;

		// Creates a new Entity.
		// Note:
		// - Not thread safe.
		Entity create_entity() noexcept;
		
		// Creates a new Entity by copying another entity.
		// Note:
		// - The entity's components are copy constructed from the original.
		// - Not thread safe.
		Entity create_copy_of(const Entity& original) noexcept;

		// Returns the number of valid entities.
		std::size_t size() const noexcept;

		// Checks if the manager has any valid entities.
		bool empty() const noexcept;

		// Returns the maximum possible number of valid entities.
		std::size_t max_size() const noexcept;

		// Returns the number of valid entities that can be held by the manager currently.
		std::size_t capacity() const noexcept;

		// Finalizes the destruction of any entity destroyed since the last update.
		// Note:
		// - This function must be called at the end of every frame.
		// - Not thread safe.
		void update() noexcept;

		// Clears the manager of all entities (valid/reusable).
		// Note:
		// - Not thread safe.
		void clear() noexcept;

		// Returns the number of entities that can be reused.
		std::size_t number_of_reusable_entities() const noexcept;

		// Returns the number of entities waiting to be destroyed.
		std::size_t number_of_entities_to_destroy() const noexcept;

		// Returns the number of entities whose component mask includes each component type.
		template <class First, class... Rest>
		std::size_t number_of_entities_with() const noexcept;

		// Applies the function f on every entity whose component mask includes each component type.
		// Example:
		// em.for_entities_with<A, B>([](Entity e, A& a, B& b){ /*Do something with the entity & its components*/ });
		template <class First, class... Rest>
		void for_entities_with(internal::identity_t<std::function<void(Entity, First&, Rest&...)>> f);

		// Applies the function f on every entity whose component mask includes each component type.
		// Note:
		// - Application of the function is done in parallel
		// Example:
		// em.par_for_entities_with<A, B>([](A& a, B& b){ /*Do something with the entity's components*/ });
		template <class First, class... Rest>
		void par_for_entities_with(internal::identity_t<std::function<void(First&, Rest&...)>> f);

	private:

		friend class Entity;

		// The following are internal functions every entity interfaces with.
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

		// Checks if an entity will be destroyed on the next update.
		bool marked_for_destruction(std::uint32_t index) const noexcept;

		// Checks if an entity is valid.
		bool valid_id(const Entity::ID& id) const noexcept;

		// Mark an entity for destruction.
		void destroy(std::uint32_t index) noexcept;

	private:

		// The following are internal functions an entity manager only uses.

		// Given a template parameter pack of Component types, this function returns the ComponentMask for those types
		// Note:
		// - The order of types does not matter, the function will still return the same component mask. That is, if
		//   classes A and B are any two subclasses of rift::Component, signature_for<A, B>() == signature_for<B, A>()
		// Example: 
		// ComponentMask mask = signature_for<Position, Direction>();
		template <class ...Components>
		static ComponentMask signature_for() noexcept;

		// Returns the component family for a given type.
		template <class C>
		static internal::BaseComponent::Family component_family_for() noexcept;

		// Removes an index from any caches that contain it.
		void erase_caches_for(std::uint32_t index);

		// Checks if the manager is caching indices for the given signature.
		bool contains_cache_for(const ComponentMask& sig) const;

		// Creates a cache of indices for the given signature.
		void create_cache_for(const ComponentMask& sig);

	private:

		// Collection of entity component masks.
		std::vector<ComponentMask> masks;

		// Collection of indices to destroy on the next update.
		rift::internal::SparseSet indices_to_destroy;

		// Collection of index versions.
		std::vector<std::uint32_t> index_versions;

		// Stack of indices to reuse.
		std::stack<std::uint32_t, std::vector<std::uint32_t>> free_indices;

		// Collection of component pools.
		std::vector<std::unique_ptr<rift::internal::BasePool>> component_pools;

		// Collection of cached indices for faster system queries.
		std::unordered_map<ComponentMask, rift::internal::SparseSet> index_caches;
		
	};

} // namespace rift
#include "entity.inl"

inline std::ostream& operator<<(std::ostream& os, const rift::Entity::ID& id) {
	os << "ID(index=" << id.index() << ",version=" << id.version() << ")";
	return os;
}

inline std::ostream& operator<<(std::ostream& os, const rift::Entity& entity) {
	os << "Entity(" << entity.id() << ")";
	return os;
}

namespace std {
	template <> struct hash<rift::Entity>
	{
		std::size_t operator()(const rift::Entity& entity) const noexcept 
		{
			return entity.hash();
		}
	};
	template <> struct hash<const rift::Entity>
	{
		std::size_t operator()(const rift::Entity& entity) const noexcept
		{
			return entity.hash();
		}
	};
}