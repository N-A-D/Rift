#include "system.h"
#include "entity.h"

std::size_t rift::BaseSystem::m_family = 0;

rift::SystemManager::SystemManager(rift::EntityManager & entity_manager)
	: entity_manager(entity_manager)
{
}

void rift::SystemManager::update(double dt)
{
	// Update all valid systems
	for (auto system : systems) {
		if (system) {
			system->update(entity_manager, dt);
		}
	}
	// Update the entity_manager
	entity_manager.update();
}
