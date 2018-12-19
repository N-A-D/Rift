# What is an Entity Component System?
The Entity Component System is a design pattern that separates state and behaviour. It is based on the idea that applications are fundamentally transforming structured data. In such a system, a entity is a collection of data against which transformations are applied. 

Entity Component Systems is broken up into three parts:
1. Entities:   Objects whose state is defined by its components.
1. Components: Blocks of data that individually descibe some aspect of an entity.
1. Systems:    Operations that transform an entity's state.

For more information about entity component systems and component based design in general, check out these links:  
[Wikipedia](https://en.wikipedia.org/wiki/Entity%E2%80%93component%E2%80%93system)  
[Game Programming Patterns](http://gameprogrammingpatterns.com/component.html)

# Library overview
Rift is an Entity Component System library written in C++ 14. It offers fast runtime speed by caching entities based on system search criteria. The framework operates in a similar way to that of a database. Entities are keys (column indices) into a transposed table of components, where each row of the table is different component type. Systems query for entities and submit functions that will operate on each of them and their components, much like in database processing. A form of indexing is used to speed up the search for entities that match a system's search criteria. 

For information about the data structure used for index caching see this [link](https://www.geeksforgeeks.org/sparse-set/)

## Entities
As mentioned earlier, entities are essentially column indices into a component type table. As such, `rift::Entity` is a convenience class for a `std::uint64_t` index. The index is composed of two parts: a version and the actual index. The version is necessary in order to reuse indices in the table. 

Entities in Rift cannot be created directly, they must be created using a `rift::EntityManager`. This is to avoid errors related to invalid entities.

Entities are created as follows:
```cpp
rift::EntityManager manager;
rift::Entity entity = manager.create_entity();
```

## Components 
In Rift, Components are meant to be *Plain Old Data* types. Component types are required to subclass `rift::Component` in order for the type to be considered a *Component*. Moreover, all components must include a default constructor as well as a constructor that initializes all of its POD types. 

For instance, the following is an example of a *Position* component:
```cpp
struct Position : public rift::Component<Position> {
  Position() = default;
  Position(float x, float y) : x(x), y(y) {}
  float x, y;
};
```

Continuing on with the example above, an entity can add the component as follows:
```cpp
entity.add<Position>(100.0f, 25.0f);
```

## Systems
In Rift, Systems are what define the behaviour of different entities.
Every system must inherit from `rift::System` in order for the type to be considered a *System*. Moreover, every system must implement the `rift::BaseSystem::update(rift::EntityManager&, double)` member. This function serves as the point from which entity behaviour is carried out. 

Systems submit functions to an entity manager which is then carried out on every entity that matches the system's search criteria. 
For example, suppose there were two components *Position* and *Direction*, then a system's submitted query could look like the following:
```cpp
entity_manager.for_entities_with<Position, Direction>([](rift::Entity entity, Position& pos, Direction& dir){
    pos.x += dir.x;
    pos.y += dir.y;
});
```
With regards to intersystem communication, Rift does not include any form of messaging system. It is up to the user to implement such a system in the case that systems need to communicate with each other. 

# Additional notes:
The framework is strictly single-threaded as it is. However, there is plan to include multithreading support using a fork-join model. 
