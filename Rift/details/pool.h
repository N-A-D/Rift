#pragma once

#include <vector>

namespace rift {

	// The base object pool
	// Used only for pointers
	class BasePool {
	public:
		virtual ~BasePool() {}
		virtual std::size_t size() noexcept = 0;
		virtual void allocate(std::size_t slots) noexcept = 0;
	};

	// The Pool class
	// Manages a contiguous block of objects
	template <class T>
	class Pool : public BasePool {
	public:

		Pool() {}

		// Create a new Object pool is size objects preallocated
		Pool(std::size_t size)
			: objects(size, T()) {}

		// Allocates slots many objects
		void allocate(std::size_t slots) noexcept override {
			for (std::size_t i = 0; i < slots; i++)
				objects.push_back(T());
		}

		T& operator[](std::size_t index) {
			return objects[index];
		}

		const T& operator[](std::size_t index) const {
			return objects[index];
		}

		// Returns a reference to the object at index
		// Throws std::out_of_range if !(index < size())
		T& at(std::size_t index) {
			return objects.at(index);
		}

		// Returns a const reference to the object at index
		// Throws std::out_of_range if !(index < size())
		const T& at(std::size_t index) const {
			return objects.at(index);
		}
		
		// Return the number of allocated objects
		std::size_t size() noexcept override {
			return objects.size(); 
		}

	private:
		std::vector<T> objects;
	};


}