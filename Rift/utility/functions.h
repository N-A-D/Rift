#pragma once

#include "../config.h"

namespace rift {

	// Given a template parameter pack of Component types, this function returns the ComponentMask for those types
	// example: ComponentMask mask = signature_for<Position, Velocity, Direction>();
	template <class ...Components>
	ComponentMask signature_for() noexcept {
		ComponentMask mask = 0;
		[&mask](...) {}((mask.set(Components::family()))...);
		return mask;
	}

}