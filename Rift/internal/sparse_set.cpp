#include "sparse_set.h"
#include <algorithm>
#include <cassert>

bool rift::internal::SparseSet::empty() const noexcept
{
	return n == 0;
}

rift::internal::SparseSet::size_type rift::internal::SparseSet::size() const noexcept
{
	return n;
}

rift::internal::SparseSet::size_type rift::internal::SparseSet::max_size() const noexcept
{
	return sparse.max_size();
}

rift::internal::SparseSet::size_type rift::internal::SparseSet::capacity() const noexcept
{
	return sparse.capacity();
}

rift::internal::SparseSet::const_iterator rift::internal::SparseSet::begin() const noexcept
{
	return const_iterator(data());
}

rift::internal::SparseSet::const_iterator rift::internal::SparseSet::end() const noexcept
{
	return const_iterator(data() + size());
}

void rift::internal::SparseSet::insert(value_type v)
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

void rift::internal::SparseSet::erase(value_type v)
{
	assert(contains(v));
	dense[sparse[v]] = dense[n - 1];
	sparse[dense[n - 1]] = sparse[v];
	--n;
}

void rift::internal::SparseSet::clear() noexcept
{
	n = 0;
}

rift::internal::SparseSet::const_pointer rift::internal::SparseSet::data() const noexcept
{
	return dense.data();
}

bool rift::internal::SparseSet::contains(value_type v) const noexcept
{
	if (v >= sparse.size()) return false;
	if (sparse[v] < n && dense[sparse[v]] == v) return true;
	return false;
}
