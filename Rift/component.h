#pragma once
#ifndef _RIFT_COMPONENT_
#define _RIFT_COMPONENT_
#include <assert.h>
#include "config.h"

namespace rift {

	using ComponentFamily = std::size_t;

	// The BaseComponent class
	// Note: this class should not but subclassed directly as components need to be registered... See the Component class.
	class BaseComponent {
	public:
		virtual ~BaseComponent();
	protected:
		static ComponentFamily m_family;
	};

	// The Component class
	// Classes that are meant to be components must inherit from this class for registration as a 'component'
	// Note: 
	// - Derived classes must implement a default constructor
	// - Derived classes must implement a constructor that initializes all of its member variables
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
			assert(m_family < config::MAX_COMPONENT_TYPES && "The maximum number of components has been reached!");
			static ComponentFamily component_family = m_family++;
			return component_family;
		}
	};
}

#endif // !_RIFT_COMPONENT_