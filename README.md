# What is an Entity Component System?
An entity component system is a framework for decoupling data and logic. The framework separates resposibilities into three parts entities, components, and systems. Entities use groups of components to describe some object in the world, where each component is a logicless collection of data that describes a smaller idea. Systems carry out behaviour on every entity whose desciptions matches the one it is searching for. By separating roles in this way, an entity can change the way it behaves at runtime by adding/removing components as it sees fit. 

For more information about entity component systems and component based design in general, check out these links:  
[Wikipedia](https://en.wikipedia.org/wiki/Entity%E2%80%93component%E2%80%93system)  
[Game Programming Patterns](http://gameprogrammingpatterns.com/component.html)

# Library overview
Rift is an Entity Component System library written in C++ 14. It offers fast runtime speed by caching entities based on the components they have. It is implemented in a similar way to that of a database. Entities are like primary keys into a table. The columns of the table are the different component types in use. Systems query the table for entities (row indices) that have certain components. 
