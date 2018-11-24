#pragma once

#include <vector>
#include <cassert>
#include <type_traits>

namespace rift {
	namespace impl {

		// The BaseCache class
		// An interface that specifies what 'cache' objects should be able to do
		class BaseCache {
		public:

			BaseCache() = default;
			virtual ~BaseCache() = default;

			virtual bool empty() const noexcept = 0;
			virtual void clear() noexcept = 0;
			virtual std::size_t size() const noexcept = 0;
			virtual std::size_t capacity() const noexcept = 0;
			virtual bool contains(std::size_t index) const noexcept = 0;
			virtual void insert(std::size_t index, void* object) = 0;
			virtual void replace(std::size_t index, void* object) = 0;
			virtual void erase(std::size_t index) = 0;
			virtual void* get(std::size_t index) = 0;

		};

		// The Cache class
		// Provides contiguous storage for objects of a single type
		// Note:
		// - Caches cannot be sorted
		// - Caches are read only when iterating.
		// - The class is implemented similar to a SparseSet
		//   but unlike a SparseSet, clearing the cache takes 
		//   O(n) time vs the O(1) time to clear a SparseSet
		template <class T>
		class Cache : public BaseCache {
		public:

			// InputIterator type
			template <class Type>
			class Iterator final {
			public:
				using value_type        = Type;
				using pointer           = value_type*;
				using reference         = value_type&;
				using differece_type    = std::ptrdiff_t;
				using iterator_category = std::input_iterator_tag;
				
				Iterator() = default;
				Iterator(const Iterator&) = default;
				Iterator& operator=(const Iterator&) = default;
				explicit Iterator(pointer iter) noexcept : iter(iter) {}

				reference operator*() const noexcept {
					assert(iter && "Invalid iterator dereference!");
					return *iter;
				}

				pointer operator->() const noexcept {
					assert(iter && "Invalid iterator pointer to member!");
					return iter;
				}

				Iterator& operator++() noexcept {
					assert(iter && "Invalid iterator increment!");
					++iter;
					return *this;
				}

				Iterator operator++(int) noexcept {
					assert(iter && "Invalid iterator increment!");
					auto tmp(iter);
					++iter;
					return tmp;
				}

				bool operator==(const Iterator& other) const noexcept { 
					return iter == other.iter; 
				}

				bool operator!=(const Iterator& other) const noexcept { 
					return !(*this == other); 
				}

			private:
				pointer iter = nullptr;
			};

			using size_type       = std::size_t;
			using value_type      = T;
			using iterator        = Iterator<value_type>;
			using const_iterator  = Iterator<const value_type>;

			Cache()          = default;
			virtual ~Cache() = default;

			iterator       begin()       noexcept { return iterator(instances.data()); }
			iterator       end()         noexcept { return iterator(instances.data() + instances.size()); }
			const_iterator begin() const noexcept { return const_iterator(instances.data()); }
			const_iterator end()   const noexcept { return const_iterator(instances.data() + instances.size()); }

			bool empty()         const noexcept override { return instances.empty(); }
			void clear()               noexcept override { instances.clear(); reverse.clear(); }
			size_type size()     const noexcept override { return instances.size(); }
			size_type capacity() const noexcept override { return forward.capacity(); }

			// Check if there exists an object at the given index
			bool contains(size_type index) const noexcept override;

			// Insert an object at the given index
			// Notes:
			// - An assertion is made that there does not exist an object at the given index.
			//   As a result, multiple insertions at the same index is not permitted
			// - If the size of the cache is smaller than the given index, which also means
			//   there is no object assigned to the index, the cache will expand to fit the index
			void insert(size_type index, void* object) override;

			// Replace an object at the given index
			// Note:
			// - An assertion is made that there already exists an object at the given index
			void replace(size_type index, void* object) override;

			// Erase an object at the given index
			// Note:
			// - An assertion is made that there exists an object at the given index.
			//   As a result, multiple removals at the same index are not permitted
			void erase(size_type index) override;

			// Return a generic pointer to the object at the given index
			// Note:
			// - An assertion is made that there exists an object at the given index
			void* get(size_type index) override;

		private:

			// The collection of object instances
			std::vector<T> instances;

			// Collection of indexes to instance objects
			std::vector<size_type> forward;

			// Collection of active indexes in forward
			std::vector<size_type> reverse;
		};

		template<class T>
		inline bool Cache<T>::contains(size_type index) const noexcept
		{
			if (index >= forward.size())
				return false;
			if (forward[index] < reverse.size() && reverse[forward[index]] == index)
				return true;
			return false;
		}

		template<class T>
		inline void Cache<T>::insert(size_type index, void * object)
		{
			assert(!contains(index));
			if (index >= forward.size())
				forward.resize(index + 1);
			forward[index] = reverse.size();
			instances.push_back(*(static_cast<T*>(object)));
			reverse.push_back(index);
		}

		template<class T>
		inline void Cache<T>::replace(size_type index, void * object)
		{
			assert(contains(index));
			instances[forward[index]] = *(static_cast<value_type*>(object));
		}

		template<class T>
		inline void Cache<T>::erase(size_type index)
		{
			assert(contains(index));
			instances[forward[index]] = instances.back();
			reverse[forward[index]] = reverse.back();
			forward[reverse.back()] = forward[index];
			instances.pop_back();
			reverse.pop_back();
		}

		template<class T>
		inline void * Cache<T>::get(size_type index)
		{
			assert(contains(index));
			return &instances[forward[index]];
		}
	}
}
