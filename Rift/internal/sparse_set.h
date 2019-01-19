#pragma once

#include <vector>
#include <cstddef>
#include <stdexcept>

namespace rift {
	namespace internal {

		// The SparseSet class
		// Unordered set of nonnegative integers.
		class SparseSet final {
		public:

			template <class T>
			class Iterator final {
			public:

				using value_type        = T;
				using reference         = value_type & ;
				using pointer           = value_type * ;
				using difference_type   = std::ptrdiff_t;
				using iterator_category = std::forward_iterator_tag;

				Iterator() = default;
				Iterator(const Iterator&) = default;
				Iterator& operator=(const Iterator&) = default;
				explicit Iterator(pointer pos) noexcept : it(pos) {}

				reference operator*() const { 
					if (!it)
						throw std::runtime_error("Invalid iterator dereference!");
					return *it; 
				}

				pointer operator->() const { 
					if (!it)
						throw std::runtime_error("Invalid iterator to member!");
					return it; 
				}

				Iterator& operator++() {
					if (!it)
						throw std::runtime_error("Invalid iterator increment!");
					++it;
					return *this;
				}

				Iterator operator++(int) {
					if (!it)
						throw std::runtime_error("Invalid iterator increment!");
					Iterator<value_type> tmp(it);
					++it;
					return tmp;
				}

				bool operator==(const Iterator& other) const noexcept {
					return it == other.it;
				}

				bool operator!=(const Iterator& other) const noexcept {
					return !(*this == other);
				}

			private:
				pointer it = nullptr;
			};

			using value_type      = std::uint32_t;
			using const_pointer   = const value_type * ;
			using size_type       = std::vector<value_type>::size_type;
			using const_iterator  = Iterator<const value_type>;

			SparseSet() = default;
			SparseSet(const SparseSet&) = default;
			SparseSet& operator=(const SparseSet&) = default;

			// Returns a const_iterator to the first integer contained in the container.
			const_iterator begin() const noexcept;

			// Returns a const_iterator to the end of the container.
			const_iterator end() const noexcept;

			// Returns true if the container contains no integers.
			bool empty() const noexcept;

			// Returns the number of integers contained in the container.
			size_type size() const noexcept;

			// Returns the largest possible size of the container.
			size_type max_size() const noexcept;

			// Returns the number of integers for which memory has been allocated. capacity() 
			// is always greater than or equal to size().
			size_type capacity() const noexcept;

			// Inserts v into the container if and only if it does not already exist in the container.
			void insert(value_type v);

			// Inserts each integer from the range [begin, end) if and only if the integer does
			// not already exist in the container.
			template <class InIt>
			void insert(InIt begin, InIt end);

			// Erases an integer if and only if the integer is contained in the container.
			void erase(value_type v);

			// Erases each integer from the range [begin, end) if and only if the integer is in the container.
			template <class InIt>
			void erase(InIt begin, InIt end);

			// Removes every integer in the container.
			void clear() noexcept;

			// Returns a const_pointer to the underlying array serving as integer storage.
			const_pointer data() const noexcept;

			// Returns true if the container contains the integer.
			bool contains(value_type v) const noexcept;

			// Returns true if every integer in the range from [begin, end) is contained in the container.
			template <class InIt>
			bool contains(InIt begin, InIt end) const noexcept;

		private:
			
			// The number of nonnegative integers in the set
			value_type n = 0;

			// Collection of nonnegative integers in the set (indices of the sparse vector)
			std::vector<value_type> dense;

			// Collection indices into the dense vector
			std::vector<value_type> sparse;
		};

	} // namespace internal
} // namespace rift
#include "sparse_set.inl"
