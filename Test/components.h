#pragma once

#include "..\Rift\component.h"

struct Position : public rift::Component<Position> {
	Position() : x(0.0), y(0.0) {}
	Position(double x, double y) : x(x), y(y) {}
	double x, y;
};

struct Velocity : public rift::Component<Velocity> {
	Velocity() : x(0.0), y(0.0) {}
	Velocity(double x, double y) : x(x), y(y) {}
	double x, y;
};

struct Direction : public rift::Component<Direction> {
	Direction() : x(0.0), y(0.0) {}
	Direction(double x, double y) : x(x), y(y) {}
	double x, y;
};