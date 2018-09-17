#pragma once

#include <memory>
#include <unordered_map>

namespace rift {

	using SystemFamily = std::size_t;

	class EntityManager;

	// BaseSystem class
	// Defines the means for which all systems implement their logic
	// Note: Should not  really be used directly
	class BaseSystem {
	public:

		// To allow destruction
		virtual ~BaseSystem();

		// Where derived systems implement their logic
		virtual void update(EntityManager& em, double dt) = 0;

	protected:
		static SystemFamily m_family;
	};

	// Subclasses should inherit from this class
	// example:
	// class MovementSystem : public System<MovementSystem> {}
	//
	template <class Derived>
	class System : public BaseSystem {
	public:

		// To allow destruction
		virtual ~System() {}

		// Returns the System type id
		static SystemFamily family() {
			static SystemFamily system_family = m_family++;
			return system_family;
		}
	};

	// Manages a bunch of systems
	class SystemManager final {
	public:

		// Creates a new System manager
		SystemManager();

		// Adds a new managed system
		// example:
		// SystemManager sys_mgr(entity_manager);
		// sys_mgr.add<MovementSystem>();
		// 
		template <class S, class... Args>
		void add(Args&& ...args);

		// Removes a managed system if any
		// example:
		// SystemManager sys_mgr(entity_manager);
		// sys_mgr.remove<MovementSystem>();
		//
		template <class S>
		void remove();

		// Checks if the manager has a system S
		// example:
		// SystemManager sm(em);
		// sm.add<MovementSystem>();
		// bool has = sm.has<MovementSystem>();
		// 
		template <class S>
		bool has();

		// Retrieves the system of type S if any
		// example:
		// SystemManager sm(em);
		// sm.add<MovementSystem>();
		// std::shared_ptr<MovementSystem> ms = sm.get<MovementSystem>();
		// if (ms) {}
		//
		template <class S>
		std::shared_ptr<S> get();

		// Updates all systems
		void update(EntityManager& em, double dt);

	private:
		std::unordered_map<SystemFamily, std::shared_ptr<BaseSystem>> systems;
	};

	template<class S, class ...Args>
	inline void SystemManager::add(Args && ...args)
	{
		assert(!has<S>() && "The system manager already manages a system of type S.");
		systems.insert(std::make_pair(S::system_id(), std::make_shared<S>(std::forward<Args>(args)...)));
	}

	template<class S>
	inline void SystemManager::remove()
	{
		assert(has<S>() && "The system manager does not manage a system of type S.");
		systems.erase(S::system_id());
	}

	template<class S>
	inline bool SystemManager::has()
	{
		return systems.find(S::system_id()) != systems.end();
	}

	template<class S>
	inline std::shared_ptr<S> SystemManager::get()
	{
		return systems.find(S::system_id()) != systems.end() 
			   ? std::static_pointer_cast<S>(systems.at(S::system_id())) 
			   : std::shared_ptr<S>(nullptr);
	}
}