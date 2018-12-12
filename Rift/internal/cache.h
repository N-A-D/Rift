#pragma once

#include <vector>
#include <cassert>
#include <type_traits>
#include "../component.h"

namespace rift {
	namespace impl {

		// The BaseCache class
		// Provides the interface that all component caches must implement
		struct BaseCache {
			virtual ~BaseCache() = default;
		
			virtual void insert(std::size_t index, const BaseComponent& cmp) = 0;
			virtual BaseComponent& at(std::size_t index) = 0;

		};
		
		// The Cache class
		// Provides very simple storage medium for a single component type
		// Note:
		// - Admittedly this class has the potential to be very wasteful in terms 
		//   of memory usage as not all entities will have the every component type.
		//   However, it is very simple and requires next to no maintenance.
		template <class C>
		class Cache : public BaseCache {
			static_assert(std::is_base_of_v<BaseComponent, C>
				, "The component type does not inherit from rift::Component");
		public:

			// Inserts a component into the cache
			inline void insert(std::size_t index, const BaseComponent& cmp) override;

			// Fetches the component at index n from cache
			inline BaseComponent& at(std::size_t index) override;

		private:

			// Checks if the index is within the cache size
			inline bool has_space_for(std::size_t index);

			// Expands the size of the cache to fix n components
			inline void expand(std::size_t n);

			// The block of components
			std::vector<C> components;
		};

		template<class C>
		inline void Cache<C>::insert(std::size_t index, const BaseComponent & cmp)
		{
			if (!has_space_for(index))
				expand(index);
			components[index] = static_cast<const C&>(cmp);
		}

		template<class C>
		inline BaseComponent & Cache<C>::at(std::size_t index)
		{
			assert(index < components.size());
			return components[index];
		}

		template<class C>
		inline bool Cache<C>::has_space_for(std::size_t index)
		{
			return index < components.size();
		}

		template<class C>
		inline void Cache<C>::expand(std::size_t n)
		{
			components.resize(n + 1);
		}

	}
}
