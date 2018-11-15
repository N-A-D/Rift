#pragma once

#include "rift_traits.h"
#include "../component.h"

namespace rift {
	namespace util {
		// Given a template parameter pack of Component types, this function returns the ComponentMask for those types
		// example: ComponentMask mask = signature_for<Position, Direction>();
		// Note:
		// - Ordering of the types does not matter, the function will still return the same component mask. That is, if
		//   classes A and B are any two subclasses of rift::Component, signature_for<A, B>() == signature_for<B, A>()
		template <class ...Components>
		ComponentMask signature_for() noexcept {
			static_assert(static_all_of<std::is_base_of<BaseComponent, Components>::value...>::value, 
				"Not all components inherit from rift::Component!");
			ComponentMask mask;
			[](...) {}((mask.set(Components::family()))...);
			return mask;
		}
	}
}
