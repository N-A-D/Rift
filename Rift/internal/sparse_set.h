#pragma once

#include <vector>
#include <cstddef>
#include <stdexcept>

namespace rift {
	namespace impl {

		// Sparse integer set for storing indices compactly
		class SparseSet final {
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
					auto tmp(it);
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

			using value_type      = std::size_t;
			using size_type       = std::vector<value_type>::size_type;
			using const_iterator  = Iterator<const value_type>;

			SparseSet() = default;
			SparseSet(const SparseSet&) = default;
			SparseSet& operator=(const SparseSet&) = default;

			// iterator:
			const_iterator begin() const noexcept;
			const_iterator end() const noexcept;

			// capacity:
			bool      empty() const noexcept;
			size_type size() const noexcept;
			size_type max_size() const noexcept;
			size_type capacity() const noexcept;

			// modifiers:
			void insert(value_type v);
			void erase(value_type v);
			void clear() noexcept;

			// operations:
			bool contains(value_type v) const noexcept;
			void sort();

		private:
			
			// The number of integers in the set
			size_type n = 0;

			// The compact collection of integers
			std::vector<value_type> dense;

			// The collection of integers present in the set
			std::vector<value_type> sparse;
		};

	}
}