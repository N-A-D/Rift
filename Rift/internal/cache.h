#pragma once

#include <vector>
#include <cassert>
#include <type_traits>
#include "../component.h"

namespace rift {
	namespace impl {

		struct BaseCache {
			virtual ~BaseCache() = default;
		
			virtual void insert(std::size_t n, const BaseComponent& cmp) = 0;
			virtual BaseComponent& at(std::size_t n) = 0;

		protected:

			virtual void expand(std::size_t n) = 0;

		};
		
		template <class C>
		class Cache : public BaseCache {
			static_assert(std::is_base_of_v<BaseComponent, C>
				, "The component type does not inherit from rift::Component");
		public:

			// Inserts a component into the cache
			inline void insert(std::size_t n, const BaseComponent& cmp) override;

			// Fetches the component at index n from cache
			inline BaseComponent& at(std::size_t n) override;

		private:

			// Checks if the index is within the cache size
			inline bool has_space_for(std::size_t index);

			// Expands the size of the cache to n
			inline void expand(std::size_t n) override;

			std::vector<C> components;
		};

		template<class C>
		inline void Cache<C>::insert(std::size_t n, const BaseComponent & cmp)
		{
			if (!has_space_for(n))
				expand(n);
			components[n] = static_cast<const C&>(cmp);
		}

		template<class C>
		inline BaseComponent & Cache<C>::at(std::size_t n)
		{
			assert(n < components.size());
			return components[n];
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
