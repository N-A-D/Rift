# What is an Entity Component System?
An entity component system is a framework for decoupling data and logic. The framework separates resposibilities into three parts: entities, components, and systems. Entities use groups of components to describe some object in the world, where each component is a logicless collection of data that describes some smaller idea. Systems carry out behaviour on every entity whose desciptions matches the one it is searching for. By separating roles in this way, an entity can change the way it behaves at runtime by adding/removing components as it sees fit. 

For more information about entity component systems and component based design in general, check out these links:  
[Wikipedia](https://en.wikipedia.org/wiki/Entity%E2%80%93component%E2%80%93system)  
[Game Programming Patterns](http://gameprogrammingpatterns.com/component.html)

# Library overview
Rift is an Entity Component System library written in C++ 14. It offers fast runtime speed by caching entities based on system search criteria. The framework is implemented in a similar way to that of a database. Entities are primary keys into a transposed table of components, where each row of the table is different component type. Systems query the table for entities (column indices) that match their search criteria (component types). 

## Entities
As mentioned earlier, entities are essentially column indices into a component type table. As such, `rift::Entity` is a convenience class for a `std::uint64_t` index. The index is composed of two parts: a version and the actual index. The version is necessary in order to reuse indices in the table. 

Entitites in Rift cannot be created directly, they must be created using a `rift::EntityManager`. This is to avoid errors related to invalid entities.

Entities are created as follows:
```
rift::EntityManager manager;
rift::Entity entity = manager.create_entity();
```

## Components 
In Rift, Components are meant to be *Plain Old Data* types. Component types are required to subclass `rift::Component` in order for the type to be considered a *Component*. Moreover, all components must include a default constructor as well as a constructor that initializes all of its POD types. 

For instance, the following is an example of a *Position* component:
```
struct Position : public rift::Component<Position> {
  Position() = default;
  Position(float x, float y) : x(x), y(y) {}
  float x, y;
};
```

Continuing on with the example above, an entity can add the component as follows:
```
entity.add<Position>(100.0f, 25.0f);
```

## Systems
In Rift, Systems are what define the behaviour of different entities.
Systems perform entity queries using the `rift::EntityManager::for_entities_with` member.
