#pragma once

#include "components.h"
#include "..\Rift\system.h"
#include "..\Rift\entity.h"

struct Movement : public rift::System<Movement> {
	Movement() {}

	void update(rift::EntityManager& em, double dt) noexcept override {
		em.entities_with<Position, Velocity>([dt](const rift::Entity& e) {
			Position &p = e.get<Position>();
			Velocity  v = e.get<Velocity>();
			p.x += v.x * dt;
			p.y += v.y * dt;
		});
	}
};