#include "entity.h"

rift::Entity::Entity()
	: m_id(), mgr(nullptr)
{
}

rift::Entity::Entity(EntityManager * em, rift::Entity::ID id)
	: mgr(em), m_id(id)
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
	return mgr->component_mask_for(m_id);
}

bool rift::operator<(const rift::Entity & a, const rift::Entity & b) noexcept
{
	return a.m_id < b.m_id;
}

bool rift::operator>(const rift::Entity & a, const rift::Entity & b) noexcept
{
	return b < a;
}

bool rift::operator==(const rift::Entity & a, const rift::Entity & b) noexcept
{
	return !(a < b) && !(b < a);
}

bool rift::operator!=(const rift::Entity & a, const rift::Entity & b) noexcept
{
	return !(a == b);
}

bool rift::operator<(const Entity::ID & a, const Entity::ID & b) noexcept
{
	return a.number() < b.number();
}

bool rift::operator>(const Entity::ID & a, const Entity::ID & b) noexcept
{
	return b < a;
}

bool rift::operator==(const Entity::ID & a, const Entity::ID & b) noexcept
{
	return !(a < b) && !(b < a);
}

bool rift::operator!=(const Entity::ID & a, const Entity::ID & b) noexcept
{
	return !(a == b);
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

bool rift::EntityManager::valid_id(const rift::Entity::ID & id) const noexcept
{
	return id_versions.at(id.index()) == id.version();
}

void rift::EntityManager::invalidate_id(const rift::Entity::ID & id) noexcept
{
	delete_components_for(id);
	auto idx = id.index();
	masks.at(idx) = 0;
	++id_versions.at(idx);
	reusable_ids.push(Entity::ID(idx, id_versions.at(idx)));
}

void rift::EntityManager::delete_components_for(const Entity::ID & id) noexcept
{
	// Get the mask for the id. For each bit that is active, that is, each component the entity had,
	// go to the component pool associated with that bit (Component type id) and remove the component that belonged to the entity
	auto mask = component_mask_for(id);
	for (std::size_t i = 0; i < mask.size(); i++) {
		if (mask.test(i)) {
			component_pools.at(i)->erase(id.index());
		}
	}
}

rift::ComponentMask rift::EntityManager::component_mask_for(const rift::Entity::ID & id) const noexcept
{
	return masks.at(id.index());
}
