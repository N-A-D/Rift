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

rift::Entity::Record::Record()
	: id()
	, component_list(0)
{
}

rift::Entity::Record::Record(rift::Entity::ID id)
	: id(id)
	, component_list(0)
{
}

rift::Entity::ID rift::Entity::Record::renew_master_id() noexcept
{
	component_list.reset(); return id.renew();
}

rift::Entity::ID rift::Entity::Record::master_id_copy() const noexcept
{
	return id;
}

rift::ComponentMask rift::Entity::Record::components() const noexcept
{
	return component_list;
}

void rift::Entity::Record::insert_component(ComponentFamily family) noexcept
{
	component_list.set(family);
}

void rift::Entity::Record::remove_component(ComponentFamily family) noexcept
{
	component_list.reset(family);
}

bool rift::Entity::Record::check_component(ComponentFamily family) noexcept
{
	return component_list.test(family);
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
		std::size_t index = entity_records.size();
		entity_records.emplace_back(Entity::Record(Entity::ID(index, 1)));
		return Entity(this, entity_records.back().master_id_copy());
	}
}

bool rift::EntityManager::valid_id(const Entity::ID& id) const noexcept
{
	return entity_records.at(id.index()).master_id_copy() == id;
}

void rift::EntityManager::invalidate_id(const Entity::ID& id) noexcept
{
	reusable_ids.push(entity_records.at(id.index()).renew_master_id());
}

rift::ComponentMask rift::EntityManager::component_mask(const Entity::ID& id) const noexcept
{
	return entity_records.at(id.index()).components();
}

rift::EntityManager::Cache::Cache()
	: n(0), dense(1, Entity())
{
}

rift::EntityManager::Cache::iterator rift::EntityManager::Cache::begin()
{
	return dense.begin();
}

rift::EntityManager::Cache::iterator rift::EntityManager::Cache::end()
{
	return dense.begin() + n;
}

rift::EntityManager::Cache::const_iterator rift::EntityManager::Cache::begin() const
{
	return dense.begin();;
}

rift::EntityManager::Cache::const_iterator rift::EntityManager::Cache::end() const
{
	return dense.begin() + n;
}

bool rift::EntityManager::Cache::search(const Entity & e)
{
	auto idx = e.id().index();
	if (idx >= sparse.size())
		return false;
	if (sparse.at(idx) < n && dense.at(sparse.at(idx)) == e)
		return true;
	return false;
}

void rift::EntityManager::Cache::insert(const Entity & e)
{
	if (search(e))
		return;
	auto idx = e.id().index();
	if (idx >= sparse.size()) {
		sparse.resize(idx + 1);
	}
	if (n >= dense.size())
		dense.resize(n + 1);
	dense.at(n) = e;
	sparse.at(idx) = n++;
}

void rift::EntityManager::Cache::remove(const Entity & e)
{
	if (!search(e))
		return;
	auto idx = e.id().index();
	auto entity = dense.at(n - 1);
	dense.at(sparse.at(idx)) = entity;
	sparse.at(entity.id().index()) = sparse.at(idx);
	--n;
}

void rift::EntityManager::Cache::clear()
{
	n = 0;
}

bool rift::EntityManager::Cache::empty() const
{
	return n == 0;
}
