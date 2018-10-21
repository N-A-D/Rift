#pragma once
#ifndef _RIFT_SYSTEM_
#define _RIFT_SYSTEM_
#include <memory>
#include <assert.h>
#include <unordered_map>

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

	// Responsible for the management of a single instance of any system type
	// All Systems managed by a rift::SystemManager must subclass rift::System
	class SystemManager final {
	public:

		// Creates a new System manager
		SystemManager(rift::EntityManager& em);

		// Adds a new managed system
		// Note: 
		// - Asserts the system type is not managed
		// SystemManager sm();
		// sm.add<MovementSystem>();
		// 
		template <class S, class... Args>
		void add(Args&& ...args) noexcept;

		// Removes a managed system if any
		// example:
		// SystemManager sm();
		// sm.remove<MovementSystem>();
		// Note:
		// - Asserts the system type is managed
		template <class S>
		void remove() noexcept;

		// Checks if the manager has a system S
		// example:
		// SystemManager sm();
		// sm.add<MovementSystem>();
		// bool has = sm.has<MovementSystem>();
		// 
		template <class S>
		bool has() noexcept;

		// Retrieves the system of type S if any
		// example:
		// SystemManager sm();
		// sm.add<MovementSystem>();
		// std::shared_ptr<MovementSystem> ms = sm.get<MovementSystem>();
		// if (ms) {}
		// Note:
		// - Asserts the system type is managed
		template <class S>
		std::shared_ptr<S> get() noexcept;

		// Updates all systems
		void update(double dt);

	private:
		rift::EntityManager & entity_manager;
		std::unordered_map<SystemFamily, std::shared_ptr<BaseSystem>> systems;
	};

	template<class S, class ...Args>
	inline void SystemManager::add(Args && ...args) noexcept
	{
		assert(!has<S>() && "Cannot manager more than one system of a given type!");
		systems.insert(std::make_pair(S::family(), std::make_shared<S>(std::forward<Args>(args)...)));
	}

	template<class S>
	inline void SystemManager::remove() noexcept
	{
		assert(has<S>() && "Cannot remove an unmanaged system type!");
		systems.erase(S::family());
	}

	template<class S>
	inline bool SystemManager::has() noexcept
	{
		return systems.find(S::family()) != systems.end();
	}

	template<class S>
	inline std::shared_ptr<S> SystemManager::get() noexcept
	{
		assert(has<S>() && "Cannot fetch an unmanaged system type!");
		return std::static_pointer_cast<S>(systems.at(S::family()));
	}
}

#endif // !_RIFT_SYSTEM_