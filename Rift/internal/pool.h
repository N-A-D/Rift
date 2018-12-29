#pragma once

#include <vector>
#include <cassert>
#include <cstdint>
#include <type_traits>
#include "../component.h"

namespace rift {
	namespace internal {

		// The BasePool class
		// Provides the interface for the Pool class below.
		struct BasePool {
			// operations:
			virtual void insert(std::uint32_t index, const BaseComponent& component) = 0;
			virtual void replace(std::uint32_t index, const BaseComponent& component) = 0;
			virtual BaseComponent& at(std::uint32_t index) = 0;
		};
		
		// The Pool class
		// A very simple storage medium for a single component type.
		// Note:
		// - Has the potential to waste a significant amount of memory if there are many
		//   entities that do not own a component in the pool. 
		template <class C>
		class Pool final : public BasePool {
		public:

			// Inserts a new component into the pool.
			// Note:
			// - Expands the pool size to accommodate an index greater than the current size.
			void insert(std::uint32_t index, const BaseComponent& component) override;

			// Replaces an existing component in the pool.
			// Note:
			// - Asserts the index fits within the size of the pool.
			void replace(std::uint32_t index, const BaseComponent& component) override;

			// Returns the component at index in the pool.
			// Note:
			// - Asserts the index fits within the size of the pool.
			BaseComponent& at(std::uint32_t index) override;

		private:

			// Checks if an index is within the pool size
			bool contains(std::uint32_t index);

			// Expands the size of the pool to fit up to n components.
			void expand(std::uint32_t n);

			// The block of components
			std::vector<C> components;
		};

	} // namespace internal
} // namespace rift
#include "pool.inl"
