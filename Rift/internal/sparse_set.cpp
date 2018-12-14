#include "sparse_set.h"
#include <algorithm>
#include <cassert>

bool rift::impl::SparseSet::empty() const noexcept
{
	return n == 0;
}

rift::impl::SparseSet::size_type rift::impl::SparseSet::size() const noexcept
{
	return n;
}

rift::impl::SparseSet::size_type rift::impl::SparseSet::max_size() const noexcept
{
	return sparse.max_size();
}

rift::impl::SparseSet::size_type rift::impl::SparseSet::capacity() const noexcept
{
	return sparse.capacity();
}

rift::impl::SparseSet::const_iterator rift::impl::SparseSet::begin() const noexcept
{
	return const_iterator(data());
}

rift::impl::SparseSet::const_iterator rift::impl::SparseSet::end() const noexcept
{
	return const_iterator(data() + size());
}

void rift::impl::SparseSet::insert(value_type v)
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

void rift::impl::SparseSet::erase(value_type v)
{
	assert(contains(v));
	dense[sparse[v]] = dense[n - 1];
	sparse[dense[n - 1]] = sparse[v];
	--n;
}

void rift::impl::SparseSet::clear() noexcept
{
	n = 0;
}

rift::impl::SparseSet::const_pointer rift::impl::SparseSet::data() const noexcept
{
	return dense.data();
}

bool rift::impl::SparseSet::contains(value_type v) const noexcept
{
	if (v >= sparse.size()) return false;
	if (sparse[v] < n && dense[sparse[v]] == v) return true;
	return false;
}

void rift::impl::SparseSet::sort()
{
	std::sort(dense.begin(), dense.begin() + n);
	for (value_type i = 0; i < n; i++)
		sparse[dense[i]] = i;
}
