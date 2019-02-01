namespace rift {
	namespace internal {

		template<class T>
		inline void Pool<T>::insert(std::uint32_t index, void* object)
		{
			if (!contains(index))
				accommodate(index);
			objects[index] = *(static_cast<T*>(object));
		}

		template<class T>
		inline void Pool<T>::replace(std::uint32_t index, void* object)
		{
			assert(contains(index));
			objects[index] = *(static_cast<T*>(object));
		}

		template<class T>
		inline void* Pool<T>::at(std::uint32_t index)
		{
			assert(contains(index));
			return &objects[index];
		}

		template<class T>
		inline bool Pool<T>::contains(std::uint32_t index)
		{
			return index < objects.size();
		}

		template<class T>
		inline void Pool<T>::accommodate(std::uint32_t index)
		{
			objects.resize(index + 1);
		}

	}
}
