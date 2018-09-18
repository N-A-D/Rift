#include "stdafx.h"
#include "CppUnitTest.h"

#include "systems.h"
#include "components.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace Test
{		
	TEST_CLASS(Component)
	{
	public:
		
		TEST_METHOD(mask_for)
		{
			Assert::IsTrue(rift::util::mask_for<>() == 0);
			Assert::IsTrue(rift::util::mask_for<Position>() == 1);
			Assert::IsTrue(rift::util::mask_for<Velocity>() == 2);
			Assert::IsTrue(rift::util::mask_for<Velocity, Position>() == 3);
			Assert::IsTrue(rift::util::mask_for<Position, Velocity>() == 3);
		}

	};

	TEST_CLASS(Pool) {
	public:
		  
		TEST_METHOD(allocate) {
			rift::Pool<Position> p;
			rift::Pool<Position> p1(10);
			p.allocate(10);
			Assert::IsTrue(p.size() == p1.size());
			rift::Pool<Position> p2;
			for (std::size_t i = 0; i < 10; i++) {
				Assert::IsFalse(p2.size() == p.size());
				p2.allocate(1);
			}
			Assert::IsTrue(p2.size() == p.size());
		}

		TEST_METHOD(at) {
			rift::Pool<Position> p1(10);
			p1.at(3) = Position(3, 3);
			Assert::IsTrue(p1.at(3).x == 3 && p1.at(3).y == 3);
		}

	};

	TEST_CLASS(Entity) {
	public:
		TEST_METHOD(Creation) {
			rift::EntityManager em;
			auto a = em.create_entity();
			auto b = em.create_entity();
			auto c = em.create_entity();
			auto d = em.create_entity();
			rift::Entity e;
			rift::Entity f(a);
			Assert::IsFalse(e);
			Assert::IsTrue(a);
			Assert::IsTrue(b);
			Assert::IsTrue(c);
			Assert::IsTrue(d);
			Assert::IsTrue(f);
		}

		TEST_METHOD(Destruction) {
			rift::EntityManager em;
			auto a = em.create_entity();
			auto c = a;
			Assert::IsTrue(a && c);
			a.destroy();
			Assert::IsFalse(a);
			Assert::IsFalse(c);
		}

		TEST_METHOD(id_fetch) {
			rift::EntityManager em;
			auto a = em.create_entity();
			auto b(a);
			Assert::IsTrue(a.id() == b.id());
			Assert::IsTrue(a.id().index() == 0);
			Assert::IsTrue(a.id().version() == 1);
		}

		TEST_METHOD(id_reuse) {
			rift::EntityManager em;
			auto a = em.create_entity();
			auto b(a);
			a.destroy();
			Assert::IsFalse(a);
			Assert::IsFalse(b);
			auto c = em.create_entity();
			Assert::IsTrue(a.id().index() == c.id().index());
			Assert::IsTrue(c.id().version() == 2);
		}

		TEST_METHOD(addition) {
			rift::EntityManager em;
			auto a = em.create_entity();
			a.add<Position>(10, 10);
			a.add<Velocity>(0, 0);
			a.add<Direction>(1, 0);
			Assert::IsTrue(a.has<Position>() && a.has<Velocity>() && a.has<Direction>());
			Assert::IsTrue(a.component_mask() == rift::util::mask_for<Position, Direction, Velocity>());
		}

		TEST_METHOD(removal) {
			rift::EntityManager em;
			auto a = em.create_entity();
			a.add<Position>(10, 10);
			a.add<Velocity>(0, 0);
			a.add<Direction>(1, 0);
			Assert::IsTrue(a.has<Position>() && a.has<Velocity>() && a.has<Direction>() && 
				           a.component_mask() == rift::util::mask_for<Position, Direction, Velocity>());
			a.remove<Velocity>();
			Assert::IsFalse(a.has<Velocity>());
			Assert::IsTrue(a.has<Position>() && a.has<Direction>() &&
						   a.component_mask() == rift::util::mask_for<Position, Direction>());
		}

		TEST_METHOD(get_component) {
			rift::EntityManager em;
			auto a = em.create_entity();
			a.add<Position>(10, 10);
			a.add<Velocity>(0, 0);
			a.add<Direction>(1, 0);
			Assert::IsTrue(a.has<Position>() && a.has<Velocity>() && a.has<Direction>() &&
				a.component_mask() == rift::util::mask_for<Position, Direction, Velocity>());

			Position p = a.get<Position>();
			Assert::IsTrue(p.x == 10 && p.y == 10);
		}

	};

	TEST_CLASS(EntityManager) {
	public:
		TEST_METHOD(creation) {
			rift::EntityManager em;
			auto entity = em.create_entity();
			auto id = entity.id();
			for (int i = 0; i < 1000; i++) {
				auto e = em.create_entity();
				Assert::IsTrue(e);
				Assert::IsTrue(e.id() > id);
				id = e.id();
			}
		}

		TEST_METHOD(destruction) {
			rift::EntityManager em;
			auto entity = em.create_entity();
			std::size_t index = entity.id().index();
			std::size_t version = entity.id().version();
			entity.destroy();
			for (int i = 0; i < 1000; i++) {
				auto e = em.create_entity();
				Assert::IsTrue(e);
				Assert::IsTrue(e.id().index() == index);
				Assert::IsTrue(e.id().version() > version);
				version = e.id().version();
				e.destroy();
			}
		}

		TEST_METHOD(signature_counting) {
			rift::EntityManager em;
			auto a = em.create_entity();
			auto b = em.create_entity();
			auto c = em.create_entity();
			auto d = em.create_entity();
			auto e = em.create_entity();
			auto f = em.create_entity();

			a.add<Position>();
			b.add<Position>();
			c.add<Position>();
			e.add<Position>();

			c.add<Velocity>();
			d.add<Velocity>();
			e.add<Velocity>();
			
			d.add<Direction>();
			e.add<Direction>();

			Assert::IsTrue(em.count_entities_with<>() == 1);
			Assert::IsTrue(em.count_entities_with<Position>() == 4);
			Assert::IsTrue(em.count_entities_with<Velocity>() == 3);
			Assert::IsTrue(em.count_entities_with<Direction>() == 2);
			Assert::IsTrue(em.count_entities_with<Position, Velocity>() == 2);
			Assert::IsTrue(em.count_entities_with<Direction, Velocity>() == 2);
			Assert::IsTrue(em.count_entities_with<Direction, Position>() == 1);
			Assert::IsTrue(em.count_entities_with<Position, Velocity, Direction>() == 1);
		}

		TEST_METHOD(apply_onto_entities_with) {
			rift::EntityManager em;
			auto a = em.create_entity();
			auto b = em.create_entity();
			auto c = em.create_entity();
			auto d = em.create_entity();
			auto e = em.create_entity();
			auto f = em.create_entity();

			a.add<Position>(10, 10);
			b.add<Position>(10, 10);
			c.add<Position>(10, 10);
			e.add<Position>(10, 10);

			c.add<Velocity>(11, 11);
			d.add<Velocity>(11, 11);
			e.add<Velocity>(11, 11);

			d.add<Direction>(12, 12);
			e.add<Direction>(12, 12);

			std::size_t count = 0;
			em.entities_with<>([&count](const rift::Entity& e) { Assert::IsTrue(e.component_mask() == 0); ++count; });
			
			Assert::IsTrue(count == 1);
			count = 0;

			em.entities_with<Position>([&count](const rift::Entity& e) { 
				Position p = e.get<Position>();
				Assert::IsTrue(p.x == 10 && p.y == 10);
				++count;
			});

			Assert::IsTrue(count == 4);
			count = 0;

			em.entities_with<Velocity>([&count](const rift::Entity& e) {
				Velocity v = e.get<Velocity>();
				Assert::IsTrue(v.x == 11 && v.y == 11);
				++count;
			});

			Assert::IsTrue(count == 3);
			count = 0;

			em.entities_with<Direction>([&count](const rift::Entity& e) {
				Direction v = e.get<Direction>();
				Assert::IsTrue(v.x == 12 && v.y == 12);
				++count;
			});

			Assert::IsTrue(count == 2);
			count = 0;

			em.entities_with<Position, Velocity>([&count](const rift::Entity& e) {
				Position p = e.get<Position>();
				Velocity v = e.get<Velocity>();
				Assert::IsTrue(p.x == 10 && p.y == 10);
				Assert::IsTrue(v.x == 11 && v.y == 11);
				++count;
			});

			Assert::IsTrue(count == 2);
			count = 0;

			em.entities_with<Position, Direction>([&count](const rift::Entity& e) {
				Position p = e.get<Position>();
				Direction d = e.get< Direction>();
				Assert::IsTrue(p.x == 10 && p.y == 10);
				Assert::IsTrue(d.x == 12 && d.y == 12);
				++count;
			});

			Assert::IsTrue(count == 1);
			count = 0;

			em.entities_with<Velocity, Direction>([&count](const rift::Entity& e) {
				Direction d = e.get<Direction>();
				Velocity v = e.get< Velocity>();
				Assert::IsTrue(d.x == 12 && d.y == 12);
				Assert::IsTrue(v.x == 11 && v.y == 11);
				++count;
			});

			Assert::IsTrue(count == 2);
			count = 0;

			em.entities_with<Position, Velocity, Direction>([&count](const rift::Entity& e) {
				Direction d = e.get<Direction>();
				Velocity v = e.get< Velocity>();
				Position p = e.get<Position>();
				Assert::IsTrue(p.x == 10 && p.y == 10);
				Assert::IsTrue(d.x == 12 && d.y == 12);
				Assert::IsTrue(v.x == 11 && v.y == 11);
				++count;
			});

			Assert::IsTrue(count == 1);
		}
	};

	TEST_CLASS(System) {
	public:
		TEST_METHOD(update) {
			rift::EntityManager em;
			Movement ms;
			auto a = em.create_entity();

			a.add<Position>(10, 10);
			a.add<Velocity>(11, 11);

			ms.update(em, 1.0);

			Position p = a.get<Position>();
			Assert::IsTrue(p.x == 21 && p.y == 21);
		}
	};

	TEST_CLASS(SystemManager) {
	public:
		TEST_METHOD(manage) {
			rift::SystemManager sm;
			sm.add<Movement>();
			Assert::IsTrue(sm.has<Movement>());
		}

		TEST_METHOD(release) {
			rift::SystemManager sm;
			sm.add<Movement>();
			Assert::IsTrue(sm.has<Movement>());
			sm.remove<Movement>();
			Assert::IsFalse(sm.has<Movement>());
		}

		TEST_METHOD(contains) {
			rift::SystemManager sm;
			sm.add<Movement>();
			Assert::IsTrue(sm.has<Movement>());
		}
		
		TEST_METHOD(update) {
			rift::EntityManager em;
			rift::SystemManager sm;
			sm.add<Movement>();

			auto a = em.create_entity();

			a.add<Position>(10, 10);
			a.add<Velocity>(11, 11);


			sm.update(em, 1.0);

			Position p = a.get<Position>();
			Assert::IsTrue(p.x == 21 && p.y == 21);
		}
	};
}