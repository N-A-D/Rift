#pragma once

#include <assert.h>
#include "details/config.h"

namespace rift {
	
	using ComponentFamily = std::size_t;

	// The BaseComponent class
	// Note: this class should not but subclassed directly as components need to be registered
	class BaseComponent {
	public:
		virtual ~BaseComponent();
	protected:
		static ComponentFamily m_family;
	};
	
	// The Component class
	// Classes that are meant to be components must inherit from this class for registration as a 'component'
	// Note: 
	// Derived classes must implement a default constructor
	// as well as  a constructor that initializing its member variables
	// example:
	// struct PositionComponent : public Component<PositionComponent> {
	//     PositionComponent() : x(x), y(y) {} // Default ctor
	//     PositionComponent(double x, double y) : x(x), y(y) {}
	//     double x, y;
	// };
	template <class Derived>
	class Component : public BaseComponent {
	public:
		virtual ~Component() {}
		// Returns the Component type id
		static ComponentFamily family() noexcept {
			assert(m_family <= config::MAX_COMPONENT_TYPES && "The maximum number of components has been reached!");
			static ComponentFamily component_family = m_family++;
			return component_family;
		}
	};

	namespace util {

		// Given a template parameter pack of Component types, this function returns the combined ComponentMask
		// example: ComponentMask mask = mask_for<Position, Velocity, Direction>();
		template <class ...Components>
		ComponentMask mask_for() noexcept {
			ComponentMask mask = 0;
			[&mask](...) {}((mask.set(Components::family()))...);
			return mask;
		}
	}
}
