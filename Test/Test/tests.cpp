#include "stdafx.h"
#include "CppUnitTest.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;
#define RIFT_ENABLE_PARALLEL_TRANSFORMATIONS
#include "../../Rift/rift.h"
#include <array>

namespace Test
{
	using namespace rift;

	const std::size_t NUM_ENTITIES_WITH = 100;

	struct Toggle {
		Toggle() = default;
		Toggle(bool state) : state(state) {}
		bool state = false;
	};

	struct ToggleSystem : public System<ToggleSystem> {
		ToggleSystem() = default;
		void update(EntityManager& manager, double dt) override {
			manager.for_entities_with<Toggle>([](Entity entity, Toggle& toggle) {
				toggle.state = true;
			});
		}
	};

	struct ParToggleSystem : public System<ParToggleSystem> {
		ParToggleSystem() = default;
		void update(EntityManager& manager, double dt) override {
			manager.par_for_entities_with<Toggle>([](Toggle& toggle) {
				toggle.state = true;
			});
		}
	};

	TEST_CLASS(EntityTests) {

		TEST_METHOD(MarkingForDestruction) {
			EntityManager manager;
			auto a = manager.create_entity();
			auto b(a);
			a.destroy();
			Assert::IsTrue(a && b);
			Assert::IsTrue(manager.number_of_entities_to_destroy() == 1);
			Assert::IsTrue(a.marked_for_destruction() && b.marked_for_destruction());
			manager.update();
			Assert::IsFalse(a);
			Assert::IsFalse(b);
		}

		TEST_METHOD(AddingComponents) {
			EntityManager manager;
			std::vector<Entity> entities;
			for (int i = 0; i < NUM_ENTITIES_WITH; i++) {
				auto e = manager.create_entity();
				e.add<Toggle>(true);
				entities.push_back(e);
			}
			for (auto entity : entities) {
				Assert::IsTrue(entity);
				Assert::IsTrue(entity.has<Toggle>());
				Assert::IsTrue(entity.get<Toggle>().state);
			}
		}

		TEST_METHOD(RemovingComponents) {
			EntityManager manager;
			std::vector<Entity> entities;
			for (int i = 0; i < NUM_ENTITIES_WITH; i++) {
				auto e = manager.create_entity();
				e.add<Toggle>();
				entities.push_back(e);
			}
			for (auto entity : entities) {
				Assert::IsTrue(entity);
				Assert::IsTrue(entity.has<Toggle>());
				entity.remove<Toggle>();
			}
			for (auto entity : entities) {
				Assert::IsTrue(entity);
				Assert::IsFalse(entity.has<Toggle>());
			}
		}

		TEST_METHOD(ReplacingComponents) {
			EntityManager manager;
			std::vector<Entity> entities;
			for (int i = 0; i < NUM_ENTITIES_WITH; i++) {
				auto e = manager.create_entity();
				e.add<Toggle>();
				entities.push_back(e);
			}

			for (auto entity : entities) {
				Assert::IsTrue(entity.has<Toggle>());
				Assert::IsFalse(entity.get<Toggle>().state);
				entity.replace<Toggle>(true);
			}
			for (auto entity : entities) {
				Assert::IsTrue(entity);
				Assert::IsTrue(entity.get<Toggle>().state);
			}

		}

		TEST_METHOD(UpdatingComponents) {
			EntityManager manager;
			std::vector<Entity> entities;
			for (int i = 0; i < NUM_ENTITIES_WITH; i++) {
				auto e = manager.create_entity();
				e.add<Toggle>();
				entities.push_back(e);
			}
			for (auto entity : entities) {
				Assert::IsTrue(entity);
				Assert::IsFalse(entity.get<Toggle>().state);
				entity.replace<Toggle>(true);
			}
			for (auto entity : entities) {
				Assert::IsTrue(entity);
				Assert::IsTrue(entity.get<Toggle>().state);
			}
		}
	};

	TEST_CLASS(SystemTests) {
		TEST_METHOD(SequentialUpdates) {
			EntityManager manager;
			std::vector<Entity> entities;
			for (int i = 0; i < NUM_ENTITIES_WITH; i++) {
				auto e = manager.create_entity();
				e.add<Toggle>();
				entities.push_back(e);
			}

			ToggleSystem ts;
			ts.update(manager, 1.0);

			Assert::IsTrue(std::all_of(entities.begin(), entities.end(), [](Entity e) {
				return e.get<Toggle>().state;
			}));
		}

		TEST_METHOD(ParallelUpdates) {
			EntityManager manager;
			std::vector<Entity> entities;
			for (int i = 0; i < NUM_ENTITIES_WITH; i++) {
				auto e = manager.create_entity();
				e.add<Toggle>();
				entities.push_back(e);
			}

			ParToggleSystem par_ts;
			par_ts.update(manager, 1.0);

			Assert::IsTrue(std::all_of(entities.begin(), entities.end(), [](Entity e) {
				return e.get<Toggle>().state;
			}));
		}
	};

	TEST_CLASS(EntityManagerTests) {
		TEST_METHOD(MassEntityCreation) {
			std::vector<Entity> entities;
			EntityManager manager;
			for (int i = 0; i < NUM_ENTITIES_WITH; i++) {
				entities.push_back(manager.create_entity());
			}
			Assert::IsTrue(manager.size() == entities.size());
			for (auto entity : entities) {
				Assert::IsTrue(entity.valid());
			}
		}

		TEST_METHOD(CountingEntitiesWith) {
			EntityManager manager;
			for (int i = 0; i < NUM_ENTITIES_WITH; i++) {
				auto e = manager.create_entity();
				if (i % 2 == 0)
					e.add<Toggle>();
			}
			Assert::IsTrue(manager.size() == NUM_ENTITIES_WITH);
			Assert::IsTrue(manager.number_of_entities_with<Toggle>() == NUM_ENTITIES_WITH / 2);
		}

		TEST_METHOD(CountingEntitiesToDestroy) {
			EntityManager manager;
			std::vector<Entity> entities;
			for (int i = 0; i < NUM_ENTITIES_WITH; i++) {
				entities.push_back(manager.create_entity());
			}
			Assert::IsTrue(entities.size() == manager.size());
			for (auto entity : entities) {
				entity.destroy();
			}
			Assert::IsTrue(manager.size() == NUM_ENTITIES_WITH);
			Assert::IsTrue(manager.number_of_entities_to_destroy() == NUM_ENTITIES_WITH);
		}

		TEST_METHOD(CountingReusableEntities) {
			EntityManager manager;
			std::vector<Entity> entities;
			for (int i = 0; i < NUM_ENTITIES_WITH; i++) {
				entities.push_back(manager.create_entity());
			}
			Assert::IsTrue(entities.size() == manager.size());
			for (auto entity : entities) {
				entity.destroy();
			}
			Assert::IsTrue(manager.size() == NUM_ENTITIES_WITH);
			Assert::IsTrue(manager.number_of_entities_to_destroy() == NUM_ENTITIES_WITH);
			manager.update();
			Assert::IsTrue(manager.size() == 0);
			Assert::IsTrue(manager.number_of_entities_to_destroy() == 0);
			Assert::IsTrue(manager.number_of_reusable_entities() == NUM_ENTITIES_WITH);
		}

		TEST_METHOD(RepeatedEntityDestruction) {
			EntityManager manager;
			std::vector<Entity> entities;
			for (int i = 0; i < NUM_ENTITIES_WITH; i++) {
				auto e = manager.create_entity();
				e.add<Toggle>(true);
				entities.push_back(e);
			}
			for (int i = 0; i < NUM_ENTITIES_WITH; i++) {
				for (auto entity : entities) {
					Assert::IsTrue(entity);
					entity.destroy();
					Assert::IsTrue(entity);
				}
				Assert::IsTrue(manager.number_of_entities_to_destroy() == NUM_ENTITIES_WITH);
				Assert::IsTrue(manager.number_of_entities_with<Toggle>() == NUM_ENTITIES_WITH);
			}
			manager.update();
			Assert::IsTrue(manager.number_of_entities_to_destroy() == 0);
			Assert::IsTrue(manager.number_of_entities_with<Toggle>() == 0);
			Assert::IsTrue(manager.number_of_reusable_entities() == NUM_ENTITIES_WITH);
		}

		TEST_METHOD(MassEntityCreationFromCopy) {
			std::vector<Entity> entities;
			EntityManager manager;
			auto e = manager.create_entity();
			e.add<Toggle>(true);
			entities.push_back(e);
			for (int i = 0; i < NUM_ENTITIES_WITH; ++i) {
				entities.push_back(manager.create_copy_of(e));
			}
			Assert::IsTrue(manager.size() == entities.size());
			Assert::IsTrue(manager.number_of_entities_with<Toggle>() == entities.size());
			Assert::IsTrue(std::all_of(entities.begin(), entities.end(), [](const Entity& e) { return e.get<Toggle>().state; }));
		}
	};

	TEST_CLASS(SystemManagerTests) {

		TEST_METHOD(AddingSystems) {
			EntityManager entity_manager;
			SystemManager system_manager(entity_manager);
			system_manager.add<ToggleSystem>();
			Assert::IsTrue(system_manager.has<ToggleSystem>());
			system_manager.add<ParToggleSystem>();
			Assert::IsTrue(system_manager.has<ParToggleSystem>());
		}
		TEST_METHOD(RemovingSystems) {
			EntityManager entity_manager;
			SystemManager system_manager(entity_manager);
			system_manager.add<ToggleSystem>();
			Assert::IsTrue(system_manager.has<ToggleSystem>());
			system_manager.remove<ToggleSystem>();
			Assert::IsFalse(system_manager.has<ToggleSystem>());
		}

		TEST_METHOD(FetchingSystems) {
			EntityManager entity_manager;
			SystemManager system_manager(entity_manager);
			system_manager.add<ToggleSystem>();
			Assert::IsTrue(system_manager.has<ToggleSystem>());
			Assert::IsTrue(system_manager.get<ToggleSystem>() != nullptr);
		}

		TEST_METHOD(UpdatingAllSystems) {
			EntityManager entity_manager;
			SystemManager system_manager(entity_manager);
			system_manager.add<ToggleSystem>();
			std::vector<Entity> entities;
			for (int i = 0; i < NUM_ENTITIES_WITH; i++) {
				auto e = entity_manager.create_entity();
				e.add<Toggle>();
			}
			for (auto entity : entities) {
				Assert::IsTrue(entity);
				Assert::IsFalse(entity.get<Toggle>().state);
			}
			system_manager.update_all(1.0);
			for (auto entity : entities) {
				Assert::IsTrue(entity);
				Assert::IsTrue(entity.get<Toggle>().state);
			}
		}

		TEST_METHOD(UpdatingSelectedSystems) {
			EntityManager entity_manager;
			SystemManager system_manager(entity_manager);
			system_manager.add<ToggleSystem>();
			std::vector<Entity> entities;
			for (int i = 0; i < NUM_ENTITIES_WITH; i++) {
				auto e = entity_manager.create_entity();
				e.add<Toggle>();
			}
			for (auto entity : entities) {
				Assert::IsTrue(entity);
				Assert::IsFalse(entity.get<Toggle>().state);
			}
			system_manager.update<ToggleSystem>(1.0);
			for (auto entity : entities) {
				Assert::IsTrue(entity);
				Assert::IsTrue(entity.get<Toggle>().state);
			}
		}
	};

	TEST_CLASS(SparseSet) {

		TEST_METHOD(Insertions) {
			internal::SparseSet integers;
			Assert::IsTrue(integers.empty());
			auto a = { 1, 2, 3, 4, 5, 6 };
			integers.insert(a.begin(), a.end());
			a = { 4, 3, 6, 2, 1, 5 };
			Assert::IsTrue(integers.contains(a.begin(), a.end()));
			a = { 10, 11, 7, 8, 9, 22 };
			Assert::IsFalse(integers.contains(a.begin(), a.end()));
		}

		TEST_METHOD(Erasure) {
			internal::SparseSet integers;
			Assert::IsTrue(integers.empty());
			auto a = { 1, 2, 3, 4, 5, 6 };
			integers.insert(a.begin(), a.end());
			a = { 4, 3, 6, 2, 1, 5 };
			Assert::IsTrue(integers.contains(a.begin(), a.end()));
			a = { 10, 11, 7, 8, 9, 22 };
			Assert::IsFalse(integers.contains(a.begin(), a.end()));
			a = { 4, 3, 1 };
			integers.erase(a.begin(), a.end());
			a = { 3, 1, 4 };
			Assert::IsFalse(integers.contains(a.begin(), a.end()));
		}

	};
}