#pragma once

#include <cassert>
#include "config/config.h"

namespace rift {

	using ComponentFamily = std::size_t;

	// The BaseComponent class
	// Note: 
	// - This class should not but subclassed directly as components need to be registered. 
	//   See the Component class below.
	class BaseComponent {
	public:
		virtual ~BaseComponent() = default;
	protected:
		static ComponentFamily generate_family() noexcept {
			static ComponentFamily family_counter = 0;
			return family_counter++;
		}
	};

	class EntityManager;

	// The Component class
	// Classes that are meant to be components must inherit from this class for registration as a component.
	// Note: 
	// - Derived classes must implement a default constructor.
	// - Derived classes must implement a constructor that initializes all of its member variables.
	// Example:
	// struct Position : public Component<Position> {
	//     Position() : x(0.0), y(0.0) {} // Default ctor
	//     Position(double x, double y) : x(x), y(y) {}
	//     double x, y;
	// };
	template <class Derived>
	class Component : public BaseComponent {
	public:
		virtual ~Component() = default;
	private:
		friend class EntityManager;
		// Returns the Component class id.
		static ComponentFamily family() noexcept {
			static ComponentFamily family_id = generate_family();
			assert(family_id < config::MAX_COMPONENT_TYPES && "The maximum number of components has been reached!");
			return family_id;
		}
	};

} // namespace rift
