namespace rift {
	namespace internal {

		template<class C>
		inline void Pool<C>::insert(std::uint32_t index, const BaseComponent & component)
		{
			if (!contains(index))
				expand(index);
			components[index] = static_cast<const C&>(component);
		}

		template<class C>
		inline void Pool<C>::replace(std::uint32_t index, const BaseComponent & component)
		{
			assert(contains(index));
			components[index] = static_cast<const C&>(component);
		}

		template<class C>
		inline BaseComponent & Pool<C>::at(std::uint32_t index)
		{
			assert(contains(index));
			return components[index];
		}

		template<class C>
		inline bool Pool<C>::contains(std::uint32_t index)
		{
			return index < components.size();
		}

		template<class C>
		inline void Pool<C>::expand(std::uint32_t n)
		{
			components.resize(n + 1);
		}

	}
}
