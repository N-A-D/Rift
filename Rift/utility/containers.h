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

		iterator begin() { return components.begin(); }
		iterator end() { return components.end(); }
		const_iterator begin() const { return components.begin(); }
		const_iterator end() const { return components.end(); }

		inline reference at(size_type index) {
			//assert(forward.at(index) < components.size());
			//return components.at(forward.at(index));
			return *(static_cast<Component *>(get(index)));
		}

		inline const_reference at(size_type index) const {
			//assert(forward.at(index) < components.size());
			//return components.at(forward.at(index));
			return *(static_cast<Component *>(get(index)));
		}

		inline void insert(size_type index, void* object) override {
			//assert(forward.at(index) < components.size());
			if (forward.size() <= index)
				forward.resize(index + 1);
			auto size = components.size();
			forward.push_back(size);
			backward.push_back(index);
			components.push_back(*(static_cast<Component *>(object)));
		}

		inline void remove(size_type index) override {
			assert(forward.at(index) < components.size());
			auto sub = components.back();
			components.at(forward.at(index)) = sub;
			forward.at(backward.back()) = forward.at(index);
			components.pop_back();
			backward.pop_back();
		}

		void clear() noexcept override {
			components.clear();
			backward.clear();
		}

		size_type size() const noexcept override {
			return components.size();
		}

		size_type capacity() const noexcept override {
			return forward.size();
		}

	private:
		
		inline void* get(std::size_t index) override {
			assert(forward.at(index) < components.size());
			return (void *)(&components.at(forward.at(index)));
		}
		
		// The collection of components
		std::vector<Component> components;
		// Collection of indexes to components
		std::vector<std::size_t> forward;
		// Collection of indexes to forward
		// This is used to remap indexes pointing
		// to the last component to its new location
		std::vector<std::size_t> backward;

		
	};
	
	template <typename...>
	class Cache;

	template <class Entity>
	class Cache<Entity> {
	public:

		using iterator = typename std::vector<Entity>::iterator;
		using const_iterator = typename std::vector<Entity>::const_iterator;

		Cache<Entity>() = default;

		iterator begin() { return data.begin(); }
		iterator end() { return data.end(); }
		const_iterator begin() const { return data.begin(); }
		const_iterator end() const { return data.end(); }

		inline bool has(const Entity& e) const {
			auto idx = e.id().index();
			if (idx >= indexes.size())
				return false;
			if (indexes.at(idx) < data.size() && 
				data.at(indexes.at(idx)) == e)
				return true;
			return false;
		}

		inline void insert(const Entity& e) {
			assert(!has(e));
			auto idx = e.id().index();
			if (idx >= indexes.size())
				indexes.resize(idx + 1);
			auto size = data.size();
			data.push_back(e);
			indexes.at(idx) = size;
		}

		inline void remove(const Entity& e) {
			assert(has(e));
			auto idx = e.id().index();
			auto sub = data.back();
			data.at(indexes.at(idx)) = sub;
			indexes.at(sub.id().index()) = indexes.at(idx);
			data.pop_back();
		}

	private:

		std::vector<Entity> data;
		std::vector<std::size_t> indexes;
	};

}