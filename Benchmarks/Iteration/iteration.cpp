#include "../../Rift/rift.h"
#include <vector>
#include <tuple>
#include <chrono>
#include <iostream>
#include <cassert>

class Timer {
	std::chrono::high_resolution_clock::time_point start;
	const char *str;
public:
	Timer(const char *str) : start(std::chrono::high_resolution_clock::now()),
		str(str) {}
	~Timer() {
		auto end = std::chrono::high_resolution_clock::now();
		std::cout << str << std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count() << "\n";
	}
};

struct Position : public rift::Component<Position> {
	Position() = default;
	Position(float x, float y) : x(x), y(y) {}
	float x = 0, y = 0;
};

struct Direction : public rift::Component<Direction> {
	Direction() = default;
	Direction(float x, float y) : x(x), y(y) {}
	float x = 0, y = 0;
};


void seq_run(int entity_count, int iteration_count) {
	using namespace rift;
	std::cout << "-------------------------------" << std::endl;
	std::cout << "Entity count: " << entity_count << std::endl;
	std::cout << "Iterations: " << iteration_count << std::endl;

	// Double the entity count as we're trying to get interleaved entity indices
	entity_count *= 2;

	EntityManager manager;
	std::vector<rift::Entity> entities;
	entities.reserve(entity_count);
	{
		// Allocate entity_count number of entities
		for (int i = 0; i < entity_count; i++) {
			entities.push_back(manager.create_entity());
			entities[i].add<Position>(1.f, 1.f);
			entities[i].add<Direction>(1.f, 1.f);
		}
	}
	{
		// Build the index cache
		manager.for_entities_with<Position, Direction>([](Entity, Position& p, Direction& d) {});
	}
	{
		// Destroy every other entity
		for (int i = 0; i < entity_count; i++) {
			if (i % 2 == 0)
				entities[i].destroy();
		}
		manager.update();
		assert((manager.number_of_entities_with<Position, Direction>() == entity_count / 2));
		assert((manager.number_of_reusable_entities() == entity_count / 2));
	}
	{
		Timer timer("Iteration speed: ");
		for (int i = 0; i < iteration_count; i++) {
			manager.for_entities_with<Position, Direction>([](Entity e, Position& p, Direction& d) {
				p.x += d.x * 1.0f;
				p.y += d.y * 1.0f;
			});
		}
	}
}

void par_run(int entity_count, int iteration_count) {
	using namespace rift;
	std::cout << "-------------------------------" << std::endl;
	std::cout << "Entity count: " << entity_count << std::endl;
	std::cout << "Iterations: " << iteration_count << std::endl;

	// Double the entity count as we're trying to get interleaved entity indices
	entity_count *= 2;

	EntityManager manager;
	std::vector<rift::Entity> entities;
	entities.reserve(entity_count);
	{
		// Allocate entity_count number of entities
		for (int i = 0; i < entity_count; i++) {
			entities.push_back(manager.create_entity());
			entities[i].add<Position>(1.f, 1.f);
			entities[i].add<Direction>(1.f, 1.f);
		}
	}
	{
		// Build the index cache
		manager.for_entities_with<Position, Direction>([](Entity, Position& p, Direction& d) {});
	}
	{
		// Destroy every other entity
		for (int i = 0; i < entity_count; i++) {
			if (i % 2 == 0)
				entities[i].destroy();
		}
		manager.update();
		assert((manager.number_of_entities_with<Position, Direction>() == entity_count / 2));
		assert((manager.number_of_reusable_entities() == entity_count / 2));
	}
	{

		Timer timer("Iteration speed: ");
		for (int i = 0; i < iteration_count; i++) {
			manager.par_for_entities_with<Position, Direction>([](Position& p, Direction& d) {
				p.x += d.x * 1.0f;
				p.y += d.y * 1.0f;
			});
		}
	}
}

int main() {

	std::vector<std::tuple<int, int>> run_params;
	run_params.push_back(std::make_tuple(1000, 1000));
	run_params.push_back(std::make_tuple(3000, 1000));
	run_params.push_back(std::make_tuple(5000, 1000));
	run_params.push_back(std::make_tuple(10000, 1000));
	run_params.push_back(std::make_tuple(30000, 1000));
	run_params.push_back(std::make_tuple(50000, 1000));
	run_params.push_back(std::make_tuple(100000, 1000));
	run_params.push_back(std::make_tuple(300000, 1000));
	run_params.push_back(std::make_tuple(500000, 1000));

	std::cout << "-- Sequential transformations --" << std::endl;
	for (auto param : run_params)
		seq_run(std::get<0>(param), std::get<1>(param));

	std::cout << std::endl;

	std::cout << "-- Parallel transformations --" << std::endl;
	for (auto param : run_params)
		par_run(std::get<0>(param), std::get<1>(param));

	return 0;
}