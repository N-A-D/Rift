#pragma once

#include <queue>
#include <memory>
#include <numeric>
#include <cassert>
#include <functional>
#include <type_traits>
#include <unordered_map>
#include "utility/cache.h"
#include "utility/signature.h"
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

		bool operator<(const Entity& other) const noexcept { return m_id < other.m_id; }
		bool operator>(const Entity& other) const noexcept { return other < *this; }
		bool operator==(const Entity& other) const noexcept { return mgr == other.mgr && !(*this < other) && !(other < *this); }
		bool operator!=(const Entity& other) const noexcept { return !(*this == other); }

	private:
		friend class EntityManager;

		// Only EntityManagers are permitted to create valid entity handles
		Entity(EntityManager *em, Entity::ID id);

		// The manager that created this entity
		EntityManager* mgr;

		// The entity's index. Only valid while the version is valid
		Entity::ID m_id;

	};

	// The EntityManager class
	// Manages the lifecycle of Entity handles
	class EntityManager final : rift::util::NonCopyable {
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
		std::size_t reusable_entities() const noexcept;

		// Returns the number of entities waiting to be destroyed
		std::size_t entities_to_destroy() const noexcept;

		// Returns the number of entities whose component mask includes each component type
		template <class First, class... Rest>
		std::size_t entities_with() const noexcept;

		// Applies the function f on entities whose component mask includes each component type
		// example:
		// EntityManager em;
		// em.for_each_entity_with<Position, Direction, Health>([](rift::Entity entity){ *do something with the entity* });
		template <class First, class... Rest>
		void for_each_entity_with(std::function<void(Entity)> f);

		// Cleanup the resources for entities that were destroyed last frame
		void update() noexcept;

	private:

		/*
		 *
		 * The following are internal functions that an Entity interfaces with.
		 * In all cases an entity is asserts it is valid before using any of them.
		 *
		 */
		
		// Enable the component type C in the entity's component mask
		template <class C, class... Args>
		void add_component(const Entity::ID& id, Args&& ...args) noexcept;

		// Replace the entity's already existing component
		template <class C, class ...Args>
		void replace_component(const Entity::ID& id, Args&& ...args) noexcept;

		// Disable the component type C in the entity's component mask
		template <class C>
		void remove_component(const Entity::ID& id) noexcept;

		// Check if the component type C is enabled in the entity's component mask
		template <class C>
		bool has_component(const Entity::ID& id) noexcept;

		// Fetch the component for the entity
		template <class C>
		C &get_component(const Entity::ID& id) noexcept;

		// Fetch the ComponentMask for the entity
		ComponentMask component_mask_for(const Entity::ID& id) const noexcept;

		// Checks if the entity is pending deletion
		bool pending_invalidation(const Entity::ID& id) const noexcept;

		// Check the validity of the entity
		bool valid_id(const Entity::ID& id) const noexcept;

		// Prep all entities with the same Entity::ID for invalidation
		void destroy(const Entity::ID& id) noexcept;

		/*
		 *
		 * The following are internal functions that the entity manager only uses
		 *
		 */

		// Returns the component pool for component type C
		template <class C>
		std::shared_ptr<rift::util::Cache<C>> component_cache_for(std::size_t id) noexcept;

		// Delete all components the entity owns
		void delete_components_for(const Entity::ID& id) noexcept;

		// Delete the entity from all search caches it may be in
		void delete_all_caches_for(const Entity::ID& id) noexcept;

		// Checks if a search cache for a given signature exists
		bool contains_search_cache_for(const ComponentMask& signature) const noexcept;

	private:

		// Collection of entities to be destroyed next frame
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
		std::unordered_map<ComponentMask, rift::util::Cache<Entity>> search_caches;
	};


	template<class C, class ...Args>
	inline void Entity::add(Args && ...args) const noexcept
	{
		assert(valid() && "Cannot add a component to an invalid entity!");
		assert(!has<C>() && "Adding multiple components of the same type to the same entity is not allowed!");
		mgr->add_component<C>(m_id, std::forward<Args>(args)...);
	}

	template<class C, class ...Args>
	inline void Entity::replace(Args && ...args) const noexcept
	{
		assert(valid() && "Cannot replace a component for an invalid entity!");
		assert(has<C>() && "The entity does own a component of the given type!");
		mgr->replace_component<C>(m_id, std::forward<Args>(args)...);
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
		static_assert(std::is_base_of<BaseComponent, C>::value, "The component type does not inherit from rift::Component!");
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
	inline std::size_t EntityManager::entities_with() const noexcept
	{
		auto signature = rift::util::signature_for<First, Rest...>();

		if (contains_search_cache_for(signature)) {
			return search_caches.at(signature).size();
		}
		else {
			return std::accumulate(masks.begin(), masks.end(), std::size_t(0), 
				[&signature](std::size_t n, ComponentMask mask) {
				return (mask & signature) == signature ? ++n : n;
			});
		}
	}

	template<class First, class ...Rest>
	inline void EntityManager::for_each_entity_with(std::function<void(Entity)> f)
	{
		auto signature = rift::util::signature_for<First, Rest...>();
	
		if (!contains_search_cache_for(signature)) {
			rift::util::Cache<Entity> search_cache;
			for (std::size_t i = 0; i < masks.size(); i++) {
				if ((masks[i] & signature) == signature) {
					Entity e(this, Entity::ID(static_cast<std::uint32_t>(i), index_versions[i]));
					search_cache.insert(i, &e);
				}
			}
			search_caches.emplace(signature, search_cache);
		}
		for (auto entity : search_caches.at(signature)) {
			f(entity);
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
		for (auto& search_cache : search_caches) {
			if (search_cache.first.test(family_id) &&
				(mask & search_cache.first) == search_cache.first) {
				search_cache.second.insert(index, &e);
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

		for (auto& search_cache : search_caches) {
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
	inline std::shared_ptr<rift::util::Cache<C>> EntityManager::component_cache_for(std::size_t id) noexcept
	{
		if (id >= component_caches.size())
			component_caches.resize(id + 1);
		if (!component_caches[id])
			component_caches[id] = std::make_shared<rift::util::Cache<C>>();
		return std::static_pointer_cast<rift::util::Cache<C>>(component_caches[id]);
	}

}
