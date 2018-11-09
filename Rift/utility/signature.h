#pragma once

#include "../config/config.h"

namespace rift {
	namespace util {
		// Given a template parameter pack of Component types, this function returns the ComponentMask for those types
		// example: ComponentMask mask = signature_for<Position, Direction>();
		// Note:
		// - Ordering of the types does not matter, the function will still return the same component mask. That is, 
		//   signature_for<Position, Direction>() == signature_for<Direction, Position>()
		template <class ...Components>
		ComponentMask signature_for() noexcept {
			ComponentMask mask;
			[](...) {}((mask.set(Components::family()))...);
			return mask;
		}
	}
}
