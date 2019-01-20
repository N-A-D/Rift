#include <cassert>
#include <algorithm>

namespace rift {
	namespace internal {

		inline bool SparseSet::empty() const noexcept
		{
			return n == 0;
		}

		inline SparseSet::size_type SparseSet::size() const noexcept
		{
			return n;
		}

		inline SparseSet::size_type SparseSet::max_size() const noexcept
		{
			return sparse.max_size();
		}

		inline SparseSet::size_type SparseSet::capacity() const noexcept
		{
			return sparse.capacity();
		}

		inline SparseSet::const_iterator SparseSet::begin() const noexcept
		{
			return const_iterator(data());
		}

		inline SparseSet::const_iterator SparseSet::end() const noexcept
		{
			return const_iterator(data() + size());
		}

		inline void SparseSet::insert(value_type v)
		{
			assert(!contains(v));
			if (sparse.size() <= v)
				sparse.resize(v + 1);
			if (dense.size() <= n)
				dense.resize(n + 1);
			sparse[v] = n;
			dense[n] = v;
			++n;
		}

		template <class InIt>
		inline void SparseSet::insert(InIt begin, InIt end)
		{
			for (InIt i = begin; i != end; ++i)
				insert(*i);
		}

		inline void SparseSet::erase(value_type v)
		{
			assert(contains(v));
			dense[sparse[v]] = dense[n - 1];
			sparse[dense[n - 1]] = sparse[v];
			--n;
		}

		template <class InIt>
		inline void SparseSet::erase(InIt begin, InIt end) 
		{
			for (InIt i = begin; i != end; ++i)
				erase(*i);
		}

		inline void SparseSet::clear() noexcept
		{
			n = 0;
		}

		inline SparseSet::const_pointer SparseSet::data() const noexcept
		{
			return dense.data();
		}

		inline bool SparseSet::contains(value_type v) const noexcept
		{
			if (v >= sparse.size()) return false;
			if (sparse[v] < n && dense[sparse[v]] == v) return true;
			return false;
		}

		template <class InIt>
		inline bool SparseSet::contains(InIt begin, InIt end) const noexcept {
			for (InIt i = begin; i != end; ++i)
				if (!contains(*i))
					return false;
			return true;
		}

	}
}