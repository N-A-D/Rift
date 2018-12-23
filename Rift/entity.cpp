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
	assert(valid() && "Cannot check if an invalid entity is waiting to be invalidated!");
	return manager->pending_invalidation(uid.index());
}

void rift::Entity::destroy() const noexcept
{
	assert(valid() && "Cannot destroy and invalid entity!");
	manager->destroy(uid.index());
}

rift::ComponentMask rift::Entity::component_mask() const noexcept
{
	assert(valid() && "Cannot get the component mask for an invalid entity!");
	return manager->component_mask_for(uid.index());
}

rift::Entity::Entity(EntityManager * manager, Entity::ID uid) noexcept
	: manager(manager), uid(uid)
{
}

Entity rift::EntityManager::create_entity() noexcept
{
	std::uint32_t index, version;
	if (free_indices.empty()) {
		index = static_cast<std::uint32_t>(masks.size());
		version = 1;
		masks.push_back(0);
		index_versions.push_back(version);
	}
	else {
		index = free_indices.front();
		version = index_versions[index];
		free_indices.pop();
	}
	return Entity(this, Entity::ID(index, version));
}

std::size_t rift::EntityManager::size() const noexcept
{
	return masks.size() - free_indices.size();
}

std::size_t rift::EntityManager::capacity() const noexcept
{
	return masks.capacity();
}

std::size_t rift::EntityManager::number_of_reusable_entities() const noexcept
{
	return free_indices.size();
}

std::size_t rift::EntityManager::number_of_entities_to_destroy() const noexcept
{
	return invalid_indices.size();
}

void rift::EntityManager::update() noexcept
{
	for (auto index : invalid_indices) {
		erase_caches_for(index);
		masks[index].reset();
		index_versions[index]++;
		free_indices.push(index);
	}
	invalid_indices.clear();
}

void rift::EntityManager::clear() noexcept
{
	invalid_indices.clear();
	while (!free_indices.empty())
		free_indices.pop();
	masks.clear();
	index_versions.clear();
	component_pools.clear();
	index_caches.clear();
}

ComponentMask rift::EntityManager::component_mask_for(std::uint32_t index) const noexcept
{
	return masks[index];
}

bool rift::EntityManager::pending_invalidation(std::uint32_t index) const noexcept
{
	return invalid_indices.contains(index);
}

bool rift::EntityManager::valid_id(const Entity::ID & id) const noexcept
{
	return id.index() < masks.size() && index_versions[id.index()] == id.version();
}

void rift::EntityManager::destroy(std::uint32_t index) noexcept
{
	if (!invalid_indices.contains(index))
		invalid_indices.insert(index);
}

void rift::EntityManager::erase_caches_for(std::uint32_t index)
{
	auto mask = component_mask_for(index);
	for (auto& index_cache : index_caches) {
		if ((mask & index_cache.first) == index_cache.first)
			index_cache.second.erase(index);
	}
}

bool rift::EntityManager::contains_cache_for(const ComponentMask & sig) const
{
	return index_caches.find(sig) != index_caches.end();
}

void rift::EntityManager::create_cache_for(const ComponentMask & sig)
{
	rift::impl::SparseSet indices;
	for (std::uint32_t i = 0; i < masks.size(); i++) {
		if ((masks[i] & sig) == sig)
			indices.insert(i);
	}
	index_caches.emplace(sig, indices);
}
