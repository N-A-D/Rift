namespace rift { // Entity definitions

	inline Entity::ID Entity::id() const noexcept
	{
		return uid;
	}

	inline std::size_t Entity::hash() const noexcept
	{
		return std::size_t(uid.index() ^ uid.version());
	}

	inline bool Entity::valid() const noexcept
	{
		return manager && manager->valid_id(uid);
	}

	inline Entity::operator bool() const noexcept
	{
		return valid();
	}

	inline bool Entity::marked_for_destruction() const noexcept
	{
		assert(valid() && "Invalid entities are not allowed to check if they are marked for destruction!");
		return manager->marked_for_destruction(uid.index());
	}

	inline void Entity::destroy() const noexcept
	{
		assert(valid() && "Invalid entities are not allowed to destroy themselves!");
		manager->destroy(uid.index());
	}

	inline ComponentMask Entity::component_mask() const noexcept
	{
		assert(valid() && "Invalid entities do not have any components!");
		return manager->component_mask_for(uid.index());
	}

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
		assert(valid() && "Invalid entities do not have any components!");
		return manager->has_component<C>(uid.index());
	}

	template<class C>
	inline C& Entity::get() const noexcept
	{
		assert(has<C>());
		return manager->get_component<C>(uid.index());
	}

} // End of Entity definitions

namespace rift { // EntityManager definitions

	inline Entity EntityManager::create_entity() noexcept
	{
		std::uint32_t index, version;
		if (free_indices.empty()) {
			index = static_cast<std::uint32_t>(masks.size());
			version = 1;
			masks.push_back(0);
			index_versions.push_back(version);
		}
		else {
			index = free_indices.top();
			version = index_versions[index];
			free_indices.pop();
		}
		return Entity(this, Entity::ID(index, version));
	}

	inline Entity EntityManager::create_copy_of(const Entity& original) noexcept
	{
		assert(original && "Cannot create a copy of an invalid entity!");
		auto clone = create_entity();
		auto clone_index = clone.id().index();
		auto original_index = original.id().index();

		// Clones have the same ComponentMask as the original.
		masks[clone_index] = masks[original_index];

		// Insert the clone into every search cache the original is in.
		auto& mask = masks[clone_index];
		for (auto& index_cache : index_caches) {
			if ((index_cache.first & mask) == index_cache.first) {
				index_cache.second.insert(clone_index);
			}
		}

		// Copy every component from the original to the clone.
		for (std::size_t family = 0; family < mask.size(); ++family) {
			if (mask.test(family)) {
				component_pools[family]->insert(clone_index, component_pools[family]->at(original_index));
			}
		}
	
		return clone;
	}

	inline std::size_t EntityManager::size() const noexcept
	{
		return masks.size() - free_indices.size();
	}

	inline bool EntityManager::empty() const noexcept
	{
		return size() == 0;
	}

	inline std::size_t EntityManager::max_size() const noexcept
	{
		return masks.max_size();
	}

	inline std::size_t EntityManager::capacity() const noexcept
	{
		return masks.capacity();
	}

	inline void rift::EntityManager::update() noexcept
	{
		for (auto index : indices_to_destroy) {
			erase_caches_for(index);
			masks[index].reset();
			index_versions[index]++;
			free_indices.push(index);
		}
		indices_to_destroy.clear();
	}

	inline void rift::EntityManager::clear() noexcept
	{
		indices_to_destroy.clear();
		while (!free_indices.empty())
			free_indices.pop();
		masks.clear();
		index_versions.clear();
		component_pools.clear();
		index_caches.clear();
	}

	inline std::size_t EntityManager::number_of_reusable_entities() const noexcept
	{
		return free_indices.size();
	}

	inline std::size_t EntityManager::number_of_entities_to_destroy() const noexcept
	{
		return indices_to_destroy.size();
	}

	template<class First, class ...Rest>
	inline std::size_t EntityManager::number_of_entities_with() const noexcept
	{
		auto sig = signature_for<First, Rest...>();
		if (contains_cache_for(sig)) {
			return index_caches.at(sig).size();
		}
		else {
			return std::count_if(masks.begin(), masks.end(),
				[&sig](ComponentMask mask) {
				return (mask & sig) == sig;
			});
		}
	}

	template<class First, class ...Rest>
	inline void EntityManager::for_entities_with(internal::identity_t<std::function<void(Entity, First&, Rest&...)>> f)
	{
		auto sig = signature_for<First, Rest...>();
		if (!contains_cache_for(sig))
			create_cache_for(sig);
		auto& indices = index_caches.at(sig);
		// Apply the system transformation sequentially.
		for (auto index : indices)
			f(Entity(this, Entity::ID(index, index_versions[index])),      // The entity
			  get_component<First>(index), get_component<Rest>(index)...); // The entity's components
	}

	template<class First, class ...Rest>
	inline void EntityManager::par_for_entities_with(internal::identity_t<std::function<void(First&, Rest&...)>> f)
	{
		auto sig = signature_for<First, Rest...>();
		if (!contains_cache_for(sig))
			create_cache_for(sig);
		auto& indices = index_caches.at(sig);
		// Apply the system transformation in parallel.
		std::for_each(std::execution::par, indices.begin(), indices.end(),
		[this, f](std::uint32_t index) {
			f(get_component<First>(index), get_component<Rest>(index)...);
		});
	}

	template<class C, class ...Args>
	inline void EntityManager::add_component(std::uint32_t index, Args && ...args) noexcept
	{
		auto family_id = component_family_for<C>();
		auto& mask = masks[index].set(family_id);

		// Ensure there is a component pool for the component type.
		if (family_id >= component_pools.size())
			component_pools.resize(family_id + 1);
		if (!component_pools[family_id])
			component_pools[family_id] = std::make_unique<internal::Pool<C>>();

		// Insert the component into the pool for the type.
		C component(std::forward<Args>(args)...);
		component_pools[component_family_for<C>()]->insert(index, &component);

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
		C component(std::forward<Args>(args)...);
		component_pools[component_family_for<C>()]->replace(index, &component);
	}

	template<class C>
	inline void EntityManager::remove_component(std::uint32_t index) noexcept
	{
		auto family_id = component_family_for<C>();
		auto& mask = masks[index];

		// Remove the entity from every existing index cache whose signature 
		// includes the component type and matches the entity's component mask
		for (auto& index_cache : index_caches) {
			if (index_cache.first.test(family_id) && (mask & index_cache.first) == index_cache.first)
				index_cache.second.erase(index);
		}

		mask.reset(family_id);
	}

	template<class C>
	inline bool EntityManager::has_component(std::uint32_t index) noexcept
	{
		return masks[index].test(component_family_for<C>());
	}

	template<class C>
	inline C& EntityManager::get_component(std::uint32_t index) noexcept
	{
		return *(static_cast<C*>(component_pools[component_family_for<C>()]->at(index)));
	}

	template<class ...Components>
	inline ComponentMask EntityManager::signature_for() noexcept
	{
		ComponentMask mask;
		(mask.set(component_family_for<Components>()), ...);
		return mask;
	}

	template<class C>
	inline internal::BaseComponent::Family EntityManager::component_family_for() noexcept
	{
		return internal::Component<C>::family();
	}

	inline ComponentMask EntityManager::component_mask_for(std::uint32_t index) const noexcept
	{
		return masks[index];
	}

	inline bool EntityManager::marked_for_destruction(std::uint32_t index) const noexcept
	{
		return indices_to_destroy.contains(index);
	}

	inline bool EntityManager::valid_id(const Entity::ID & id) const noexcept
	{
		return index_versions[id.index()] == id.version();
	}

	inline void EntityManager::destroy(std::uint32_t index) noexcept
	{
		if (!indices_to_destroy.contains(index))
			indices_to_destroy.insert(index);
	}

	inline void EntityManager::erase_caches_for(std::uint32_t index)
	{
		auto& mask = masks[index];
		for (auto& index_cache : index_caches) {
			if ((mask & index_cache.first) == index_cache.first)
				index_cache.second.erase(index);
		}
	}

	inline bool EntityManager::contains_cache_for(const ComponentMask & sig) const
	{
		return index_caches.find(sig) != index_caches.end();
	}

	inline void EntityManager::create_cache_for(const ComponentMask & sig)
	{
		internal::SparseSet indices;
		for (std::uint32_t i = 0; i < masks.size(); i++) {
			if ((masks[i] & sig) == sig)
				indices.insert(i);
		}
		index_caches.emplace(sig, indices);
	}

} // End of EntityManager definitions
