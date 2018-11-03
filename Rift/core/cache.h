#pragma once
#ifndef _RIFT_CACHE_
#define _RIFT_CACHE_
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
		virtual bool exists(std::size_t index) const noexcept = 0;
		virtual void insert(std::size_t index, void* object) = 0;
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
		
		using size_type = std::size_t;
		using value_type = T;
		using reference = T & ;
		using const_reference = const T&;
		using const_iterator = typename std::vector<T>::const_iterator;

		Cache() = default;
		virtual ~Cache() = default;
		
		// Caches are read only
		const_iterator begin() const { return instances.begin(); }
		const_iterator end() const { return instances.end(); }

		bool empty() const noexcept override { return reverse.empty(); }
		void clear() noexcept override { instances.clear(); reverse.clear(); }
		size_type size() const noexcept override { return reverse.size(); }
		size_type capacity() const noexcept override { return forward.size(); }

		// Check if there exists an object at the given index
		bool exists(size_type index) const noexcept override;

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
	inline bool Cache<T>::exists(size_type index) const noexcept
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
		assert(!exists(index));
		if (index >= forward.size())
			forward.resize(index + 1);
		forward[index] = reverse.size();
		instances.push_back(*(static_cast<T *>(object)));
		reverse.push_back(index);
	}

	template<class T>
	inline void Cache<T>::erase(size_type index)
	{
		assert(exists(index));
		instances[forward[index]] = instances.back();
		reverse[forward[index]] = reverse.back();
		forward[reverse.back()] = forward[index];
		instances.pop_back();
		reverse.pop_back();
	}

	template<class T>
	inline void * Cache<T>::get(size_type index)
	{
		assert(exists(index));
		return &instances[forward[index]];
	}

}

#endif // !_RIFT_CACHE_