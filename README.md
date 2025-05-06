# entity-mini
A simple header-only entity component system. 

## Background
An entity component system (ECS) is a data structure designed for games that manages lists of components that belong to various entities.    
        
It's useful to treat entites as containers of components, where components are any user defined type that could make up an entity (maybe transform, mesh, velocity, health, AI, etc). Having entities be built of reusable components is a nice evolution from the nightmarish 10,000 line entity base classes and inheritance trees of the 90's (so i've heard, I wasn't there).      

### consider the hardware
If entites are actually stored as aggregates of components, there is a large stride in memory between each subsequent component. This is what the layout of data in memory looks like, compared to what the cache line of a system that only cares about one component is gonna look like.

| byte | content       | cache line    |
|------------------|--------------------|--------------------|
| 0           | entity0_pos   | --noise--     |
| 8           | entity0_velo  | --noise--     |
| 16          | entity0_mesh  | entity0_mesh  |
| 24          | entity0_AI    | --noise--     |
| 32          | entity1_pos   | --noise--     |
| 40          | entity1_velo  | --noise--     |
| 48          | entity1_mesh  | entity1_mesh  |
| 52          | entity1_AI    | --noise--     |


If you're unfamiliar with this kind of thing, the difference between missing cache and not missing cache is not small, on the order of ~100x.            
If we store entities contiguously like this, we are wasting a lot of each cache line we pull in as we move through the array, because most systems only need to operate on a few components.         
         
To address this, we could store all components in separate contigous arrays, and treat an 'entity' as more of an ID that we use to index these component arrays. This is what an ECS does. Designing around this kind of stuff is often called 'data oriented design' [[wikipedia](https://en.wikipedia.org/wiki/Data-oriented_design)] and this is a big deal in AAA dev apparently.
   
Regardless of the performance theory ECS's can also just be great organizational tools.    

### How does it work?
* Entites are actually just IDs, used to link components together
* Components are any user defined type
* The scene stores components in large contigious arrays, making looping over components effecient.
* The ECS provides a simple way to loop over every entity that has a specific set of components

Here is an example of creating a scene with some entities, giving them components, and looping over them. For an example of real use, see something I've used it for like [minecraft](https://github.com/collebrusco/minecraft) or the [2d tile engine](https://github.com/collebrusco/untitled-tile-engine)

```c++
ECS scene;
auto e1 = scene.newEntity();
auto e2 = scene.newEntity();
auto e3 = scene.newEntity(); 

scene.addComp<Transform>(e1);
scene.addComp<Transform>(e2, vec2(0, 0), 90.f); // forward component constructor args
scene.addComp<Transform>(e3);    

scene.addComp<Renderable>(e1);
scene.addComp<Renderable>(e2);

// view every entity with a transform AND a renderable...
for (auto e : scene.view<Transform, Renderable>()){   
  auto& transform = scene.getComp<Transform>(e); // get reference to component...
  // do something...
}

// give view no template args to view all entities...
for (auto e : scene.view<>()){
  // do something...
}
```
