# What is an Entity Component System?
An Entity Component System is an architectural design primarily used in game development. The aim is to improve CPU cache usage by storing data contiguously and transforming it in batches.

There are three parts to an Entity Component System:
1. Entities:   Objects whose state is defined by its set of components.
1. Components: Blocks of data that describe some aspect of an entity.
1. Systems:    Operators that transform entity states en masse.

For more information on entity component systems:   
http://www.roguebasin.com/index.php?title=Entity_Component_System  
http://gameprogrammingpatterns.com/component.html   
https://medium.com/ingeniouslysimple/entities-components-and-systems-89c31464240d  
https://github.com/junkdog/artemis-odb/wiki/Introduction-to-Entity-Systems   

# Library overview
Rift is a *header-only* Entity Component System written in C++17. 

Entities are indices to a transposed table of component types, where each row of the table is a different type. Systems can query for the entities they would like to transform using a list of component types. 

The library is capable of fast iterations over entities as it caches them based on a search requests (lists of component types). The library makes use of sparse integer sets to cache entities with certain components. For more information about sparse integer sets visit https://programmingpraxis.com/2012/03/09/sparse-sets/

Parallel transformations are possible with the `rift::EntityManager::par_for_entities_with` member. Use of this function is subject to certain conditions below. **NOTE: Failure to comply with the following conditions will lead to undefined behaviour**   

A system's transformation function may not:
1. Make any calls to `rift::EntityManager::create_entity` or `rift::EntityManager::create_copy_of`.
1. Make any calls to `rift::EntityManager::update`.
1. Make any calls to any of `rift::Entity::destroy`, `rift::Entity::add`, `rift::Entity::replace`, `rift::Entity::remove`, and, in the special case of `rift::Entity::get`, have multiple writers to the same component.   

## Entities
In Rift, an Entity is an index. 
### Implementation notes:
- `rift::Entity` is a proxy class for a 64 bit identification number composed of two parts:
  - A 32 bit *index*. This is used to identify the components that belong to an entity.
  - A 32 bit *version*. This is used to distinguish stale and valid entities with the same *index*.
- There are only **two** ways to create **distinct** entities:
  - Using the `rift::EntityManager::create_entity` member function.
  - Using the `rift::EntityManager::create_copy_of` member function. (Requires an existing entity to copy construct components from).   
- The only way to destroy a `rift::Entity` is by calling its `rift::Entity::destroy` member function. Entities are still valid after calling `rift::Entity::destroy` and are only truly invalid after their manager updates.
- Entities add components to themselves using the `rift::Entity::add` member function.

## Components 
In Rift, a Component is a *POD* with no accompanying logic. Logic is handled by Systems.
### Implementation notes:
- Every *component* must include a default constructor.
- Every *component* *should* include a constructor that properly initializes it. 
- Every *component* must be copy constructible/assignable.   

For instance, the following is an example of a *Position* component:
```cpp
struct Position {
  Position() : x(0.0f), y(0.0f) {}
  Position(float x, float y) : x(x), y(y) {}
  float x, y;
};
```
Continuing on with the example above, an entity can add the component as follows:
```cpp
entity.add<Position>(100.0f, 25.0f);
```

## Systems
In Rift, Systems define behaviour by transforming components.
### Implementation notes
- Every *system* must inherit from `rift::System` in order to be considered a *System*. 
- Every *system* must implement the `rift::BaseSystem::update` member.

Systems submit their transformation functions to the entity manager given to them in the `rift::BaseSystem::update` function. The entity manager will immediately apply that function on every entity that satisfies the systems search criteria.   
For example, suppose there are two components *Position* and *Direction*. A system's query could look like the following:
```cpp
struct MovementSystem : public rift::System<MovementSystem> {
  void update(rift::EntityManager& em, double dt) override {
    em.for_entities_with<Position, Direction>([](rift::Entity entity, Position& pos, Direction& dir){
        pos.x += dir.x * dt;
        pos.y += dir.y * dt;
    });
  }
};
```

Alternatively, since this system only modifies the components and not the entity itself, we can make use of the `rift::EntityManager::par_for_entities_with` to parallelize execution of the transform:
```cpp
struct MovementSystem : public rift::System<MovementSystem> {
  void update(rift::EntityManager& em, double dt) override {
    em.par_for_entities_with<Position, Direction>([](Position& pos, Direction& dir){
        pos.x += dir.x * dt;
        pos.y += dir.y * dt;
    });
  }
};
```

**NOTE:** Rift does not provide any facilities that enable intersystem communication. It is up to the user to implement such a system if there is a need for it.

## Benchmarks
Two benchmarks accompany the main project. Both only compare speeds between some two related features of the library and not with any other ECS. The *Iteration* benchmark compares the speeds between sequential and parallel entity transformations. The *EntityCreationAndDestruction* benchmark compares the speeds between the only two entity creation functions. 

## Example   
The following example demonstrates essentially how to use the library. It simulates moving entities using *Position* and *Direction*.   
```cpp
#define RIFT_USE_SINGLE_PRECISION_DELTA_TIME // float is the data type for delta time 
#include <rift/rift.h>

struct Position {
  Position() = default;
  Position(float x, float y) : x(x), y(y) {}
  float x = 0.f, y = 0.f;
};
struct Direction {
  Direction() = default;
  Direction(float x, float y) : x(x), y(y) {}
  float x = 0.f, y = 0.f;
};

struct MovementSystem : public rift::System<MovementSystem> {
  MovementSystem() = default;
  void update(rift::EntityManager& entities, float dt) override {
    entities.for_entities_with<Position, Direction>([](auto entity, auto& p, auto& d) {
      p.x += d.x * dt;
      p.y += d.y * dt;
    });
  }
};

int main(int, char**) {
  rift::EntityManager entities;
  rift::SystemManager systems(entities);
  
  systems.add<MovementSystem>();
  
  // ... Creating entities with Position and Direction
  /*
    ....
  */
  // ... Updating the entities
  /*
    ...
  */
  systems.update<MovementSystem>(1.0 / 60.0);
  
  return 0;
}

```
