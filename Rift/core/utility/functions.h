#pragma once
#ifndef _RIFT_UTIL_FUNCTIONS_
#define _RIFT_UTIL_FUNCTIONS_
#include "../config.h"

namespace rift {

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
#define _RIFT_UTIL_FUNCTIONS_
#endif // !_RIFT_UTIL_FUNCTIONS_