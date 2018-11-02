#pragma once

#include "../rift/core/rift.h"

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

struct Toggle : public rift::Component<Toggle> {
	Toggle() : on(false) {}
	Toggle(bool start) : on(start) {}
	bool on;
};

struct ToggleSystem : public rift::System<ToggleSystem> {

	void update(rift::EntityManager& em, double dt) noexcept override {
		em.for_each_entity_with<Toggle>([](rift::Entity e) {
			auto& t = e.get<Toggle>();
			t.on = true;
		});
	}
};

struct MovementSystem : public rift::System<MovementSystem> {
	MovementSystem() {}

	void update(rift::EntityManager& em, double dt) noexcept override {
		em.for_each_entity_with<Position, Direction>([dt](rift::Entity e) {
			Position &p = e.get<Position>();
			Direction& d = e.get<Direction>();
			p.x += d.x * dt;
			p.y += d.y * dt;
		});
	}
};
