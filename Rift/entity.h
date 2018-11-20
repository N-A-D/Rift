#pragma once

#include <queue>
#include <memory>
#include <numeric>
#include <cassert>
#include <functional>
#include "component.h"
#include <unordered_map>
#include "utility/cache.h"
#include "utility/rift_traits.h"
#include "utility/noncopyable.h"

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

		// The entity's index. Only valid while the version is valid
		Entity::ID uid = INVALID_ID;

	};

	// The EntityManager class
	// Manages the lifecycle of Entity handles
	class EntityManager final : rift::util::NonCopyable {
		friend class Entity;
	public:

		EntityManager() = default;
		EntityManager(std::size_t starting_size) noexcept;

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
		// em.entities_with<Position, Direction, Health>([](rift::Entity entity){ *do something with the entity* });
		template <class First, class... Rest>
		void entities_with(std::function<void(Entity)> f);

		// Cleanup the resources for entities that were destroyed last frame
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
		
		// Include the compnoent into the entity's component mask
		template <class C, class... Args>
		void add_component(const Entity::ID& id, Args&& ...args) noexcept;

		// Replace the entity's already existing component
		template <class C, class ...Args>
		void replace_component(const Entity::ID& id, Args&& ...args) noexcept;

		// Remove the component from the entity's component mask
		template <class C>
		void remove_component(const Entity::ID& id) noexcept;

		// Check if the component is a part of the entity's component mask
		template <class C>
		bool has_component(const Entity::ID& id) noexcept;

		// Fetch the component for the entity
		template <class C>
		C &get_component(const Entity::ID& id) noexcept;

		// Fetch the ComponentMask for the entity
		ComponentMask component_mask_for(const Entity::ID& id) const noexcept;

		// Checks if the entity is pending deletion
		bool pending_invalidation(const Entity::ID& id) const noexcept;

		// Checks the validity of the entity
		bool valid_id(const Entity::ID& id) const noexcept;

		// Queue the id for recycling
		void destroy(const Entity::ID& id) noexcept;

		/*
		 *
		 * The following are internal functions that the entity manager only uses
		 *
		 */


		 // Given a template parameter pack of Component types, this function returns the ComponentMask for those types
		 // example: ComponentMask mask = signature_for<Position, Direction>();
		 // Note:
		 // - Ordering of the types does not matter, the function will still return the same component mask. That is, if
		 //   classes A and B are any two subclasses of rift::Component, signature_for<A, B>() == signature_for<B, A>()
		template <class ...Components>
		static ComponentMask signature_for() noexcept;
			
		// Delete all components that belong to the entity
		void delete_components_for(const Entity::ID& id) noexcept;

		// Remove the entity from any entity caches
		void delete_all_caches_for(const Entity::ID& id) noexcept;

		// Returns the component pool for component type C
		template <class C>
		std::shared_ptr<rift::util::Cache<C>> component_cache_for(std::size_t id) noexcept;

		// Checks if a cache of entities exists for the given signature
		bool contains_entity_cache_for(const ComponentMask& signature) const noexcept;
	
		// Caches all entities whose component mask matches the given signature
		void create_entity_cache_for(const ComponentMask& signature) noexcept;

	private:

		// Collection of Entity::IDs to recycle in the current frame
		rift::util::Cache<Entity::ID> invalid_ids;

		// Collection of free indexes
		std::queue<std::uint32_t> free_indexes;

		// Collection of component masks
		std::vector<ComponentMask> masks;

		// Collection of index versions
		std::vector<std::uint32_t> index_versions;

		// The component caches
		std::vector<std::shared_ptr<rift::util::BaseCache>> component_caches;

		// Entity search caches
		std::unordered_map<ComponentMask, rift::util::Cache<Entity>> entity_caches;
	};


	template<class C, class ...Args>
	inline void Entity::add(Args && ...args) const noexcept
	{
		assert(valid() && "Cannot add a component to an invalid entity!");
		assert(!has<C>() && "Adding multiple components of the same type to the same entity is not allowed!");
		manager->add_component<C>(uid, std::forward<Args>(args)...);
	}

	template<class C, class ...Args>
	inline void Entity::replace(Args && ...args) const noexcept
	{
		assert(valid() && "Cannot replace a component for an invalid entity!");
		assert(has<C>() && "The entity does own a component of the given type!");
		manager->replace_component<C>(uid, std::forward<Args>(args)...);
	}

	template<class C>
	inline void Entity::remove() const noexcept
	{
		assert(valid() && "Cannot remove a component from an invalid entity!");
		assert(has<C>() && "The entity does not own a component of the given type!");
		manager->remove_component<C>(uid);
	}

	template<class C>
	inline bool Entity::has() const noexcept
	{
		static_assert(std::is_base_of<BaseComponent, C>::value, "The component type does not inherit from rift::Component!");
		assert(valid() && "Cannot check if an invalid entity has a component!");
		return manager->has_component<C>(uid);
	}

	template<class C>
	inline C & Entity::get() const noexcept
	{
		assert(valid() && "Cannot get a compnent for an invalid entity!");
		assert(has<C>() && "The entity does not have a component of the given type!");
		return manager->get_component<C>(uid);
	}

	template<class First, class ...Rest>
	inline std::size_t EntityManager::number_of_entities_with() const noexcept
	{
		auto signature = signature_for<First, Rest...>();

		if (contains_entity_cache_for(signature)) {
			return entity_caches.at(signature).size();
		}
		else {
			return std::accumulate(masks.begin(), masks.end(), std::size_t(0), 
				[&signature](std::size_t n, ComponentMask mask) {
				return (mask & signature) == signature ? ++n : n;
			});
		}
	}

	template<class First, class ...Rest>
	inline void EntityManager::entities_with(std::function<void(Entity)> f)
	{
		auto signature = signature_for<First, Rest...>();
	
		if (!contains_entity_cache_for(signature)) 
			create_entity_cache_for(signature);
		
		for (auto entity : entity_caches.at(signature)) 
			f(entity);
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
		for (auto& entity_cache : entity_caches) {
			if (entity_cache.first.test(family_id) &&
				(mask & entity_cache.first) == entity_cache.first) {
				entity_cache.second.insert(index, &e);
			}
		}
	}

	template<class C, class ...Args>
	inline void EntityManager::replace_component(const Entity::ID & id, Args && ...args) noexcept
	{
		auto index = id.index();
		auto family = C::family();
		auto component = C(std::forward<Args>(args)...);
		auto component_cache = component_cache_for<C>(family);
		component_cache->erase(index);
		component_cache->insert(index, &component);
	}

	template<class C>
	inline void EntityManager::remove_component(const Entity::ID & id) noexcept
	{
		auto family_id = C::family();
		auto index = id.index();
		auto mask = masks[index];

		for (auto& entity_cache : entity_caches) {
			if (entity_cache.first.test(family_id) &&
				(mask & entity_cache.first) == entity_cache.first) {
				entity_cache.second.erase(index);
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

	template<class ...Components>
	inline ComponentMask EntityManager::signature_for() noexcept
	{
		static_assert(rift::util::static_all_of<std::is_base_of<BaseComponent, Components>::value...>::value,
			"Not all components inherit from rift::Component!");
		ComponentMask mask;
		[](...) {}((mask.set(Components::family()))...);
		return mask;
	}

	template<class C>
	inline std::shared_ptr<rift::util::Cache<C>> EntityManager::component_cache_for(std::size_t id) noexcept
	{
		if (id >= component_caches.size())
			component_caches.resize(id + 1);
		if (!component_caches[id])
			component_caches[id] = std::make_shared<rift::util::Cache<C>>();
		return std::static_pointer_cast<rift::util::Cache<C>>(component_caches[id]);
	}

}
