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
		}

		TEST_METHOD(ComponentSignature) {
			auto signature = rift::signature_for<Position, Direction>();
			Assert::IsTrue(signature == 3);
		}

	};

	TEST_CLASS(Containers) {

		TEST_METHOD(Insertion) {
			rift::Cache<int> integer_cache;
			int x = 3;
			integer_cache.insert(0, &x);

			Assert::IsTrue(integer_cache.test(0));
			Assert::IsTrue((*(static_cast<int *>(integer_cache.get(0))) == x));
		}

		TEST_METHOD(Search) {
			rift::Cache<int> integer_cache;
			int x = 3;
			integer_cache.insert(0, &x);

			Assert::IsTrue(integer_cache.test(0));
			Assert::IsTrue((*(static_cast<int *>(integer_cache.get(0))) == x));
			Assert::IsFalse(integer_cache.test(10));
		}

		TEST_METHOD(Removal) {
			rift::Cache<int> integer_cache;
			int x = 3;
			integer_cache.insert(0, &x);

			Assert::IsTrue(integer_cache.test(0));
			Assert::IsTrue((*(static_cast<int *>(integer_cache.get(0))) == x));
			
			integer_cache.erase(0);

			Assert::IsFalse(integer_cache.test(0));
		}
		TEST_METHOD(Get) {
			rift::Cache<int> integer_cache;
			int x = 3;
			integer_cache.insert(0, &x);

			Assert::IsTrue(integer_cache.test(0));
			Assert::IsTrue((*(static_cast<int *>(integer_cache.get(0))) == x));
		}

	};

	TEST_CLASS(Entity) {
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

		TEST_METHOD(EntityManagerComponentPoolCreation) {
			rift::EntityManager em;
			auto a = em.create_entity();
			auto b = em.create_entity();

			a.add<Position>();
			b.add<Position>();

			// There should now be a cache of Position components
			// for the two entities that have it
			Assert::IsTrue(em.has_component_cache_for<Position>());
			// The size of the cache should be 2
			Assert::IsTrue(em.component_cache_size_for<Position>() == 2);

			// The component cache for Position should now be size 1
			a.remove<Position>();
			Assert::IsTrue(em.component_cache_size_for<Position>() == 1);
		}

		TEST_METHOD(EntityManagerEntitiesWith) {
			rift::EntityManager em;
			auto a = em.create_entity();
			auto b = em.create_entity();

			a.add<Position>();
			b.add<Position>();

			// This will cache the two entities, a and b into a 
			// search cache to speed up subsequent lookups
			em.entities_with<Position>([](rift::Entity e) {});

			// The size of the search cache must be 2
			Assert::IsTrue(em.entity_cache_size_for<Position>() == 2);

			a.destroy();
			// The size of the search cache must now be 1 since a is dead
			Assert::IsTrue(em.entity_cache_size_for<Position>() == 1);
		}

	};

	TEST_CLASS(System) {

	};
}