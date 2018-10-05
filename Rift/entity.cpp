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

rift::EntityManager * rift::Entity::manager() const noexcept
{
	return mgr;
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

void rift::Entity::destroy() const noexcept
{
	assert(valid() && "Cannot destroy an invalid entity");
	mgr->invalidate_id(m_id);
}

rift::ComponentMask rift::Entity::component_mask() const noexcept
{
	assert(valid() && "Cannot fetch the component mask of an invalid entity");
	return mgr->component_mask(m_id);
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

bool rift::operator==(const rift::Entity & a, const rift::Entity & b) noexcept
{
	return a.manager() == b.manager() && a.id() == b.id();
}

bool rift::operator!=(const rift::Entity & a, const rift::Entity & b) noexcept
{
	return !(a == b);
}

rift::EntityManager::EntityRecord::EntityRecord()
	: entity_id()
	, component_list(0)
{
}

rift::EntityManager::EntityRecord::EntityRecord(rift::Entity::ID id)
	: entity_id(id)
	, component_list(0)
{
}

rift::Entity::ID rift::EntityManager::EntityRecord::refresh_id() noexcept
{
	component_list.reset(); return entity_id.renew();
}

rift::EntityManager::EntityManager()
{
}

rift::Entity rift::EntityManager::create_entity() noexcept
{
	if (!reusable_ids.empty()) {
		auto id = reusable_ids.front();
		reusable_ids.pop();
		return Entity(this, id);
	}
	else {
		return Entity(this, allocate_entity_record());
	}
}

bool rift::EntityManager::valid_id(Entity::ID id) const noexcept
{
	return entity_records.at(id.index()).entity_id == id;
}

void rift::EntityManager::invalidate_id(Entity::ID id) noexcept
{
	reusable_ids.push(entity_records.at(id.index()).refresh_id());
}

rift::Entity::ID rift::EntityManager::allocate_entity_record() noexcept
{
	auto index_number = entity_records.size();
	entity_records.push_back(EntityRecord(Entity::ID(index_number, 1)));
	for (auto pool : component_pools) {
		pool.second->allocate(1);
		assert(pool.second->size() == entity_records.size());
	}
	return entity_records.back().entity_id;
}

rift::ComponentMask rift::EntityManager::component_mask(Entity::ID id) const noexcept
{
	return entity_records.at(id.index()).component_list;
}
