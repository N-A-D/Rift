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

	inline bool Entity::pending_invalidation() const noexcept
	{
		assert(valid() && "Cannot check if an invalid entity is waiting to be invalidated!");
		return manager->pending_invalidation(uid.index());
	}

	inline void Entity::destroy() const noexcept
	{
		assert(valid() && "Cannot destroy and invalid entity!");
		manager->destroy(uid.index());
	}

	inline ComponentMask Entity::component_mask() const noexcept
	{
		assert(valid() && "Cannot get the component mask for an invalid entity!");
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
		auto mask = component_mask_for(original.id().index());
		for (std::size_t i = 0; i < mask.size(); ++i)
		{
			if (mask.test(i))
				component_operators[i]->copy_component_from(original, clone);
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
		for (auto index : invalid_indices) {
			erase_caches_for(index);
			masks[index].reset();
			index_versions[index]++;
			free_indices.push(index);
		}
		invalid_indices.clear();
	}

	inline void rift::EntityManager::clear() noexcept
	{
		invalid_indices.clear();
		while (!free_indices.empty())
			free_indices.pop();
		masks.clear();
		index_versions.clear();
		component_pools.clear();
		component_operators.clear();
		index_caches.clear();
	}

	inline std::size_t EntityManager::number_of_reusable_entities() const noexcept
	{
		return free_indices.size();
	}

	inline std::size_t EntityManager::number_of_entities_to_destroy() const noexcept
	{
		return invalid_indices.size();
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
	inline void EntityManager::for_entities_with(rift::internal::identity_t<std::function<void(Entity, First&, Rest&...)>> f)
	{
		auto sig = signature_for<First, Rest...>();
		if (!contains_cache_for(sig))
			create_cache_for(sig);
		auto& indices = index_caches.at(sig);
		// Apply the system transformation sequentially
		for (auto index : indices)
			f(Entity(this, Entity::ID(index, index_versions[index])),      // The entity
			  get_component<First>(index), get_component<Rest>(index)...); // The entity's components
	}

#ifdef RIFT_ENABLE_PARALLEL_TRANSFORMATIONS

	template<class First, class ...Rest>
	inline void EntityManager::par_for_entities_with(rift::internal::identity_t<std::function<void(First&, Rest&...)>> f)
	{
		auto sig = signature_for<First, Rest...>();
		if (!contains_cache_for(sig))
			create_cache_for(sig);
		auto& indices = index_caches.at(sig);
		// Apply the system transformation in parallel
		std::for_each(std::execution::par, indices.begin(), indices.end(),
			[this, f](std::uint32_t index) {
			f(get_component<First>(index), get_component<Rest>(index)...);
		});
	}

#endif // RIFT_ENABLE_PARALLEL_TRANSFORMATIONS

	template<class C, class ...Args>
	inline void EntityManager::add_component(std::uint32_t index, Args && ...args) noexcept
	{
		auto family_id = C::family();
		auto mask = masks[index].set(family_id);
		accommodate_component<C>();
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
		component_pools[C::family()]->replace(index, C(std::forward<Args>(args)...));
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
		static_assert(rift::internal::all_of_v<std::is_base_of_v<BaseComponent, Components>...>,
			"All components must inherit from rift::Component!");
		ComponentMask mask;
		[](...) {}((mask.set(Components::family()))...);
		return mask;
	}

	inline ComponentMask EntityManager::component_mask_for(std::uint32_t index) const noexcept
	{
		return masks[index];
	}

	inline bool EntityManager::pending_invalidation(std::uint32_t index) const noexcept
	{
		return invalid_indices.contains(index);
	}

	inline bool EntityManager::valid_id(const Entity::ID & id) const noexcept
	{
		return id.index() < masks.size() && index_versions[id.index()] == id.version();
	}

	inline void EntityManager::destroy(std::uint32_t index) noexcept
	{
		if (!invalid_indices.contains(index))
			invalid_indices.insert(index);
	}

	template<class C>
	inline void EntityManager::accommodate_component() noexcept
	{
		auto family_id = C::family();
		if (family_id >= component_pools.size()) 
			component_pools.resize(family_id + 1);
		if (family_id >= component_operators.size())
			component_operators.resize(family_id + 1);
		if (!component_pools[family_id]) 
			component_pools[family_id] = std::make_unique<rift::internal::Pool<C>>();
		if (!component_operators[family_id])
			component_operators[family_id] = std::make_unique<ComponentOperator<C>>();
	}

	inline void EntityManager::erase_caches_for(std::uint32_t index)
	{
		auto mask = component_mask_for(index);
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
		rift::internal::SparseSet indices;
		for (std::uint32_t i = 0; i < masks.size(); i++) {
			if ((masks[i] & sig) == sig)
				indices.insert(i);
		}
		index_caches.emplace(sig, indices);
	}

} // End of EntityManager definitions
