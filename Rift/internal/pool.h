#pragma once

#include <vector>
#include <cassert>
#include <cstdint>
#include <type_traits>

namespace rift {
	namespace internal {

		// The BasePool class
		// Provides the interface for the Pool class below.
		struct BasePool {
			virtual ~BasePool() = default;
			// operations:
			virtual void insert(std::uint32_t index, void* object) = 0;
			virtual void replace(std::uint32_t index, void* object) = 0;
			virtual void* at(std::uint32_t index) = 0;
		};
		
		// The Pool class
		// A very simple storage medium for a single type.
		// Note:
		// - Has the potential to waste a significant amount of memory if there are many
		//   entities that do not own a component in the pool. 
		template <class T>
		class Pool final : public BasePool {
		public:
			static_assert(std::is_copy_constructible_v<T>, "The component type is not copy constructible!");
			static_assert(std::is_copy_assignable_v<T>, "The component type is not copy assignable!");

			// Inserts a new component into the pool.
			// Note:
			// - Expands the pool size to accommodate an index greater than the current size.
			void insert(std::uint32_t index, void* object) override;

			// Replaces an existing component in the pool.
			// Note:
			// - Asserts the index fits within the size of the pool.
			void replace(std::uint32_t index, void* object) override;

			// Returns the component at index in the pool.
			// Note:
			// - Asserts the index fits within the size of the pool.
			void* at(std::uint32_t index) override;

		private:

			// Checks if an index is within the pool size
			bool contains(std::uint32_t index);

			// Expands the size of the pool to include the index.
			void accommodate(std::uint32_t index);

			// Collection of objects.
			std::vector<T> objects;
		};

	} // namespace internal
} // namespace rift
#include "pool.inl"
