#include "stdafx.h"
#include "CppUnitTest.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

#include "dummies.h"

namespace UnitTests
{		
	TEST_CLASS(Components)
	{
	public:
		
		TEST_METHOD(ComponentFamilies)
		{
			Assert::IsTrue(Position::family() != Direction::family());
			Assert::IsTrue(Position::family() != Toggle::family());
			Assert::IsTrue(Direction::family() != Toggle::family());
		}


		TEST_METHOD(ComponentSignature) {
			Assert::IsTrue(rift::signature_for<Position, Direction>() == rift::signature_for<Direction, Position>());
			Assert::IsTrue(rift::signature_for<Position, Toggle>() == rift::signature_for<Toggle, Position>());
			Assert::IsTrue(rift::signature_for<Direction, Toggle>() == rift::signature_for<Toggle, Direction>());
			Assert::IsTrue(rift::signature_for<Position, Direction, Toggle>() == rift::signature_for<Position, Toggle, Direction>());
			Assert::IsTrue(rift::signature_for<Position, Direction, Toggle>() == rift::signature_for<Toggle, Direction, Position>());
		}

	};

	TEST_CLASS(Containers) {
	public:
		TEST_METHOD(Insertion) {
			rift::Cache<int> integer_cache;
			int x = 3;
			integer_cache.insert(0, &x);

			Assert::IsTrue(integer_cache.exists(0));
			Assert::IsTrue((*(static_cast<int *>(integer_cache.get(0))) == x));
			
			rift::EntityManager em;

			auto a = em.create_entity();
			auto b = em.create_entity();

			rift::Cache<rift::Entity> entity_cache;
			entity_cache.insert(a.id().index(), &a);
			entity_cache.insert(b.id().index(), &b);

			// There are two entities in the cache
			Assert::IsTrue(entity_cache.size() == 2);

			// Check if the inserted entities are correct
			Assert::IsTrue(a == *(static_cast<rift::Entity *>(entity_cache.get(a.id().index()))));
			Assert::IsTrue(b == *(static_cast<rift::Entity *>(entity_cache.get(b.id().index()))));

		}

		TEST_METHOD(Search) {
			rift::Cache<int> integer_cache;
			int x = 3;
			integer_cache.insert(0, &x);

			Assert::IsTrue(integer_cache.exists(0));
			Assert::IsTrue((*(static_cast<int *>(integer_cache.get(0))) == x));
			Assert::IsFalse(integer_cache.exists(10));


			rift::EntityManager em;

			auto a = em.create_entity();
			auto b = em.create_entity();

			rift::Cache<rift::Entity> entity_cache;
			entity_cache.insert(a.id().index(), &a);
			entity_cache.insert(b.id().index(), &b);

			// There are two entities in the cache
			Assert::IsTrue(entity_cache.size() == 2);

			// Check if there are inserted entities at the indices for a and b
			Assert::IsTrue(entity_cache.exists(a.id().index()));
			Assert::IsTrue(entity_cache.exists(b.id().index()));

		}

		TEST_METHOD(Removal) {
			rift::Cache<int> integer_cache;
			int x = 3;
			integer_cache.insert(0, &x);

			Assert::IsTrue(integer_cache.exists(0));
			Assert::IsTrue((*(static_cast<int *>(integer_cache.get(0))) == x));
			
			integer_cache.erase(0);

			Assert::IsFalse(integer_cache.exists(0));

			rift::EntityManager em;

			auto a = em.create_entity();
			auto b = em.create_entity();

			rift::Cache<rift::Entity> entity_cache;
			entity_cache.insert(a.id().index(), &a);
			entity_cache.insert(b.id().index(), &b);

			// There are two entities in the cache
			Assert::IsTrue(entity_cache.size() == 2);

			// Check if there are inserted entities at the indices for a and b
			Assert::IsTrue(entity_cache.exists(a.id().index()));
			Assert::IsTrue(entity_cache.exists(b.id().index()));

			entity_cache.erase(a.id().index());
			entity_cache.erase(b.id().index());

			Assert::IsFalse(entity_cache.exists(a.id().index()));
			Assert::IsFalse(entity_cache.exists(b.id().index()));

		}
		TEST_METHOD(Get) {
			rift::Cache<int> integer_cache;
			int x = 3;
			integer_cache.insert(0, &x);

			Assert::IsTrue(integer_cache.exists(0));
			Assert::IsTrue((*(static_cast<int *>(integer_cache.get(0))) == x));
		}

	};

	TEST_CLASS(Entity) {
	public:
		TEST_METHOD(EntityID) {
			auto e = rift::Entity::ID(10, 11);
			auto f(e);
			auto g = rift::Entity::ID(11, 11);

			// Test the index + version functions
			Assert::IsTrue(e.index() == 10);
			Assert::IsTrue(e.version() == 11);

			// Test the equality/inequality functions
			Assert::IsTrue(e == f);
			Assert::IsTrue(e != g);
			Assert::IsTrue(e < g);
		}

		TEST_METHOD(EntityCreation) {
			rift::EntityManager em;
			rift::Entity a = em.create_entity();
			rift::Entity b(a);
			rift::Entity c;
			Assert::IsTrue(a);
			Assert::IsTrue(b);
			Assert::IsFalse(c);
		}

		TEST_METHOD(EntityDestruction) {
			rift::EntityManager em;
			rift::Entity a = em.create_entity();
			rift::Entity b(a);
			rift::Entity c;

			Assert::IsTrue(a);
			Assert::IsTrue(b);
			Assert::IsFalse(c);

			// Destroying a will destroy b
			a.destroy();
			Assert::IsFalse(a);
			Assert::IsFalse(b);
		}

		TEST_METHOD(EntityComponentMask){
			rift::EntityManager em;
			rift::Entity a = em.create_entity();
			
			// Add position, the component mask for 'a'
			// should match the signature for the Position type
			a.add<Position>();
			Assert::IsTrue(a.component_mask() == rift::signature_for<Position>());
			
			// Add direction, the component mask for 'a'
			// should now match the signature for Position & Direction
			a.add<Direction>();

			Assert::IsTrue(a.component_mask() == rift::signature_for<Position, Direction>());

			// Remove position, the component mask for 'a'
			// should now only match the signature for Direction
			a.remove<Position>();
			Assert::IsTrue(a.component_mask() == rift::signature_for<Direction>());

		}
	
		TEST_METHOD(EntityComponentAddition) {
			rift::EntityManager em;
			auto e = em.create_entity();
			// The entity should now have a Position component
			e.add<Position>();
			Assert::IsTrue(e.has<Position>());
			// The entity should not have a Direction component
			Assert::IsTrue(!e.has<Direction>());
			// The entity should now have a Direction component
			e.add<Direction>();
			Assert::IsTrue(e.has<Direction>());
		}

		TEST_METHOD(EntityComponentRemoval) {
			rift::EntityManager em;
			auto e = em.create_entity();
			// The entity should now have a Position component
			e.add<Position>();
			Assert::IsTrue(e.has<Position>());
			// The entity should not have a Direction component
			Assert::IsTrue(!e.has<Direction>());
			// The entity should now have a Direction component
			e.add<Direction>();
			Assert::IsTrue(e.has<Direction>());

			// The entity should not have a Direction component
			e.remove<Direction>();
			Assert::IsTrue(!e.has<Direction>());
		}

		TEST_METHOD(EntityComponentGet) {
			rift::EntityManager em;
			auto e = em.create_entity();
			// The entity should now have a Position component
			e.add<Position>(1, 1);
			Assert::IsTrue(e.has<Position>());

			// Get the position component just added to the entity
			// and check if the data for it is the same as what we
			// pass into the add() function
			Position p = e.get<Position>();
			Assert::IsTrue(p.x == 1 && p.y == 1);
		}

		TEST_METHOD(EntityComparisons) {
			rift::EntityManager em;
			auto a = em.create_entity();
			auto b = em.create_entity();
			auto c = em.create_entity();
			auto d(a);

			// a is 'less' than b and c meaning a is ordered first
			Assert::IsTrue(a < b && a < c);
			// b is in between a and c meaning its ordered second
			Assert::IsTrue(a < b && b < c);
			// c is ordered last because its index the greatest
			Assert::IsTrue(c > b && c > a);
			// a and d are the same entity, they have the same index
			// and the same manager
			Assert::IsTrue(a == d);
		}

		TEST_METHOD(EntityManagerSize){
			rift::EntityManager em;
			auto a = em.create_entity();
			auto b = em.create_entity();
			auto c = em.create_entity();
			auto d = em.create_entity();
			auto e = em.create_entity();

			// The manager created 5 entities so its 
			// should possess 5 valid entity masks
			Assert::IsTrue(em.size() == 5);
			
			c.destroy();
			// Now that c is destroyed, the manager should
			// only have 4 entity masks, the other free for reuse
			Assert::IsTrue(em.size() == 4);

		}

		TEST_METHOD(EntityManagerCapacity) {
			rift::EntityManager em;
			auto a = em.create_entity();
			auto b = em.create_entity();
			auto c = em.create_entity();
			auto d = em.create_entity();
			auto e = em.create_entity();

			// The manager created 5 entities so its 
			// should possess 5 valid entity masks
			Assert::IsTrue(em.size() == 5);

			c.destroy();
			// Now that c is destroyed, the manager should
			// only have 4 entity masks, the other free for reuse
			Assert::IsTrue(em.size() == 4);

			e.destroy();
			// Now that e is destroyed, the manager should only have
			// 3 entity masks, the others 2 are now free for resuse
			Assert::IsTrue(em.size() == 3);

			// Assert that there are 2 reusable slots available
			Assert::IsTrue(em.capacity() - em.size() == 2);
		}

		TEST_METHOD(EntityManagerCountEntitiesWith) {
			rift::EntityManager em;
			auto a = em.create_entity();
			auto b = em.create_entity();
			auto c = em.create_entity();

			a.add<Position>();
			b.add<Direction>();
			c.add<Position>();
			c.add<Direction>();

			// There should be 2 entities with Position, a and c
			Assert::IsTrue(em.count_entities_with<Position>() == 2);
			// There should be 2 entities with Direction, b and c
			Assert::IsTrue(em.count_entities_with<Direction>() == 2);
			// There should be only 1 entity with Position & Direction
			Assert::IsTrue(em.count_entities_with<Position, Direction>() == 1);
		
			// a lost its Position component but gained a Direction component
			// b gained a Position component
			a.remove<Position>();
			a.add<Direction>();
			b.add<Position>();
			// There should now be 1 entity with Position, b and c
			Assert::IsTrue(em.count_entities_with<Position>() == 2);
			// There should now be 3 entities with Direction, a b c
			Assert::IsTrue(em.count_entities_with<Direction>() == 3);
			// There should be 2 entities with Position & Direction, b and c
			Assert::IsTrue(em.count_entities_with<Position, Direction>() == 2);
		}
		
		TEST_METHOD(EntityManagerEntitiesWithComponentUpdate) {

			rift::EntityManager em;
			auto a = em.create_entity();
			auto b = em.create_entity();
			auto c = em.create_entity();

			a.add<Toggle>();
			b.add<Toggle>();

			em.entities_with<Toggle>([a, b](rift::Entity e) {
				Assert::IsTrue(e.id() == a.id() || e.id() == b.id());
				auto& t = e.get<Toggle>();
				t.on = true;
			});

			auto t = a.get<Toggle>();
			Assert::IsTrue(t.on);

			t = b.get<Toggle>();
			Assert::IsTrue(t.on);
		}
		
	};
	
	TEST_CLASS(System) {
	public:
		TEST_METHOD(SingleSystemIteration) {
			rift::EntityManager em;
			for (int i = 0; i < 20; i++) {
				auto e = em.create_entity();
				e.add<Toggle>();
			}

			// Test stand alone system update
			ToggleSystem ts;
			ts.update(em, 1.0);
			em.entities_with<Toggle>([](rift::Entity e) {
				auto& t = e.get<Toggle>();
				Assert::IsTrue(t.on);
				t.on = false;
			});

			// Test system manager update
			rift::SystemManager sm;
			sm.add<ToggleSystem>();
			sm.update(em, 1.0);
			em.entities_with<Toggle>([](rift::Entity e) {
				auto& t = e.get<Toggle>();
				Assert::IsTrue(t.on);
			});
		}
		
		TEST_METHOD(MultiSystemIteration) {
			rift::EntityManager em;
			for (int i = 0; i < 20; i++) {
				auto e = em.create_entity();
				e.add<Toggle>();
				e.add<Position>(1.0f, 1.0f);
				e.add<Direction>(1.0f, 1.0f);
			}

			// Two single system update
			ToggleSystem ts;
			MovementSystem ms;
			// Update the toggle system
			ts.update(em, 1.0);
			em.entities_with<Toggle>([](rift::Entity e) {
				auto& t = e.get<Toggle>();
				Assert::IsTrue(t.on);
				t.on = false;
			});
			ms.update(em, 1.0);
			em.entities_with<Direction, Position>([](rift::Entity e) {
				auto& p = e.get<Position>();
				Assert::IsTrue(p.x == 2.0f && p.y == 2.0f);
			});

			// SystemManager multisystem update
			rift::SystemManager sm;
			sm.add<ToggleSystem>();
			sm.add<MovementSystem>();
			sm.update(em, 1.0);
			em.entities_with<Toggle>([](rift::Entity e) {
				auto& t = e.get<Toggle>();
				Assert::IsTrue(t.on);
			});
			em.entities_with<Direction, Position>([](rift::Entity e) {
				auto& p = e.get<Position>();
				Assert::IsTrue(p.x == 3.0f && p.y == 3.0f);
			});
		}


		TEST_METHOD(SystemManagerSystemInsertion) {
			rift::SystemManager sm;
			sm.add<ToggleSystem>();
			Assert::IsTrue(sm.has<ToggleSystem>());
		}
		TEST_METHOD(SystemManagerSystemRemoval) {
			rift::SystemManager sm;
			sm.add<ToggleSystem>();
			Assert::IsTrue(sm.has<ToggleSystem>());
			sm.remove<ToggleSystem>();
			Assert::IsTrue(!sm.has<ToggleSystem>());
		}
		TEST_METHOD(SystemManagerSystemCheck) {
			rift::SystemManager sm;
			Assert::IsTrue(!sm.has<ToggleSystem>());
			sm.add<ToggleSystem>();
			Assert::IsTrue(sm.has<ToggleSystem>());
		}
	};
}