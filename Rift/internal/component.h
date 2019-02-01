#pragma once

#include <cassert>
#include "config/config.h"

namespace rift {

	class EntityManager;

	namespace internal {

		// The BaseComponent class
		class BaseComponent {
		protected:
			using Family = std::size_t;
			// Used internally for generating component type ids.
			inline static Family family_counter = 0;
		};

		// The Component class
		// Used internally for registering types as components.
		template <class Derived>
		class Component : public BaseComponent {
			friend class EntityManager;
			static Family family() noexcept {
				static Family family_id = family_counter++;
				assert(family_id < MAX_COMPONENTS && "The maximum number of components has been reached!");
				return family_id;
			}
		};
	}

} // namespace rift
