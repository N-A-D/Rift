#pragma once

#include <vector>
#include <cassert>
#include <type_traits>
#include "../component.h"

namespace rift {
	namespace impl {

		// The BasePool class
		// Provides the interface that all component caches must implement
		struct BasePool {
			virtual ~BasePool() = default;
			virtual void insert(std::size_t index, const BaseComponent& cmp) = 0;
			virtual BaseComponent& at(std::size_t index) = 0;
		};
		
		// The Pool class
		// Provides a very simple storage medium for a single component type
		// Note:
		// - Potentially wasteful with memory if many entities do not own a component
		//   in the cache
		template <class C>
		class Pool final : public BasePool {
			static_assert(std::is_base_of_v<BaseComponent, C>
				, "The component type does not inherit from rift::Component");
		public:

			// Insert a component at the given index
			inline void insert(std::size_t index, const BaseComponent& cmp) override;

			// Fetch the component at the given index
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
		inline void Pool<C>::insert(std::size_t index, const BaseComponent & cmp)
		{
			if (!has_space_for(index))
				expand(index);
			components[index] = static_cast<const C&>(cmp);
		}

		template<class C>
		inline BaseComponent & Pool<C>::at(std::size_t index)
		{
			assert(index < components.size());
			return components[index];
		}

		template<class C>
		inline bool Pool<C>::has_space_for(std::size_t index)
		{
			return index < components.size();
		}

		template<class C>
		inline void Pool<C>::expand(std::size_t n)
		{
			components.resize(n + 1);
		}

	}
}
