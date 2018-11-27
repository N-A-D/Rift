#pragma once

#include <vector>
#include <cassert>
#include "rift_traits.h"
#include "../component.h"

namespace rift {
	namespace impl {

		struct BaseCache {
			virtual ~BaseCache() = default;
			
			virtual void insert(std::size_t n, const BaseComponent& cmp) = 0;
			virtual BaseComponent& at(std::size_t n) = 0;
			virtual void expand(std::size_t n) = 0;

		};
		
		template <class C>
		class Cache : public BaseCache {
			static_assert(std::is_base_of_v<BaseComponent, C>
				, "The component type does not inherit from rift::Component");
		public:

			// Inserts a component into the cache
			void insert(std::size_t n, const BaseComponent& cmp) override;

			// Fetches the component at index n from cache
			BaseComponent& at(std::size_t n) override;

			// Expands the size of the cache to n
			void expand(std::size_t n) override;

		private:
			std::vector<C> components;
		};

		template<class C>
		inline void Cache<C>::insert(std::size_t n, const BaseComponent & cmp)
		{
			if (n >= components.size())
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
		inline void Cache<C>::expand(std::size_t n)
		{
			components.resize(n + 1);
		}

	}
}
