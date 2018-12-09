#include "stdafx.h"
#include "CppUnitTest.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

#include "dummies.h"

namespace UnitTests
{		
	
	TEST_CLASS(SparseSet) {
	public:

	};

	TEST_CLASS(Entities) {
	public:
		TEST_METHOD(ID) {
			rift::EntityManager em;
			auto e = em.create_entity();
			auto f = em.create_entity();
			auto g = em.create_entity();

			// Check if the above entities have unique ids
			Assert::IsTrue(e.id().index() == 0 && e.id().version() == 1);
			Assert::IsTrue(f.id().index() == 1 && f.id().version() == 1);
			Assert::IsTrue(g.id().index() == 2 && g.id().version() == 1);

			rift::Entity::ID id(e.id());

			Assert::IsTrue(id.index() == e.id().index());
			Assert::IsTrue(id.version() == e.id().version());
		}

		TEST_METHOD(Ordering) {
			rift::EntityManager em;
			auto e = em.create_entity();
			auto f = em.create_entity();
			auto g = em.create_entity();

			Assert::IsTrue(e < f);
			Assert::IsTrue(e < g);
			Assert::IsTrue(f < g);
		}

		TEST_METHOD(Creation) {
			rift::EntityManager em;
			auto e = em.create_entity();
			auto f = em.create_entity();
			rift::Entity g;

			// e and f are valid because em created them
			Assert::IsTrue(e && f);

			// g is invalid as it wasn't spawned from an entity manager
			Assert::IsFalse(g);
		}

		TEST_METHOD(Destruction) {
			rift::EntityManager em;

			auto e = em.create_entity();
			auto f(e);

			// The pair of entities are valid
			Assert::IsTrue(e && f);

			// Entities are destroyed upon the next manager update
			// This will destroy both e and f but they're still valid
			// as their manager hasn't updated itself yet
			e.destroy();
			Assert::IsTrue(e && f);
			Assert::IsTrue(e.pending_invalidation() && f.pending_invalidation());

			// Now that their manager has updated itself, e and f are now invalid
			em.update();
			Assert::IsFalse(e);
			Assert::IsFalse(f);
		}
		
		TEST_METHOD(AddComponent) {
			rift::EntityManager em;
			auto e = em.create_entity();
			
			// The entity has no components
			Assert::IsTrue(e.component_mask() == 0);

			// The entity now has a component
			e.add<Position>();
			Assert::IsTrue(e.component_mask() != 0);
		}

		TEST_METHOD(ReplaceComponent) {
			rift::EntityManager em;
			auto e = em.create_entity();
			auto f = em.create_entity();

			// The entity has no components
			Assert::IsTrue(e.component_mask() == 0);

			// The entity now has a component
			e.add<Toggle>();
			Assert::IsTrue(e.component_mask() != 0);

			auto t = e.get<Toggle>();

			// The Toggle component given to e was off 
			Assert::IsFalse(t.on);

			e.replace<Toggle>(true);

			// The Toggle component given to e is now on
			t = e.get<Toggle>();
			Assert::IsTrue(t.on);

		}

		TEST_METHOD(RemoveComponent) {
			rift::EntityManager em;
			auto e = em.create_entity();

			// The entity has no components
			Assert::IsTrue(e.component_mask() == 0);

			// The entity now has a component
			e.add<Position>();
			Assert::IsTrue(e.component_mask() != 0);

			// The entity does not have any components
			e.remove<Position>();
			Assert::IsTrue(e.component_mask() == 0);
		}

		TEST_METHOD(HasComponent) {
			rift::EntityManager em;
			auto e = em.create_entity();

			// The entity has no components
			Assert::IsTrue(e.component_mask() == 0);

			// The entity now has a position component
			e.add<Position>();
			Assert::IsTrue(e.component_mask() != 0);
			Assert::IsTrue(e.has<Position>());
		}

		TEST_METHOD(GetComponent) {
			rift::EntityManager em;
			auto e = em.create_entity();

			// The entity has no components
			Assert::IsTrue(e.component_mask() == 0);

			// The entity now has a position component
			e.add<Toggle>(true);
			Assert::IsTrue(e.component_mask() != 0);
			Assert::IsTrue(e.has<Toggle>());
			
			// Fetch the component we added to e and check if its 'on' like we told it to be
			auto t = e.get<Toggle>();
			Assert::IsTrue(t.on);
		}

	};

	TEST_CLASS(Systems) {
	public:
		TEST_METHOD(Update) {
			rift::EntityManager em;
			auto e = em.create_entity();
			auto f = em.create_entity();
			auto g = em.create_entity();

			e.add<Toggle>();
			f.add<Toggle>();
			g.add<Toggle>();

			ToggleSystem ts;
			ts.update(em, 1.0);

			auto t = e.get<Toggle>();
			Assert::IsTrue(t.on);

			t = f.get<Toggle>();
			Assert::IsTrue(t.on);

			t = g.get<Toggle>();
			Assert::IsTrue(t.on);
		}
	};

	TEST_CLASS(EntityManager) {
	public:

		TEST_METHOD(EntityManagerNonCopyable) {
			Assert::IsFalse(std::is_copy_constructible<rift::EntityManager>());
			Assert::IsFalse(std::is_copy_assignable<rift::EntityManager>());
		}

		TEST_METHOD(EntityCreation) {
			rift::EntityManager em;
			auto e = em.create_entity();
			auto f = em.create_entity();
			auto g = em.create_entity();
			auto h(g);

			// four valid entities, one is a copy of the third
			Assert::IsTrue(em.size() == 3);
		}

		TEST_METHOD(EntityDestructionNoComponents) {
			rift::EntityManager em;
			auto e = em.create_entity();
			auto f = em.create_entity();
			auto g = em.create_entity();
			auto h(g);

			// All of the above entities should be valid
			Assert::IsTrue(e && f && g && h);

			// Ensure that g and h are not pending delete
			Assert::IsTrue(!g.pending_invalidation() && !h.pending_invalidation());

			// The number of entities to be destroyed should now be one
			// The number of reusable entities should now be zero
			// The number of managed entities is still three
			// The capacity and the number of managed entities should be the same
			// h and g should now be pending deletion
			g.destroy();
			Assert::IsTrue(em.number_of_entities_to_destroy() == 1);
			Assert::IsTrue(em.number_of_reusable_entities() == 0);
			Assert::IsTrue(em.size() == 3);
			Assert::IsTrue(em.capacity() == em.size());
			Assert::IsTrue(g.pending_invalidation() && h.pending_invalidation());

			// Ensure that multiple calls to destroy the same entity does not 
			// affect the true number of entities to destroy
			for (int i = 0; i < 10; i++) {
				g.destroy();
				Assert::IsTrue(em.number_of_entities_to_destroy() == 1);
				Assert::IsTrue(em.number_of_reusable_entities() == 0);
				Assert::IsTrue(em.size() == 3);
				Assert::IsTrue(em.capacity() == em.size());
				Assert::IsTrue(g.pending_invalidation() && h.pending_invalidation());
			}

			// Update the manager and ensure that the number of reusable entities
			// is now one and the number of entities to destroy is now zero
			em.update();
			Assert::IsTrue(em.number_of_entities_to_destroy() == 0);
			Assert::IsTrue(em.number_of_reusable_entities() == 1);
			Assert::IsTrue(em.size() == 2);
			// The number of managed entities + the number of reusable entities should be equal
			// to the manager's capacity
			Assert::IsTrue(em.capacity() == (em.size() + em.number_of_reusable_entities()));
			// Ensure that g and h are now invalid
			Assert::IsTrue(!g && !h);
		}

		TEST_METHOD(EntityDestructionWithComponents) {
			// Test system to destroy entities with a toggle component
			struct DestructionSystem : rift::System<DestructionSystem> {
				void update(rift::EntityManager &em, double dt) override {
					em.for_entities_with<Toggle>([](rift::Entity e, Toggle& toggle) {
						e.destroy();
					});
				}
			};

			rift::EntityManager em;
			auto a = em.create_entity();
			auto b = em.create_entity();
			auto c = em.create_entity();
			auto d = em.create_entity();
			auto e(d);

			a.add<Toggle>();
			b.add<Toggle>();
			c.add<Toggle>();
			d.add<Toggle>();

			// Ensure that there are five valid entities, one of which is a copy
			Assert::IsTrue(a && b && c && d && e);	
			
			// Ensure that the number of managed entities is four
			Assert::IsTrue(em.size() == 4);
			
			// Ensure that the capacity of the manager is four
			Assert::IsTrue(em.capacity() == 4);

			// Ensure that the number of entities to destroy is zero 
			Assert::IsTrue(em.number_of_entities_to_destroy() == 0);

			// Ensure that the number of reusable slots is zero
			Assert::IsTrue(em.number_of_reusable_entities() == 0);

			// Ensure that the number of entities with Toggle is four
			Assert::IsTrue(em.number_of_entities_with<Toggle>() == 4);

			// Ensure that a-d entities are not pending deletion
			Assert::IsTrue(!a.pending_invalidation() && !b.pending_invalidation() && !c.pending_invalidation() && !d.pending_invalidation() && !e.pending_invalidation());

			// Inform the EntityManager to destroy every entity with a toggle component
			DestructionSystem ds;
			ds.update(em, 1.0);

			// Ensure that the number of entities with Toggle is four
			Assert::IsTrue(em.number_of_entities_with<Toggle>() == 4);

			// Ensure the number of entities to destroy is now four
			Assert::IsTrue(em.number_of_entities_to_destroy() == 4);

			// Ensure that a-d entities are now pending deletion
			Assert::IsTrue(a.pending_invalidation() && b.pending_invalidation() && c.pending_invalidation() && d.pending_invalidation() && e.pending_invalidation());

			// Update the entity manager to ensure that destruction takes place
			em.update();

			// Ensure that each entity a-e is now invalid
			Assert::IsTrue(!a && !b && !c && !d && !e);

			// Ensure the number of managed entities is zero
			Assert::IsTrue(em.size() == 0);

			// Ensure the capacity of the manager is still four
			Assert::IsTrue(em.capacity() == 4);

			// Ensure that the number of entities to destroy is back to zero
			Assert::IsTrue(em.number_of_entities_to_destroy() == 0);

			// Ensure the number of reusable entities is now 4
			Assert::IsTrue(em.number_of_reusable_entities() == 4);

			// Ensure the number of entities with toggle components is zero
			Assert::IsTrue(em.number_of_entities_with<Toggle>() == 0);

			// Create a new entity and ensure that it was one of the EntityManager's reusable ones
			auto f = em.create_entity();
			Assert::IsTrue(em.number_of_reusable_entities() == 3);

			// Add a toggle component and ensure that the number of entities with Toggle is now one
			f.add<Toggle>(true);
			Assert::IsTrue(em.number_of_entities_with<Toggle>() == 1);
		}

		TEST_METHOD(SimulatedUsage) {
			rift::EntityManager em;
			{
				for (int i = 0; i < 100; i++)
					em.create_entity();

				Assert::IsTrue(em.size() == 100);
			}
			{
				em.clear();

				Assert::IsTrue(em.size() == 0);

				std::vector<rift::Entity> entities;
				for (int i = 0; i < 100; i++)
					entities.push_back(em.create_entity());

				for (std::size_t i = 0; i < entities.size(); i++)
				{
					entities[i].add<Position>(10.0f, 10.0f);
					entities[i].add<Direction>(10.0f, 10.0f);
					if (i % 2 == 0) {
						entities[i].add<Toggle>();
					}
				}

				Assert::IsTrue(em.number_of_entities_with<Position, Direction>() == 100);
				Assert::IsTrue(em.number_of_entities_with<Toggle>() == 50);

			}
			{
				for (auto i = 0; i < 100; i++) {
					em.for_entities_with<Position, Direction>([](rift::Entity e, Position& pos, Direction& dir) {
						pos.x += dir.x;
						pos.y += dir.y;
					});
				}
			}
			{
				em.for_entities_with<Position>([](rift::Entity e, Position& pos) {
					if (e.has<Toggle>())
						e.remove<Toggle>();
				});

				Assert::IsTrue(em.number_of_entities_with<Toggle>() == 0);
			}
			{

				em.for_entities_with<Direction>([](rift::Entity e, Direction& dir) {
					e.add<Toggle>();
				});
				Assert::IsTrue(em.number_of_entities_with<Toggle>() == 100);
			}
			{
				em.for_entities_with<Position, Direction, Toggle>([](rift::Entity e, Position& p, Direction& d, Toggle& t) {
					e.destroy();
				});

				Assert::IsTrue(em.number_of_entities_with<Position>() == 100);
				Assert::IsTrue(em.number_of_entities_with<Direction>() == 100);
				Assert::IsTrue(em.number_of_entities_with<Toggle>() == 100);
				Assert::IsTrue(em.number_of_entities_to_destroy() == 100);

				em.update();

				Assert::IsTrue(em.number_of_reusable_entities() == 100);
				Assert::IsTrue(em.number_of_entities_with<Position>() == 0);
				Assert::IsTrue(em.number_of_entities_with<Direction>() == 0);
				Assert::IsTrue(em.number_of_entities_with<Toggle>() == 0);
			}
		}

		TEST_METHOD(CountingEntitiesWithComponents) {
			rift::EntityManager em;
			auto a = em.create_entity();
			auto b = em.create_entity();
			auto c = em.create_entity();
			auto d = em.create_entity();

			a.add<Toggle>();
			b.add<Toggle>();
			c.add<Toggle>();

			// Ensure that there are three entities with toggle components
			Assert::IsTrue(em.number_of_entities_with<Toggle>() == 3);

			ToggleSystem ts;
			ts.update(em, 1.0);

			// Ensure that there are still three entities with toggle compnents
			// after the system update
			Assert::IsTrue(em.number_of_entities_with<Toggle>() == 3);

			// Let d have a toggle component
			d.add<Toggle>();

			// Ensure there are four entities with toggle compnents
			Assert::IsTrue(em.number_of_entities_with<Toggle>() == 4);

			// Remove the toggle component from b
			b.remove<Toggle>();

			// Ensure there are only three entities with toggle components
			Assert::IsTrue(em.number_of_entities_with<Toggle>() == 3);
		}
	};
	
	TEST_CLASS(SystemManager) {
	public:

		TEST_METHOD(SystemManagerNonCopyable) {
			Assert::IsFalse(std::is_copy_constructible<rift::SystemManager>());
			Assert::IsFalse(std::is_copy_assignable<rift::SystemManager>());
		}

		TEST_METHOD(AddSystem) {
			rift::EntityManager em;
			rift::SystemManager sm(em);

			// Ensure the system manager doesn't manage a MovementSystem
			Assert::IsFalse(sm.has<MovementSystem>());

			sm.add<MovementSystem>();
			
			// Ensure the system manage manages a movement system
			Assert::IsTrue(sm.has<MovementSystem>());
		}

		TEST_METHOD(RemoveSystem) {
			rift::EntityManager em;
			rift::SystemManager sm(em);

			// Ensure the system manager doesn't manage a MovementSystem
			Assert::IsFalse(sm.has<MovementSystem>());

			sm.add<MovementSystem>();

			// Ensure the system manager manages a movement system
			Assert::IsTrue(sm.has<MovementSystem>());

			sm.remove<MovementSystem>();

			// Ensure the system manager no longer manages a movement system
			Assert::IsFalse(sm.has<MovementSystem>());
		}

		TEST_METHOD(GetSystem) {
			rift::EntityManager em;
			rift::SystemManager sm(em);

			// Ensure the system manager doesn't manage a MovementSystem
			Assert::IsFalse(sm.has<MovementSystem>());

			sm.add<MovementSystem>();

			// Ensure the system manage manages a movement system
			Assert::IsTrue(sm.has<MovementSystem>());

			// Ensure the MovementSystem was created correctly
			Assert::IsTrue(sm.get<MovementSystem>() != nullptr);
		}

		TEST_METHOD(UpdateAllSystems) {

			// Test system to destroy entities with a toggle component
			struct DestructionSystem : rift::System<DestructionSystem> {
				void update(rift::EntityManager &em, double dt) override {
					em.for_entities_with<Toggle>([](rift::Entity e, Toggle& toggle) {
						e.destroy();
					});
				}
			};

			rift::EntityManager em;
			rift::SystemManager sm(em);

			sm.add<ToggleSystem>();
			sm.add<DestructionSystem>();

			auto a = em.create_entity();
			auto b = em.create_entity();
			auto c = em.create_entity();
			auto d = em.create_entity();

			// Ensure the number of managed entities is four
			Assert::IsTrue(em.size() == 4);

			// Ensure the number of reusable entities is zero
			Assert::IsTrue(em.number_of_reusable_entities() == 0);

			// Ensure that there are zero entities with toggle components before all systems update
			Assert::IsTrue(em.number_of_entities_with<Toggle>() == 0);

			a.add<Toggle>();
			b.add<Toggle>();
			c.add<Toggle>();
			d.add<Toggle>();

			// Ensure that there are four entities with toggle components before all systems update
			Assert::IsTrue(em.number_of_entities_with<Toggle>() == 4);

			// Update all systems
			sm.update(1.0);

			// Ensure there are zero entities with toggle components
			Assert::IsTrue(em.number_of_entities_with<Toggle>() == 0);

			// Ensure the number of reusable entities is now four
			Assert::IsTrue(em.number_of_reusable_entities() == 4);

			// Ensure the number of managed entities is zero
			Assert::IsTrue(em.size() == 0);

		}

		TEST_METHOD(UpdateSelectedSystems) {
			// Test system to destroy entities with a toggle component
			struct DestructionSystem : rift::System<DestructionSystem> {
				void update(rift::EntityManager &em, double dt) override {
					em.for_entities_with<Toggle>([](rift::Entity e, Toggle& toggle) {
						e.destroy();
					});
				}
			};

			rift::EntityManager em;
			rift::SystemManager sm(em);

			sm.add<ToggleSystem>();
			sm.add<DestructionSystem>();

			auto a = em.create_entity();
			auto b = em.create_entity();
			auto c = em.create_entity();
			auto d = em.create_entity();

			// Ensure the number of managed entities is four
			Assert::IsTrue(em.size() == 4);

			// Ensure the number of reusable entities is zero
			Assert::IsTrue(em.number_of_reusable_entities() == 0);

			// Ensure that there are zero entities with toggle components before all systems update
			Assert::IsTrue(em.number_of_entities_with<Toggle>() == 0);

			a.add<Toggle>();
			b.add<Toggle>();
			c.add<Toggle>();
			d.add<Toggle>();

			// Ensure that there are four entities with toggle components before all systems update
			Assert::IsTrue(em.number_of_entities_with<Toggle>() == 4);

			// Update all systems
			sm.ordered_update<DestructionSystem, ToggleSystem>(1.0);

			// Ensure there are zero entities with toggle components
			Assert::IsTrue(em.number_of_entities_with<Toggle>() == 0);

			// Ensure the number of reusable entities is now four
			Assert::IsTrue(em.number_of_reusable_entities() == 4);

			// Ensure the number of managed entities is zero
			Assert::IsTrue(em.size() == 0);
		}

		TEST_METHOD(UpdateSystemsUpdateComponents) {
			rift::EntityManager entities;
			auto a = entities.create_entity();
			auto b = entities.create_entity();
			auto c = entities.create_entity();
			auto d = entities.create_entity();

			rift::SystemManager systems(entities);

			systems.add<MovementSystem>();

			a.add<Position>(0, 0);
			b.add<Position>(0, 0);
			c.add<Position>(0, 0);
			d.add<Position>(0, 0);

			a.add<Direction>(1, 0);
			b.add<Direction>(1, 0);
			c.add<Direction>(1, 0);
			d.add<Direction>(1, 0);

			systems.ordered_update<MovementSystem>(1.0);

			auto pos = a.get<Position>();
			Assert::IsTrue(pos.x == 1 && pos.y == 0);

			pos = b.get<Position>();
			Assert::IsTrue(pos.x == 1 && pos.y == 0);

			pos = c.get<Position>();
			Assert::IsTrue(pos.x == 1 && pos.y == 0);

			pos = d.get<Position>();
			Assert::IsTrue(pos.x == 1 && pos.y == 0);

			systems.add<ToggleSystem>();

			a.add<Toggle>();
			b.add<Toggle>();
			c.add<Toggle>();
			d.add<Toggle>();

			systems.ordered_update<ToggleSystem>(1.0);

			auto toggle = a.get<Toggle>();
			Assert::IsTrue(toggle.on);

			toggle = b.get<Toggle>();
			Assert::IsTrue(toggle.on);

			toggle = c.get<Toggle>();
			Assert::IsTrue(toggle.on);

			toggle = d.get<Toggle>();
			Assert::IsTrue(toggle.on);
		}

	};

}