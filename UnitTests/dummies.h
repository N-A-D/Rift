#pragma once

#include "../Rift/rift.h"

struct Position : public rift::Component<Position> {
	Position() : x(0.0), y(0.0) {}
	Position(double x, double y) : x(x), y(y) {}
	double x, y;
};

struct Direction : public rift::Component<Direction> {
	Direction() : x(0.0), y(0.0) {}
	Direction(double x, double y) : x(x), y(y) {}
	double x, y;
};

struct Movement : public rift::System<Movement> {
	Movement() {}

	void update(rift::EntityManager& em, double dt) noexcept override {
		em.entities_with<Position, Direction>([dt](const rift::Entity& e) {
			Position &p = e.get<Position>();
			Direction& d = e.get<Direction>();
			p.x += d.x * dt;
			p.y += d.y * dt;
		});
	}
};