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

struct A {};
struct B {};
struct C {};
struct D {};

void create_and_destroy(int entity_count) noexcept {
	rift::EntityManager entities;
	std::cout << "Entity count: " << entity_count << std::endl;
	{
		Timer timer("Creating entities: ");
		for (int i = 0; i < entity_count; ++i) {
			auto e = entities.create_entity();
			e.add<A>();
			e.add<B>();
			e.add<C>();
			e.add<D>();
		}
	}
	{
		// Build search caches based on type pairs
		entities.for_entities_with<A, B>([](rift::Entity, A&, B&) {});
		entities.for_entities_with<A, C>([](rift::Entity, A&, C&) {});
		entities.for_entities_with<A, D>([](rift::Entity, A&, D&) {});
		entities.for_entities_with<B, C>([](rift::Entity, B&, C&) {});
		entities.for_entities_with<B, D>([](rift::Entity, B&, D&) {});
		entities.for_entities_with<C, D>([](rift::Entity, C&, D&) {});
		entities.for_entities_with<A, B, C, D>([](rift::Entity, A&, B&, C&, D&) {});
	}
	{
		Timer timer("Destroying entities: ");
		entities.for_entities_with<A, B, C, D>([](rift::Entity e, A&, B&, C&, D&) {
			e.destroy();
		});
		entities.update();
	}
	std::cout << "-----------------------------" << std::endl;
}

void clone_and_destroy(int entity_count) noexcept {
	rift::EntityManager entities;
	std::cout << "Entity count: " << entity_count << std::endl;
	{
		Timer timer("Cloning entities: ");
		auto original = entities.create_entity();
		original.add<A>();
		original.add<B>();
		original.add<C>();
		original.add<D>();

		for (int i = 0; i < entity_count - 1; ++i) {
			entities.create_copy_of(original);
		}
	}
	{
		// Build search caches based on type pairs
		entities.for_entities_with<A, B>([](rift::Entity, A&, B&) {});
		entities.for_entities_with<A, C>([](rift::Entity, A&, C&) {});
		entities.for_entities_with<A, D>([](rift::Entity, A&, D&) {});
		entities.for_entities_with<B, C>([](rift::Entity, B&, C&) {});
		entities.for_entities_with<B, D>([](rift::Entity, B&, D&) {});
		entities.for_entities_with<C, D>([](rift::Entity, C&, D&) {});
		entities.for_entities_with<A, B, C, D>([](rift::Entity, A&, B&, C&, D&) {});
	}
	{
		Timer timer("Destroying entities: ");
		entities.for_entities_with<A, B, C, D>([](rift::Entity e, A&, B&, C&, D&) {
			e.destroy();
		});
		entities.update();
	}
	std::cout << "-----------------------------" << std::endl;
}

int main() {
	std::cout << "-----------------------------" << std::endl;

	create_and_destroy(1'000'000);
	create_and_destroy(5'000'000);
	create_and_destroy(10'000'000);

	std::cout << "-----------------------------" << std::endl;

	clone_and_destroy(1'000'000);
	clone_and_destroy(5'000'000);
	clone_and_destroy(10'000'000);

	std::cout << "-----------------------------" << std::endl;

	return 0;
}