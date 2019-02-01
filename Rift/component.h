#pragma once

#include <cassert>
#include "config/config.h"

namespace rift {

	// The BaseComponent class
	// Note: 
	// - This class should not but subclassed directly as components need to be registered. 
	//   See the Component class below.
	class BaseComponent {
	protected:
		using Family = std::size_t;
		// Used internally for generating component type ids.
		inline static Family family_counter = 0;
	};

	class EntityManager;

	// The Component class
	// Classes that are meant to be components must inherit from this class for registration as a component.
	// Note: 
	// - Derived classes must implement a default constructor.
	// - Derived classes must implement a constructor that initializes all of its member variables.
	// - Derived classes must be copy constructible/assignable.
	// Example:
	// struct Position : public Component<Position> {
	//     Position() : x(0.0), y(0.0) {} // Default ctor
	//     Position(double x, double y) : x(x), y(y) {}
	//     double x, y;
	// };
	template <class Derived>
	class Component : public BaseComponent {
		friend class EntityManager;
		// Used internally for component registration.
		static Family family() noexcept {
			static Family family_id = family_counter++;
			assert(family_id < MAX_COMPONENTS && "The maximum number of components has been reached!");
			return family_id;
		}
	};

} // namespace rift
