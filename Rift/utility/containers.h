#pragma once
#ifndef _RIFT_UTIL_CONTAINERS_
#define _RIFT_UTIL_CONTAINERS_
#include <vector>

namespace rift {

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
		virtual bool test(std::size_t index) const noexcept = 0;
		virtual void insert(std::size_t index, void* object) = 0;
		virtual void erase(std::size_t index) = 0;
		virtual void* get(std::size_t index) = 0;

	};

	// The Cache class
	// Provides contiguous storage for objects of a single type
	// Note:
	// - Caches cannot be sorted
	// - Caches are read only when iterating.
	// - The class is implemented in a very similar way to a SparseSet
	//   *Clearing the cache takes O(n) time operator vs the O(1) time to clear a SparseSet
	template <class T>
	class Cache : public BaseCache {
	public:

		template <bool is_const>
		class Iterator final {
		public:

			using value_type = T;
			using difference_type = std::ptrdiff_t;
			using reference = typename std::conditional_t<is_const, const T&, T&>;
			using pointer = typename std::conditional_t<is_const, const T*, T*>;
			using iterator_category = std::input_iterator_tag;

			Iterator() = default;
			Iterator(const Iterator&) = default;
			Iterator& operator=(const Iterator&) = default;

			explicit Iterator(T* data)
				: current(data) {}

			template <bool _cond = is_const>
			std::enable_if_t<_cond, reference>
				operator*() const {
				return *current;
			}

			template <bool _cond = is_const>
			std::enable_if_t<!_cond, reference>
				operator*() const {
				return *current;
			}

			Iterator& operator++() {
				++current;
				return *this;
			}

			Iterator& operator++(int) {
				Iterator tmp = *this;
				++current;
				return tmp;
			}

			bool operator==(const Iterator& other) const {
				return current == other.current;
			}

			bool operator!=(const Iterator& other) const {
				return !(*this == other);
			}

		private:
			T* current;
		};

		using size_type = std::size_t;
		using value_type = T;
		using reference = T & ;
		using const_reference = const T&;
		using iterator = Iterator<false>;
		using const_iterator = Iterator<true>;

		Cache() = default;
		virtual ~Cache() = default;

		iterator begin() { return Iterator<false>(instances.data()); }
		iterator end() { return Iterator<false>(instances.data() + instances.size()); }
		const_iterator begin() const { return Iterator<true>(instances.data()); }
		const_iterator end() const { return Iterator<true>(instances.data() + instances.size()); }

		bool empty() const noexcept override { return reverse.empty(); }
		void clear() noexcept override { instances.clear(); reverse.clear(); }
		size_type size() const noexcept override { return reverse.size(); }
		size_type capacity() const noexcept override { return forward.size(); }

		// Check if there exists an object at the given index
		bool test(size_type index) const noexcept override;

		// Insert an object at the given index
		// Notes:
		// - An assertion is made that there does not exist an object at the given index.
		//   As a result, multiple insertions at the same index is not permitted
		// - If the size of the cache is smaller than the given index, which also means
		//   there is no object assigned to the index, the cache will expand to fit the index
		void insert(size_type index, void* object) override;

		// Remove an object at the given index
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
	inline bool Cache<T>::test(size_type index) const noexcept
	{
		if (index >= forward.size())
			return false;
		if (forward.at(index) < reverse.size() && reverse.at(forward.at(index)) == index)
			return true;
		return false;
	}

	template<class T>
	inline void Cache<T>::insert(size_type index, void * object)
	{
		assert(!test(index));
		if (index >= forward.size())
			forward.resize(index + 1);
		forward.at(index) = reverse.size();
		instances.push_back(*(static_cast<T *>(object)));
		reverse.push_back(index);
	}

	template<class T>
	inline void Cache<T>::erase(size_type index)
	{
		assert(test(index));
		instances.at(forward.at(index)) = instances.back();
		reverse.at(forward.at(index)) = reverse.back();
		forward.at(reverse.back()) = forward.at(index);
		instances.pop_back();
		reverse.pop_back();
	}

	template<class T>
	inline void * Cache<T>::get(size_type index)
	{
		assert(test(index));
		return &instances.at(forward.at(index));
	}

}

#endif // !_RIFT_UTIL_CONTAINERS_