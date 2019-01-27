# What is an Entity Component System?
An Entity Component System is an architectural design motivated by efficient CPU cache usage. The design separates state from behaviour by storing data in contiguous memory and applying transformations on that data to achieve intended behaviour(s).

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
Rift is a *header-only* Entity Component System written in C++14 and only requires C++17 if parallel processing is desired. 

Entities are *essentially* primary keys (column indices) to a transposed table of component types, where each row of the table is a different type. Systems can query for the entities they would like to transform using a list of component types. 

The library is capable of fast iterations over entities as it caches them based on a system's search criteria (component type lists). The library makes use of sparse integer sets to cache entities with certain components. For more information about sparse integer sets visit https://programmingpraxis.com/2012/03/09/sparse-sets/

Parallelized execution of a system's transformation function is possible with the `rift::EntityManager::par_for_entities_with` member. Use of this member function is subject to certain preconditions (see the conditions below). Moreover, its use requires a C++17 conformant compiler in order to utilize the parallel version of `std::for_each`.

**Preconditions:**
A system's transformation function must satisfy these conditions *before* it is submitted with a call to `rift::EntityManager::par_entities_with`:
1. Does not make any calls to `rift::EntityManager::create_entity` or `rift::EntityManager::create_copy_of`.
1. Does not make any calls to `rift::EntityManager::update`.
1. Does not make any calls to any of `rift::Entity::destroy`, `rift::Entity::add`, `rift::Entity::replace`, `rift::Entity::remove`, and `rift::Entity::get` (multiple writes).
1. Does not modify component(s) other entities depend on.

**NOTE:** Failure to comply with the aforementioned conditions **will** result in undefined behaviour.

In order to use the `rift::EntityManager::par_entities_with` member function, define `RIFT_ENABLE_PARALLEL_TRANSFORMATIONS` before including either `rift.h` or `entity.h` in some source file.   
For example:
```cpp
// SomeSourceFile.cpp
#define RIFT_ENABLE_PARALLEL_TRANSFORMATIONS
#include "rift/rift.h"
``` 
**NOTE:** If C++17 is not available with your compiler **DO NOT** define `RIFT_ENABLE_PARALLEL_TRANSFORMATIONS`.

Lastly, parallelization is an optimization that can increase performance if the number of entities is a system is transforming is *large enough*. If there are too few entities to transform, parallel execution may actually perform worse than sequential execution. Therefore, it is imperative that there exists a performance problem concerning large numbers of entities before attempting to use `rift::EntityManager::par_entities_with`.

## Entities
As mentioned earlier, entities are column indices into a component type table. As such, `rift::Entity` is a proxy class for a `std::uint64_t` identification number. The id is composed of two parts: a 32 bit **version** and a 32 bit **index**. The **index** is used to identify components that belong to an entity. The **version** distinguishes between **stale** (deceased) and **valid** (alive) entities that have the same **index**.   

There are only two ways to create *distinct* entities:
1. Using the `rift::EntityManager::create_entity` member function.
1. Using the `rift::EntityManager::create_copy_of` member function. (Requires an existing entity to copy construct components from).   

## Components 
In Rift, Components are meant to have as little logic associated with them as possible. In fact, an ideal component is a *POD* that just stores state information. State is then modified using a system transformation.
### Implementation notes:
- Every *component* must inherit from `rift::Component` in order to be considered a *component*.
- Every *component* must include a default constructor as well as a constructor that initializes all of its POD members. 
- Every *component* must be copy constructible/assignable.   

For instance, the following is an example of a *Position* component:
```cpp
struct Position : public rift::Component<Position> {
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
In Rift, Systems define entity behaviour by transforming an entity's state.
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
