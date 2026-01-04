# Development Notes

## Step 1: Initiation: Box2D && SFML

Pain Points:

- Hassle of maintaining both physics engine and rendering
- Understanding key concepts such as units used, scale and frame settings

---

## Step 2: Simple Box Spawn

Simulate a box falling to the ground, and spawn new boxes on right mouse click. The goal was to just learn the mechanics of
creating bodies and drawing them and how the SFML event system works.

### The Plan

> create a ground -> spawn a box above ground -> box collides with ground and comes to rest
> on right click -> spawn box at mouse location -> box collides with ground and comes to rest

The first thing was to create a simple box that is created at a certain height above the ground:

- Create a ground body and render it
- Create a dynamic body and render it
- Spawn boxes on right mouse click

Sounds simple at first, but later some issues arised due to bugs such as:

- Wrong `SCALE` used to scale down the box2d world dimensions to match sfml's pixed based rendering
- Wrong half widths set for box2d bodies
- Box falling through the ground and to infinity (not colliding at ground and stopping)
- Box getting stuck at the released mouse location and not continuing to simulate its physics

### The Problem

The issues were ofcouse due to lack of knowledge of key concepts, however others were dumb such as the last one.

The issue with the box falling through the ground wasn't about a static body, nor setting the box as a bullet, nor was
it a scaling issue. The problem was much more elementary that it broke my brain for several days. LLM not helping, retried
the getting started example from box2d docs and just couldnt figure it out... Until.

The issue was stupidly simple:

```
    // Create ground body
    b2BodyDef groundBodyDef;
    groundBodyDef.position = {groundX, groundY}; // near bottom center in meters
    b2BodyId groundId = b2CreateBody(world_id, &groundBodyDef);
    // Box2D shapes expect half-widths for boxes
    b2Polygon groundBox = b2MakeBox((width / SCALE) / 1.0f, (20.f / SCALE) / 2.0f);
    b2ShapeDef groundShapeDef = b2DefaultShapeDef();

```

From a naiive typescript perspective, this code should not even compile. In a base case scenario, you get a syntax
error under:

```
groundBodyDef.position = {groundX, groundY};
```

with a error message along the lines of "Variable 'groundBodyDef' is used before being assigned".

#### The Fix

By now it was obvious that I had not initialized the `groundBodyDef` and therefore, it is not even registered
in the box2d engine.

```
b2BodyDef groundBodyDef = b2DefaultBodyDef();
```

The key was to just initialize the ground body, and the rest of the code worked as it should. The box is now perfectly landing on
the ground and coming to a stop.

---

## Step 3: Drag and Drop Bodies

The task is straightforward, create a functionality to drag and drop objects in the simulation.

### The Plan

> on left mouse click -> find objects under the mouse location -> on mouse move event -> set box position to mouse position

The implementation is relatively simple and I wrote the code in a matter of minutes:

- On creation of a body (box), store the box and shape struct in a map
- On mouse left click SFML event, check if any of the boxes in the map are within the mouse coordinates, if found, return the entity
- On mouse move SFML event, update the position of the box2d body to match the mouse coordinates

This works in practice. For a few times, till it breaks the physics engine for that specific box.

### The Problem

Here's what happened:

- You drag an object
- You release the object
- It sometimes falls back to ground (obeys the engine)
- Sometimes it just breaks the physics and sticks to where the mouse was released

A few interesting properties of the stuck objects:

- They can no longer react to gravity
- Upon colliding with a functional body, they spring back to life and fall down to the ground
- They can no longer obey the drag and drop rules, when dragged, they stick to the last position before release
- The only way to unstuck the box is to spawn another box on top of it such that they collide

#### The Fix

The issue with the approach I took was that it was too expensive to set the position of a **box2d body** and the SFML shape. The
method for moving the object was non-conventional. The solution is to use a joint to handle the position of the box2d body, and continue
to update the shape position based on the body.

I learned that there are `joints` in box2d which can be attached to objects to join multiple objects. The exact type is a
`Mouse Joint` which literally attaches an object to the mouse via a slick joint.

This is a huge upgrade as it resulted in smooth realistic movement of the shape when held with the mouse joint, which can be pinned from
any point on the surface of the box instead of just snapping the center to the mouse like the method I did first.

```
// Create the mouse joint and attach the object to drag
void start_drag(BoxEntity *entity, b2WorldId world_id, b2BodyId groundBodyId, sf::RenderWindow &window)
{
    b2MouseJointDef jd = b2DefaultMouseJointDef();
    jd.bodyIdA = groundBodyId;
    jd.bodyIdB = entity->body;
    jd.target = get_mouse_world(window);
    jd.maxForce = 1000.0f * b2Body_GetMass(entity->body);
    jd.dampingRatio = 0.7f;
    jd.hertz = 5.0f;

    mouseJoint = b2CreateMouseJoint(world_id, &jd);

    b2Body_SetAwake(entity->body, true);
}

// Update target if the mouse joint is attached
void update_drag(sf::RenderWindow &window)
{
    if (!b2Joint_IsValid(mouseJoint))
        return;

    b2MouseJoint_SetTarget(mouseJoint, get_mouse_world(window));
}
```

### The Second Problem

I had not initially picked this up since the ground was not even working yet. But when I had finished the drag and drop feature, it had become apparent.

The physics seemed a bit off, and the animation is too static:

- The speed at which the boxes fall with gravity (9.8Kg/m2) and the impact of the box are not realistic and therefore "slower" than reality
- When boxes fall on each other, they dont properly reflect the rotation that would happen upon collision in reality

#### The Fix

These bugs were a bit deeper than syntax issues, they were linked to the `SCALE` used to scale down the physic world dimensions and render them on-screen.
Changing the scale from `30` to `100` helped make the simulation appear more similar to that physics we'd expect on earth.

At scale of 30, the boxes frame was too zoomed. Or actually, the objects were too large in front of the 800x600 window. At scale 100, the objects appear farther
and the relative speed of animation increases causing the physics timing to match what we see in real life.
