#include "entity.h"

using namespace rift;

Entity::ID rift::Entity::id() const noexcept
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
	assert(valid() && "Cannot destroy an invalid entity!");
	mgr->destroy(m_id);
}

rift::ComponentMask rift::Entity::component_mask() const noexcept
{
	assert(valid() && "Cannot get the component mask for an invalid entity!");
	return mgr->component_mask_for(m_id);
}

Entity rift::EntityManager::create_entity() noexcept
{
	if (reusable_ids.empty()) {
		std::size_t index = masks.size();
		masks.push_back(0);
		id_versions.push_back(1);
		return Entity(this, Entity::ID(index, id_versions.back()));
	}
	else {
		auto id = reusable_ids.front();
		reusable_ids.pop();
		return Entity(this, id);
	}
}

bool rift::EntityManager::valid_id(const Entity::ID & id) const noexcept
{
	return id_versions[id.index()] == id.version();
}

void rift::EntityManager::destroy(const Entity::ID & id) noexcept
{
	delete_components_for(id);
	delete_any_caches_for(id);
	auto idx = id.index();
	masks[idx] = 0;
	++id_versions[idx];
	reusable_ids.push(Entity::ID(idx, id_versions[idx]));
}

void rift::EntityManager::delete_components_for(const Entity::ID & id) noexcept
{
	auto mask = component_mask_for(id);
	for (std::size_t i = 0; i < mask.size(); i++) {
		if (mask[i]) {
			component_caches[i]->erase(id.index());
		}
	}
}

void rift::EntityManager::delete_any_caches_for(const Entity::ID & id) noexcept
{
	auto mask = component_mask_for(id);
	for (auto& search_cache : search_caches) {
		if ((mask & search_cache.first) == search_cache.first) {
			search_cache.second.erase(id.index());
		}
	}
}

ComponentMask rift::EntityManager::component_mask_for(const Entity::ID & id) const noexcept
{
	return masks[id.index()];
}
