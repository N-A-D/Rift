namespace rift {

	template<class S, class ...Args>
	inline void SystemManager::add(Args && ...args) noexcept
	{
		assert(!has<S>() && "Cannot manage more than one system of a given type!");
		if (S::family() >= systems.size())
			systems.resize(S::family() + 1);
		systems[S::family()] = std::make_shared<S>(std::forward<Args>(args)...);
	}

	template<class S>
	inline void SystemManager::remove() noexcept
	{
		assert(has<S>() && "Cannot remove an unmanaged system type!");
		systems[S::family()] = nullptr;
	}

	template<class S>
	inline bool SystemManager::has() const noexcept
	{
		static_assert(std::is_base_of_v<BaseSystem, S>, "The system type does not inherit from rift::System!");
		if (S::family() >= systems.size())
			return false;
		if (systems[S::family()])
			return true;
		return false;
	}

	template<class S>
	inline std::shared_ptr<S> SystemManager::get() const noexcept
	{
		return std::static_pointer_cast<S>(fetch_system<S>());
	}

	template<class First, class ...Rest>
	inline void SystemManager::update(double dt) const noexcept
	{
		static_assert(rift::internal::all_of_v<std::is_base_of_v<BaseSystem, First>
			, std::is_base_of_v<BaseSystem, Rest>...>
			, "All systems must inherit from rift::System!");
		for (auto system : { fetch_system<First>(), fetch_system<Rest>()... }) {
			system->update(entity_manager, dt);
		}
		entity_manager.update();
	}

	template<class S>
	inline std::shared_ptr<BaseSystem> SystemManager::fetch_system() const noexcept
	{
		assert(has<S>() && "Cannot fetch an unmanaged system type!");
		return systems.at(S::family());
	}

}