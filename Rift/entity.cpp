#include "entity.h"

rift::Entity::ID::ID()
	: m_number(0)
{
}

rift::Entity::ID::ID(std::size_t index, std::size_t version)
	: m_number(std::uint64_t(index) << 32 | std::uint64_t(version))
{
}

std::uint64_t rift::Entity::ID::number() const noexcept
{
	return m_number;
}

void rift::Entity::ID::renew() noexcept
{
	*this = ID(index(), version() + 1);
}

std::size_t rift::Entity::ID::index() const noexcept
{
	return m_number >> 32;
}

std::size_t rift::Entity::ID::version() const noexcept
{
	return m_number & 0xffffffffUL;
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
	return a.number() < b.number();
}

bool rift::operator>(const rift::Entity::ID & a, const rift::Entity::ID & b) noexcept
{
	return b < a;
}

bool rift::operator==(const rift::Entity::ID & a, const rift::Entity::ID & b) noexcept
{
	return !(a < b) && !(b < a);
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
		std::size_t index = masks.size();
		masks.push_back(ComponentMask(0));
		id_versions.push_back(1);
		return Entity(this, Entity::ID(index, id_versions.back()));
	}
}

bool rift::EntityManager::valid_id(const Entity::ID& id) const noexcept
{
	return id_versions.at(id.index()) == id.version();
}

void rift::EntityManager::invalidate_id(const Entity::ID& id) noexcept
{
	masks.at(id.index()) = 0;
	id_versions.at(id.index())++;
	reusable_ids.push(Entity::ID(id.index(), id_versions.at(id.index())));
}

rift::ComponentMask rift::EntityManager::component_mask(const Entity::ID& id) const noexcept
{
	return masks.at(id.index());
}
