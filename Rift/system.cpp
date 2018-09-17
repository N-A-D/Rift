#include "system.h"

std::size_t rift::BaseSystem::m_family = 0;

rift::BaseSystem::~BaseSystem()
{
}

rift::SystemManager::SystemManager()
{
}

void rift::SystemManager::update(EntityManager & em, double dt)
{
	for (auto system_pair : systems)
		system_pair.second->update(em, dt);
}
