#pragma once

#include <vector>
#include <cassert>
#include <cstddef>
#include <type_traits>

namespace rift {
	namespace impl {

		// Sparse integer set for storing indices compactly
		class SparseSet {
		public:

			template <class T>
			class Iterator final {
			public:

				using value_type        = T;
				using reference         = value_type & ;
				using pointer           = value_type * ;
				using difference_type   = std::ptrdiff_t;
				using iterator_category = std::input_iterator_tag;

				Iterator() = default;
				Iterator(const Iterator&) = default;
				Iterator& operator=(const Iterator&) = default;
				explicit Iterator(pointer pos) : it(pos) {}

				reference operator*() const { 
					assert(it); return *it; 
				}

				pointer operator->() const { 
					assert(it); 
					return it; 
				}

				Iterator& operator++() {
					assert(it);
					++it;
					return *this;
				}

				Iterator operator++(int) {
					assert(it);
					auto tmp(it);
					return *this;
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

			using value_type      = std::size_t;
			using reference       = value_type&;
			using const_reference = const value_type&;
			using pointer         = value_type*;
			using size_type       = std::vector<value_type>::size_type;
			using iterator        = Iterator<value_type>;
			using const_iterator  = Iterator<const value_type>;

			SparseSet() = default;
			SparseSet(const SparseSet&) = default;
			SparseSet& operator=(const SparseSet&) = default;

			// capacity:
			bool empty() const noexcept;
			size_type size() const noexcept;
			size_type max_size() const noexcept;

			// iterators:
			iterator begin();
			iterator end();
			const_iterator begin() const;
			const_iterator end() const;

			// modifiers:
			void insert(value_type v);
			void erase(value_type v);
			void clear() noexcept;

			// operations:
			bool contains(value_type v) const noexcept;
			void sort();

		private:
			size_type n = 0;
			std::vector<value_type> dense;
			std::vector<value_type> sparse;
		};

	}
}