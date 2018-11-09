#pragma once

#include <vector>
#include <memory>
#include <cassert>
#include <type_traits>
#include "utility/non_copyable.h"

namespace rift {

	using SystemFamily = std::size_t;

	class EntityManager;

	// BaseSystem class
	// Defines the means for which all systems implement their logic
	// Note: this class should not be subclassed directly as systems need to be registered... See the System class. 
	class BaseSystem {
	public:
		virtual ~BaseSystem() = default;

		// Where derived systems implement their logic
		virtual void update(EntityManager& em, double dt) = 0;

	protected:
		static SystemFamily m_family;
	};

	class SystemManager;

	// The System class
	// Classes that are meant to be systems must inherit from this class for registration as a 'system'
	// example:
	// class MovementSystem : public System<MovementSystem> {}
	//
	template <class Derived>
	class System : public BaseSystem {
	public:
		virtual ~System() = default;

	private:
		friend class SystemManager;
		// Returns the System type id
		// Used only by the system manager
		static SystemFamily family() noexcept {
			static SystemFamily system_family = m_family++;
			return system_family;
		}
	};

	// Manages different system types
	class SystemManager final : rift::util::NonCopyable {
	public:

		// Creates a new System manager
		SystemManager(rift::EntityManager& em);

		// Adds a new managed system
		// Note: 
		// - Asserts the system type is not already managed
		// Example:
		// EntityManager em;
		// ...
		// SystemManager sm(em);
		// sm.add<MovementSystem>();
		template <class S, class... Args>
		void add(Args&& ...args) noexcept;

		// Removes a managed system if any
		//
		// Note:
		// - Asserts the system type is managed
		//
		// Example:
		// EntityManager em;
		// ...
		// SystemManager sm(em);
		// sm.remove<MovementSystem>();
		template <class S>
		void remove() noexcept;

		// Checks if the manager has a system S
		//
		// Example:
		// EntityManager em;
		// ...
		// SystemManager sm(em);
		// sm.add<MovementSystem>();
		// bool has = sm.has<MovementSystem>();
		template <class S>
		bool has() const noexcept;

		// Retrieves the system of type S if any
		// Note:
		// - Asserts the system type is managed
		// Example:
		// EntityManager em;
		// ...
		// SystemManager sm(em);
		// sm.add<MovementSystem>();
		// std::shared_ptr<MovementSystem> ms = sm.get<MovementSystem>();
		// if (ms) {}
		template <class S>
		std::shared_ptr<S> get() const noexcept;

		// Updates all systems
		void update(double dt);
		
		// Update an ordered list of managed system types
		// Note:
		// - Asserts that each system type is managed
		// Example:
		// EntityManager em;
		// ...
		// SystemManager sm(em);
		// sm.update_systems<Movement, Collision>(dt);
		template <class First, class... Rest>
		void update_systems(double dt);
		
	private:

		// Fetch a generic pointer to a specific system
		template <class S>
		std::shared_ptr<BaseSystem> fetch_system() const noexcept;

		rift::EntityManager& entity_manager;
		std::vector<std::shared_ptr<BaseSystem>> systems;
	};

	template<class S, class ...Args>
	inline void SystemManager::add(Args && ...args) noexcept
	{
		assert(!has<S>() && "Cannot manager more than one system of a given type!");
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
		static_assert(std::is_base_of<BaseSystem, S>::value, "Invalid system type!");
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
	inline void SystemManager::update_systems(double dt)
	{
		auto system_list = { (fetch_system<First>()), (fetch_system<Rest>())... };
		for (auto system : system_list) {
			system->update(entity_manager, dt);
		}
		entity_manager.update();
	}

	template<class S>
	inline std::shared_ptr<BaseSystem> SystemManager::fetch_system() const noexcept
	{
		assert(has<S>() && "Cannot update an unmanaged system type!");
		return systems.at(S::family());
	}
}
