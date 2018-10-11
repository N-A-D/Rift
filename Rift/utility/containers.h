#pragma once

#include <vector>

namespace rift {

	// The Base Component Pool
	class BasePool {
	public:
		
		using size_type = std::size_t;

		virtual ~BasePool() {}
		// Return a pointer to the component mapped to index
		// If the index is not valid, then returns nullptr
		virtual void* get(std::size_t index) = 0;
		// Inserts a component at the given index
		// If the pool is at capacity, then the pool will expand 
		virtual void insert(size_type index, void* object) = 0;
		// Removes the component at the given index
		virtual void remove(size_type index) = 0;
		// Clears the component pool
		virtual void clear() noexcept = 0;
		// Returns the number of components in the pool
		virtual size_type size() const noexcept = 0;
		// Returns the number of components that can fit into the pool currently
		virtual size_type capacity() const noexcept = 0;
	};

	// The Component Pool class
	// Manages a contiguous block of components
	template <class Component>
	class Pool : public BasePool {
	public:

		using value_type = Component;
		using reference = Component &;
		using const_reference = const Component&;
		using iterator = typename std::vector<Component>::iterator;
		using const_iterator = typename std::vector<Component>::const_iterator;

		Pool() {}
		virtual ~Pool() {}

		/// Iterators to the packed set of components
		iterator begin() { return components.begin(); }
		iterator end() { return components.end(); }
		const_iterator begin() const { return components.begin(); }
		const_iterator end() const { return components.end(); }
		
		// Returns a reference to the component at a given index
		inline reference at(size_type index) { return *(static_cast<Component *>(get(index))); }

		// Returns a const reference to the component at a given index
		inline const_reference at(size_type index) const { return *(static_cast<Component *>(get(index))); }

		// Returns a pointer to the memory of the Component
		// Note:
		// - Do not delete the pointer!
		inline void* get(std::size_t index) override {
			assert(forward.at(index) < components.size());
			return /*(void *)*/(&components.at(forward.at(index)));
		}


		// Inserts a component into the Pool overwritting any pre-existing component at the given index
		inline void insert(size_type index, void* object) override {
			// If the pool isn't large enough to fit a component at the given index, 
			// expand the pool so that it is.
			if (forward.size() <= index)
				forward.resize(index + 1);

			// Store the location TO the new component
			forward.at(index) = components.size();

			// Store the location FROM the component to the location of the index TO the component
			backward.push_back(index);

			// Add the component to the set
			components.push_back(*(static_cast<Component *>(object)));
		}

		// Remove any component at the given index
		// Note:
		// - While is is possible to continuously delete a component at a single index, an Entity can only ever
		//   delete a component of a given type from itself once. It if wishes to delete the component again, it
		//   must first re-add the component then remove it. Since there are external control mechanisms governing
		//   the ability to remove components, there is no need to worry about repeated deletion at a given index
		inline void remove(size_type index) override {
			assert(index < forward.size() && 
				   forward.at(index) < components.size() && 
				   "There is no component at the given index!");

			// Swap the component to be destroyed with the component at the back of the set
			components.at(forward.at(index)) = components.back();

			// Update the mapping from TO the last component's location
			forward.at(backward.back()) = forward.at(index);

			// Update the mapping FROM the last component's location TO the location of the mapping in the reverse direction
			backward.at(forward.at(index)) = backward.back();

			// Remove the last component as it has been moved
			components.pop_back();
			backward.pop_back();
		}

		// Clears the pool of any components
		void clear() noexcept override {
			components.clear();
			backward.clear();
		}

		// Return the number of components that are currently in the pool
		size_type size() const noexcept override { return components.size(); }

		// Returns the current number of Components that the Pool can support
		size_type capacity() const noexcept override { return forward.size(); }

	private:

		// The collection of components
		std::vector<Component> components;
		// Collection of indexes to components
		std::vector<size_type> forward;
		// Collectino of indexes from components
		std::vector<size_type> backward;
	};

	// The ResultSet class
	// Is, and can only be, used for storing rift::Entity's in compact form for use in entities_with<Components...> queries
	template <class Entity>
	class ResultSet {
	public:

		using size_type = std::size_t;
		using iterator = typename std::vector<Entity>::iterator;
		using const_iterator = typename std::vector<Entity>::const_iterator;

		ResultSet() = default;

		// Iterators into the set

		iterator begin() { return data.begin(); }
		iterator end() { return data.end(); }
		const_iterator begin() const { return data.begin(); }
		const_iterator end() const { return data.end(); }

		// Checks if the ResultSet already contains a given entity
		inline bool has(const Entity& e) const {
			auto idx = e.id().index();
			if (idx >= forward.size())
				return false;
			if (forward.at(idx) < data.size() && 
				data.at(forward.at(idx)) == e)
				return true;
			return false;
		}

		// Inserts an entity into the result set 
		// Notes:
		// - An assertion is made that the entity does not exist in the set
		// - If unsure as to whether or not the entity exists, first check
		//   for the entity using the has() function
		// - If the entity's index is greater than the size of the set, the 
		//   set will expand to accommodate the entity
		inline void insert(const Entity& e) {
			assert(!has(e));
			auto idx = e.id().index();
			if (idx >= forward.size())
				forward.resize(idx + 1);
			forward.at(idx) = data.size();
			data.push_back(e);
		}

		// Removes an entity from the result set
		// Notes:
		// - An assertion is made that the entity exists within the set
		// - If unsure as to whether or not the entity exists, first check
		//   for the entity using the has() function
		inline void remove(const Entity& e) {
			assert(has(e));
			auto idx = e.id().index();
			auto sub = data.back();
			data.at(forward.at(idx)) = sub;
			forward.at(sub.id().index()) = forward.at(idx);
			data.pop_back();
		}

		// Returns the number of entity's in the set
		size_type size() const noexcept { return data.size(); }

		// Returns the capacity of the set. I.e., how many entities it can possibly hold
		size_type capacity() const noexcept { return forward.size(); }

	private:

		std::vector<Entity> data;
		std::vector<std::size_t> forward;
	};

}