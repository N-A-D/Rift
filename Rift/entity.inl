namespace rift {

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
			return std::count_if(masks.begin(), masks.end(),
				[&sig](ComponentMask mask) {
				return (mask & sig) == sig;
			});
		}
	}

	template<class First, class ...Rest>
	inline void EntityManager::for_entities_with(rift::internal::identity_t<std::function<void(Entity, First& first, Rest&...rest)>> f)
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
	inline void EntityManager::par_for_entities_with(rift::internal::identity_t<std::function<void(First&first, Rest&...rest)>> f)
	{
		auto sig = signature_for<First, Rest...>();
		if (!contains_cache_for(sig))
			create_cache_for(sig);
		auto& indices = index_caches.at(sig);
		// Apply the system transformation in parallel
		std::for_each(std::execution::par_unseq, indices.begin(), indices.end(),
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

		// Build new component pool if necessary
		if (family_id >= component_pools.size())
			component_pools.resize(family_id + 1);
		if (!component_pools[family_id])
			component_pools[family_id] = std::make_unique<rift::internal::Pool<C>>();

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

}
