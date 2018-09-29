#pragma once

#include <memory>
#include <unordered_map>

namespace rift {

	using SystemFamily = std::size_t;

	class EntityManager;

	// BaseSystem class
	// Defines the means for which all systems implement their logic
	// Note: this class should not be subclassed directly as systems need to be registered... See the System class. 
	class BaseSystem {
	public:
		virtual ~BaseSystem();

		// Where derived systems implement their logic
		virtual void update(EntityManager& em, double dt) = 0;

	protected:
		static SystemFamily m_family;
	};

	// The System class
	// Classes that are meant to be systems must inherit from this class for registration as a 'system'
	// example:
	// class MovementSystem : public System<MovementSystem> {}
	//
	template <class Derived>
	class System : public BaseSystem {
	public:
		virtual ~System() {}

		// Returns the System type id
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
		SystemManager();

		// Adds a new managed system
		// Note: Only one system per system type can be managed by any one system 
		// manager. If a given system manager already manages a system of type S, 
		// then if another or the same caller were to add another system of the 
		// same type S, then that system will replace the currently managed system
		// example:
		// SystemManager sys_mgr(entity_manager);
		// sys_mgr.add<MovementSystem>();
		// 
		template <class S, class... Args>
		void add(Args&& ...args) noexcept;

		// Removes a managed system if any
		// example:
		// SystemManager sys_mgr(entity_manager);
		// sys_mgr.remove<MovementSystem>();
		//
		template <class S>
		void remove() noexcept;

		// Checks if the manager has a system S
		// example:
		// SystemManager sm(em);
		// sm.add<MovementSystem>();
		// bool has = sm.has<MovementSystem>();
		// 
		template <class S>
		bool has() noexcept;

		// Retrieves the system of type S if any
		// example:
		// SystemManager sm(em);
		// sm.add<MovementSystem>();
		// std::shared_ptr<MovementSystem> ms = sm.get<MovementSystem>();
		// if (ms) {}
		//
		template <class S>
		std::shared_ptr<S> get() noexcept;

		// Updates all systems
		void update(EntityManager& em, double dt);

	private:
		std::unordered_map<SystemFamily, std::shared_ptr<BaseSystem>> systems;
	};

	template<class S, class ...Args>
	inline void SystemManager::add(Args && ...args) noexcept
	{
		systems.insert(std::make_pair(S::family(), std::make_shared<S>(std::forward<Args>(args)...)));
	}

	template<class S>
	inline void SystemManager::remove() noexcept
	{
		assert(has<S>() && "The system manager does not manage a system of type S.");
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
		return systems.find(S::system_id()) != systems.end() 
			   ? std::static_pointer_cast<S>(systems.at(S::family()))
			   : std::shared_ptr<S>(nullptr);
	}
}
