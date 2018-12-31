# What is an Entity Component System?
The Entity Component System design pattern separates state and behaviour motivated by efficient CPU cache usage. The approach focuses on storing data contiguously and applying transformations on that data.

Entity Component Systems are usually split up into three parts:
1. Entities:   Objects whose state is defined by its set of components.
1. Components: Blocks of data that describe some aspect of an entity.
1. Systems:    Operators that transform entity states en masse.

For more information on entity component systems:   
http://www.roguebasin.com/index.php?title=Entity_Component_System  
http://gameprogrammingpatterns.com/component.html   
https://medium.com/ingeniouslysimple/entities-components-and-systems-89c31464240d  
https://github.com/junkdog/artemis-odb/wiki/Introduction-to-Entity-Systems   

# Library overview
Rift is an Entity Component System written in C++14 and only requires C++17 if parallel processing is desired. The library offers very fast iteration speeds by grouping entities based on system search criterias, avoiding the need to search for entities every system update. 

Entities are primary keys (column indices) into a transposed table of component types, where each row of the table is a different type. Systems query for the entities they need using a list of component types and submit a function that performs a transformation on those entities. 

The idea to group entities based on their components is related to indexing in relational databases. The library makes use of sparse integer sets to speed up the search for entities with certain components. For more information about sparse integer sets visit https://programmingpraxis.com/2012/03/09/sparse-sets/

Parallelization has been added to the library which requires C++17 in order to make use of the standard's new parallelized implementation of `std::for_each`. Using the new `rift::EntityManager::par_for_entities_with` member, systems can now have their transformations applied in parallel.

**NOTE:**
The following preconditions must be met in order for a system to make use of the new `rift::EntityManager::par_for_entities_with` member:
1. Does not make any calls to `rift::EntityManager::create_entity`.
1. Does not make any calls to `rift::EntityManager::update`.
1. Does not make any calls to any of `rift::Entity::destroy`, `rift::Entity::add`, `rift::Entity::replace`, `rift::Entity::remove`, and `rift::Entity::get` (Component modification only).   

Failure to comply with the aforementioned conditions will result in data races.

To start using the new member function, ensure that you have access to compiler that supports C++17 and define `RIFT_ENABLE_PARALLEL_TRANSFORMATIONS` before including either `rift.h` or `entity.h`. 
For example:
```cpp
#define RIFT_ENABLE_PARALLEL_TRANSFORMATIONS
#include "rift/rift.h"
```

The library is intended to be compatible with C++14, with its only C++17 dependency being the use of the new standard algorithm. Therefore, if C++17 is not available to you, simply do not define `RIFT_ENABLE_PARALLEL_TRANSFORMATIONS`.

As a final note, parallelization *may* provide tangible benefits if the number of entities a system transforms is large enough. If the number of entities is too small, the additional overhead incurred by the new standard algorithm might outweigh any benefits with parallel execution. Therefore, it is imperative to first be sure there *is* a performance problem before trying to optimize.    

## Entities
As mentioned earlier, entities are column indices into a component type table. As such, `rift::Entity` is a proxy class for a `std::uint64_t` index. The index is composed of two parts: a 32 bit **version** and a 32 bit **index**. The **version** distinguishes between **stale** (deceased) and **valid** (alive) entities that have the same **index**. This is necessary as the **index** maps an *entity* to its components, and you wouldn't want a stale entity modifying a valid entity's state.

Entities are only created using the `rift::EntityManager::create_entity` member function. This decision was made in order to avoid potential errors related to an invalid entities modifying a valid entity's state.

Entities are created as follows:
```cpp
rift::Entity entity = manager.create_entity();
```

## Components 
In Rift, Components are meant to have as little logic associated with them as possible. In fact, an ideal component is a *POD* that just stores state information. State is then modified using a system transformation.
### Implementation notes:
- Every *component* must inherit from `rift::Component` in order to be considered a *component*.
- Every *component* must include a default constructor as well as a constructor that initializes all of its POD members. 
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
- Every *system* must implement the `rift::BaseSystem::update(rift::EntityManager&, double)` member. This function is where entity transformations should be carried out. 

Systems submit functions to an entity manager which is then carried out on every entity that matches the system's search criteria. 
For example, suppose there were two components *Position* and *Direction*, then a system's submitted query could look like the following:
```cpp
entity_manager.for_entities_with<Position, Direction>([](rift::Entity entity, Position& pos, Direction& dir){
    pos.x += dir.x;
    pos.y += dir.y;
});
```

Alternatively, since this system only modifies the components and not the entity itself, we can make use of the `rift::EntityManager::par_for_entities_with` to parallelize execution of the transform:
```cpp
entity_manager.par_for_entities_with<Position, Direction>([](Position& pos, Direction& dir) {
    pos.x += dir.x;
    pos.y += dir.y;
});
```

**Note:** Rift does not include any form of messaging system to facilitate intersystem communication. It is up to the user to implement such a system if there is a need for it. 

An example project that makes use of the Rift can be found [here](https://github.com/N-A-D/Doodle/tree/master/Particles). 
