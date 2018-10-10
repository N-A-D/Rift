#pragma once

#include <vector>

namespace rift {

	// The base object pool
	// Used only for pointers
	class BasePool {
	public:
		virtual ~BasePool() {}
	};

	// The Pool class
	// Manages a contiguous block of objects
	template <class Object>
	class Pool : public BasePool {
	public:

		using value_type = Object;
		using reference = Object &;
		using const_reference = const Object&;
		using size_type = typename std::vector<Object>::size_type;
		using iterator = typename std::vector<Object>::iterator;
		using const_iterator = typename std::vector<Object>::const_iterator;

		Pool() {}
		Pool(size_type size) : objects(size, Object()) {}
		virtual ~Pool() {}

		iterator begin() { return objects.begin(); }
		iterator end() { return objects.end(); }
		const_iterator begin() const { return objects.begin(); }
		const_iterator end() const { return objects.end(); }

		reference at(size_type index) { return objects.at(index); }
		const_reference at(size_type index) const { return objects.at(index); }
		
		void clear() noexcept { objects.clear(); }
		void allocate(size_type count) noexcept { objects.resize(objects.size() + count); }
		size_type size() const noexcept  { return objects.size(); }
		size_type capacity() const noexcept { return objects.capacity(); }

	private:
		std::vector<Object> objects;
	};
	
	template <class...>
	class Cache;

	

}