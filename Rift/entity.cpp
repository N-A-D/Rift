#include "entity.h"

rift::Entity::ID::ID()
	: m_index(0), m_version(0)
{
}

rift::Entity::ID::ID(std::size_t index, std::size_t version)
	: m_index(index)
	, m_version(version)
{
}

rift::Entity::ID & rift::Entity::ID::renew() noexcept
{
	++m_version; return *this;
}

std::size_t rift::Entity::ID::index() const noexcept
{
	return m_index;
}

std::size_t rift::Entity::ID::version() const noexcept
{
	return m_version;
}

rift::Entity::Entity()
	: mgr(nullptr)
	, m_id()
{
}

rift::Entity::Entity(rift::EntityManager* mgr, rift::Entity::ID id)
	: mgr(mgr)
	, m_id(id)
{
}

rift::Entity::ID rift::Entity::id() const noexcept
{
	return m_id;
}

bool rift::Entity::valid() const noexcept
{
	return mgr && mgr->valid_id(m_id);
}

rift::Entity::operator bool() const noexcept
{
	return valid();
}

void rift::Entity::invalidate() const noexcept
{
	assert(valid() && "Cannot invalidate an already invalid entity");
	mgr->mark_for_refresh(m_id);
}

bool rift::Entity::is_pending_invalidation() const noexcept
{
	assert(valid() && "Cannot check if an invalid entity is pending refresh");
	return mgr->is_pending_refresh(m_id);
}

rift::ComponentMask rift::Entity::component_mask() const noexcept
{
	assert(valid() && "Cannot fetch the component mask of an invalid entity");
	return mgr->component_mask(m_id);
}

bool rift::operator==(const rift::Entity & a, const rift::Entity & b) noexcept
{
	return a.mgr == b.mgr && a.id() == b.id();
}

bool rift::operator!=(const rift::Entity & a, const rift::Entity & b) noexcept
{
	return !(a == b);
}

bool rift::operator<(const rift::Entity::ID & a, const rift::Entity::ID & b) noexcept
{
	return a.index() < b.index();
}

bool rift::operator>(const rift::Entity::ID & a, const rift::Entity::ID & b) noexcept
{
	return b < a;
}

bool rift::operator==(const rift::Entity::ID & a, const rift::Entity::ID & b) noexcept
{
	return a.index() == b.index() && a.version() == b.version();
}

bool rift::operator!=(const rift::Entity::ID & a, const rift::Entity::ID & b) noexcept
{
	return !(a == b);
}

bool rift::operator<(const rift::Entity & a, const rift::Entity & b) noexcept
{
	return a.id() < b.id();
}

bool rift::operator>(const rift::Entity & a, const rift::Entity & b) noexcept
{
	return b.id() < a.id();
}

rift::EntityManager::BitMask::BitMask()
	: pending_refresh(false), id(), component_list(0)
{
}

rift::EntityManager::BitMask::BitMask(rift::Entity::ID id)
	: pending_refresh(false), id(id), component_list(0)
{
}

rift::Entity::ID rift::EntityManager::BitMask::refresh() noexcept
{
	component_list = 0; return id.renew();
}

rift::Entity rift::EntityManager::create_entity() noexcept
{
	if (!reusable_ids.empty()) {
		auto id = reusable_ids.front();
		reusable_ids.pop();
		return Entity(this, id);
	}
	else {
		return Entity(this, accommodate_entity());
	}
}

void rift::EntityManager::update() noexcept
{
	for (auto bitmask : bitmasks) {
		if (bitmask.pending_refresh) {
			reusable_ids.push(bitmask.refresh());
			bitmask.pending_refresh = false;
		}
	}
}

bool rift::EntityManager::valid_id(Entity::ID id) const noexcept
{
	return bitmasks[id.index()].id == id;
}

void rift::EntityManager::mark_for_refresh(Entity::ID id) noexcept
{
	if (!bitmasks[id.index()].pending_refresh)
		bitmasks[id.index()].pending_refresh = true;
}

bool rift::EntityManager::is_pending_refresh(Entity::ID id) noexcept
{
	return bitmasks[id.index()].pending_refresh;
}

rift::Entity::ID rift::EntityManager::accommodate_entity() noexcept
{
	auto index_number = bitmasks.size();
	bitmasks.push_back(BitMask(Entity::ID(index_number, 1)));
	for (auto pool : component_pools) {
		pool->allocate(1);
		assert(pool->size() == bitmasks.size());
	}
	return bitmasks.back().id;
}

rift::ComponentMask rift::EntityManager::component_mask(Entity::ID id) const noexcept
{
	return bitmasks[id.index()].component_list;
}
