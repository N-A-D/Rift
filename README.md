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
Rift is an Entity Component System written in C++14 but requires C++17 for one feature, see below. It offers very fast iteration speeds by grouping entities based on system search criterias. 

Entities are keys (column indices) into a transposed table of component types, where each row of the table is a different type. Systems query for the entities they need using a list of component types and submit a function that performs a transformation on those entities. 

The idea to group entities based on their components is related to indexing in relational databases. The library makes use of sparse integer sets to compactly store entities (indices) to speed up the search for entities with certain components. For more information about sparse integer sets visit https://programmingpraxis.com/2012/03/09/sparse-sets/

Threading has been added as a feature to the library but it requires C++17 in order to make use of the standard's parallel algorithm `std::for_each`. Using the `rift::EntityManager::par_for_entities_with` member, systems can now have their transformations applied in parallel. This new member differs from `rift::EntityManager::for_entities_with` in that systems lose the ability to add/remove/destroy components as well as create/destroy entities with their transformation function. Moreover, updating a `rift::EntityManager` within a system transformation (sequentially via `rift::EntityManager::for_entities_with` or in parallel via `rift::EntityManager::par_for_entities_with`) will lead to undefined behaviour. 

**NOTE:** Parallel application of system transformations *may* actually lead to degraded performance versus sequential transformations. In other words, use this new function only if it leads to increased performance.

## Entities
As mentioned earlier, entities are column indices into a component type table. As such, `rift::Entity` is a proxy class for a `std::uint64_t` index. The index is composed of two parts: a 32 bit **version** and a 32 bit **index**. The **version** distinguishes between **stale** (deceased) and **valid** (alive) entities that have the same **index**. This is necessary as the **index** maps an *entity* to its components, and you wouldn't want a stale entity modifying a valid entity's state.

Entities in Rift cannot be created directly, they must be created using a `rift::EntityManager`. This decision was made in order to avoid potential errors related to an invalid entity modifying a valid entity's state.

Entities are created as follows:
```cpp
rift::EntityManager manager;
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

Additionally, since this transformation only modifies the entity's components and not the entity itself, we can make use of the `rift::EntityManager::par_for_entities_with` parallelize execution of the transform:
```cpp
entity_manager.par_for_entities_with<Position, Direction>([](Position& pos, Direction& dir) {
    pos.x += dir.x;
    pos.y += dir.y;
});
```

**Note:** Rift does not include any form of messaging system to facilitate intersystem communication. It is up to the user to implement such a system if there is a need for it. 
