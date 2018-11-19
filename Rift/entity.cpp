#include "entity.h"

using namespace rift;

const Entity::ID Entity::INVALID_ID;

Entity::ID rift::Entity::id() const noexcept
{
	return uid;
}

bool rift::Entity::valid() const noexcept
{
	return manager && manager->valid_id(uid);
}

rift::Entity::operator bool() const noexcept
{
	return valid();
}

bool rift::Entity::pending_invalidation() const noexcept
{
	assert(valid() && "Cannot check if an invalid entity is waiting for deletion!");
	return manager->pending_invalidation(uid);
}

void rift::Entity::destroy() const noexcept
{
	assert(valid() && "Cannot destroy an invalid entity!");
	manager->destroy(uid);
}

rift::ComponentMask rift::Entity::component_mask() const noexcept
{
	assert(valid() && "Cannot get the component mask for an invalid entity!");
	return manager->component_mask_for(uid);
}

rift::Entity::Entity(EntityManager * manager, Entity::ID uid) noexcept
	: manager(manager), uid(uid)
{
}

rift::EntityManager::EntityManager(std::size_t starting_size) noexcept
	: masks(starting_size, 0), index_versions(starting_size, 1)
{
	for (std::size_t i = 0; i < starting_size; i++)
		free_indexes.push(static_cast<std::uint32_t>(i));
}

Entity rift::EntityManager::create_entity() noexcept
{
	if (free_indexes.empty()) {
		auto index = masks.size();
		masks.push_back(0);
		index_versions.push_back(1);
		return Entity(this, Entity::ID(static_cast<std::uint32_t>(index), index_versions.back()));
	}
	else {
		auto index = free_indexes.front();
		free_indexes.pop();
		return Entity(this, Entity::ID(index, index_versions[index]));
	}
}

std::size_t rift::EntityManager::size() const noexcept
{
	return masks.size() - free_indexes.size();
}

std::size_t rift::EntityManager::capacity() const noexcept
{
	return masks.size();
}

std::size_t rift::EntityManager::number_of_reusable_entities() const noexcept
{
	return free_indexes.size();
}

std::size_t rift::EntityManager::number_of_entities_to_destroy() const noexcept
{
	return invalid_ids.size();
}

void rift::EntityManager::update() noexcept
{
	for (auto id : invalid_ids) {
		delete_components_for(id);
		delete_all_caches_for(id);
		masks[id.index()].reset();
		index_versions[id.index()]++;
		free_indexes.push(id.index());
	}
	invalid_ids.clear();
}

void rift::EntityManager::clear() noexcept
{
	invalid_ids.clear();
	while (!free_indexes.empty())
		free_indexes.pop();
	masks.clear();
	index_versions.clear();
	component_caches.clear();
	entity_caches.clear();
}

bool rift::EntityManager::valid_id(const Entity::ID & id) const noexcept
{
	return index_versions[id.index()] == id.version();
}

ComponentMask rift::EntityManager::component_mask_for(const Entity::ID & id) const noexcept
{
	return masks[id.index()];
}

bool rift::EntityManager::pending_invalidation(const Entity::ID & id) const noexcept
{
	return invalid_ids.contains(id.index());
}

void rift::EntityManager::destroy(const Entity::ID & id) noexcept
{
	if (!invalid_ids.contains(id.index())) {
		auto idx(id);
		invalid_ids.insert(id.index(), &idx);
	}
}

void rift::EntityManager::delete_components_for(const Entity::ID & id) noexcept
{
	auto mask = component_mask_for(id);

	std::size_t index = 0;
	auto value = mask.to_ullong();
	while (value != 0) {
		value /= 2;
		if (mask.test(index)) {
			component_caches[index]->erase(id.index());
		}
		++index;
	}
}

void rift::EntityManager::delete_all_caches_for(const Entity::ID & id) noexcept
{
	auto mask = component_mask_for(id);
	for (auto& entity_cache : entity_caches) {
		if ((mask & entity_cache.first) == entity_cache.first) {
			entity_cache.second.erase(id.index());
		}
	}
}

bool rift::EntityManager::contains_entity_cache_for(const ComponentMask & signature) const noexcept
{
	return entity_caches.find(signature) != entity_caches.end();
}

void rift::EntityManager::create_entity_cache_for(const ComponentMask & signature) noexcept
{
	rift::util::Cache<Entity> entity_cache;
	for (std::size_t i = 0; i < masks.size(); i++) {
		if ((masks[i] & signature) == signature) {
			Entity e(this, Entity::ID(static_cast<std::uint32_t>(i), index_versions[i]));
			entity_cache.insert(i, &e);
		}
	}
	entity_caches.emplace(signature, entity_cache);
}
