namespace rift {

	template<class S, class ...Args>
	inline void SystemManager::add(Args && ...args) noexcept
	{
		assert(!has<S>() && "Cannot manage more than one system of a given type!");
		auto family = system_family_for<S>();
		if (family >= systems.size())
			systems.resize(family + 1);
		systems[family] = std::make_shared<S>(std::forward<Args>(args)...);
	}

	template<class S>
	inline void SystemManager::remove() noexcept
	{
		assert(has<S>() && "Cannot remove an unmanaged system type!");
		systems[system_family_for<S>()].reset();
	}

	template<class S>
	inline bool SystemManager::has() const noexcept
	{
		static_assert(std::is_base_of_v<BaseSystem, S>, "All systems must inherit from rift::System!");
		auto family = system_family_for<S>();
		if (family >= systems.size())
			return false;
		if (systems[family])
			return true;
		return false;
	}

	template<class S>
	inline std::shared_ptr<S> SystemManager::get() const noexcept
	{
		return std::static_pointer_cast<S>(fetch_system<S>());
	}

	inline void SystemManager::update_all(DELTA_TIME_TYPE dt) const noexcept
	{
		// Uppdate all valid systems
		for (auto system : systems) {
			if (system) {
				system->update(entity_manager, dt);
			}
		}
		// Update the entity manager
		entity_manager.update();
	}

	template<class First, class ...Rest>
	inline void SystemManager::update(DELTA_TIME_TYPE dt) const noexcept
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
		return systems[system_family_for<S>()];
	}

	template<class S>
	inline BaseSystem::Family SystemManager::system_family_for() noexcept
	{
		return System<S>::family();
	}

}