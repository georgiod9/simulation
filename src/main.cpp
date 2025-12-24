#include <SFML/Graphics.hpp>
#include <box2d/box2d.h>
#include <unordered_map>
const float SCALE = 30.f;

struct DynamicBodyConfig
{
    b2Vec2 position = {400.f / SCALE, 150.f / SCALE};
    b2Vec2 halfSize = {0.05f, 0.05f};
    float density = 1.f;
    float friction = 0.3f;
    float restitution = 0.1f;
    bool bullet = false;
};
struct BoxEntity
{
    b2BodyId body;
    sf::RectangleShape shape;
};

std::unordered_map<uint32_t, std::unique_ptr<BoxEntity>> box_shapes_map;
BoxEntity *dragged_object = nullptr;
b2JointId mouseJoint = b2_nullJointId;

b2Vec2 get_mouse_world(sf::RenderWindow &window)
{
    sf::Vector2f pixel = window.mapPixelToCoords(
        sf::Mouse::getPosition(window));

    return b2Vec2({pixel.x / SCALE, pixel.y / SCALE});
}

b2BodyId create_dynamic_body(b2WorldId world_id, const DynamicBodyConfig &cfg)
{
    // Create a dynamic box shape
    b2BodyDef bodyDef = b2DefaultBodyDef();
    bodyDef.type = b2_dynamicBody;
    bodyDef.position = cfg.position;
    b2BodyId bodyId = b2CreateBody(world_id, &bodyDef);
    b2Polygon dynamicBox = b2MakeBox(cfg.halfSize.x, cfg.halfSize.y);
    b2ShapeDef boxShapeDef = b2DefaultShapeDef();
    boxShapeDef.density = cfg.density;
    boxShapeDef.material.friction = cfg.friction;
    boxShapeDef.material.restitution = cfg.restitution;

    b2CreatePolygonShape(bodyId, &boxShapeDef, &dynamicBox);

    return bodyId;
}

sf::RectangleShape create_box_shape(const DynamicBodyConfig &cfg)
{
    sf::Vector2f size{
        cfg.halfSize.x * 2.f * SCALE,
        cfg.halfSize.y * 2.f * SCALE};

    sf::RectangleShape shape(size);
    shape.setOrigin(size * 0.5f);
    shape.setFillColor(sf::Color::Red);

    return shape;
}

void create_new_box(b2WorldId world_id, sf::RenderWindow &window, const DynamicBodyConfig &cfg)
{
    b2Vec2 mouse_pos = get_mouse_world(window);
    DynamicBodyConfig cfgWithMouse = cfg;
    cfgWithMouse.position = mouse_pos;
    printf("%f", cfgWithMouse.halfSize.x);
    b2BodyId body_id = create_dynamic_body(world_id, cfgWithMouse);

    sf::RectangleShape shape = create_box_shape(cfgWithMouse);

    BoxEntity *entity = new BoxEntity{body : body_id, shape};

    box_shapes_map.emplace(body_id.index1, std::unique_ptr<BoxEntity>(entity));
}

void follow_mouse(b2BodyId bodyId, sf::RenderWindow &window)
{
    b2Vec2 mousePos = get_mouse_world(window);
    b2Rot rot = b2Body_GetRotation(bodyId);

    b2Body_SetTransform(bodyId, mousePos, rot);
    b2Body_SetLinearVelocity(bodyId, {0, 0});
    b2Body_SetAngularVelocity(bodyId, 0);
}

void render_box(b2BodyId bodyId, sf::RectangleShape &box, sf::RenderWindow &window)
{
    b2Vec2 boxPos = b2Body_GetPosition(bodyId);
    b2Rot rotation = b2Body_GetRotation(bodyId);
    box.setPosition({boxPos.x * SCALE, boxPos.y * SCALE});
    float angleRadians = atan2f(rotation.s, rotation.c);
    float angleDegrees = angleRadians * 180.0f / B2_PI;
    box.setRotation(sf::degrees(angleDegrees));

    window.draw(box);
}

void render_ground(b2BodyId groundId, sf::RectangleShape &ground, sf::RenderWindow &window)
{
    // Ground rendering
    b2Vec2 pos = b2Body_GetPosition(groundId);

    // Render ground
    ground.setPosition({pos.x * SCALE, pos.y * SCALE});

    window.draw(ground);
}

void render_boxes(sf::RenderWindow &window)
{
    if (box_shapes_map.size() == 0)
    {
        return;
    }

    for (auto &[bodyId, entityPtr] : box_shapes_map)
    {
        b2Vec2 pos = b2Body_GetPosition(entityPtr->body);
        b2Rot rot = b2Body_GetRotation(entityPtr->body);

        entityPtr->shape.setPosition({pos.x * SCALE, pos.y * SCALE});

        float angle = atan2f(rot.s, rot.c) * 180.f / B2_PI;
        entityPtr->shape.setRotation(sf::degrees(angle));

        window.draw(entityPtr->shape);
    }
}

bool is_mouse_down()
{
    static bool wasDown = false;
    bool isDown = sf::Mouse::isButtonPressed(sf::Mouse::Button::Left);

    bool clicked = isDown && !wasDown;
    wasDown = isDown;
    return clicked;
}

void drag_object(BoxEntity *entity, sf::RenderWindow &window)
{
    sf::Vector2i mousePixelPos = sf::Mouse::getPosition(window);         // mouse in pixels
    sf::Vector2f mouseWorldPos = window.mapPixelToCoords(mousePixelPos); // world coords
    entity->shape.setPosition(mouseWorldPos);

    // Update Box2D body position
    b2Body_SetTransform(entity->body, {mouseWorldPos.x / SCALE, mouseWorldPos.y / SCALE}, b2Body_GetRotation(entity->body));

    printf("Mouse x:%f y:%f", mouseWorldPos.x, mouseWorldPos.y);
    printf("Object x:%f y:%f", entity->shape.getPosition().x, entity->shape.getPosition().y);
    printf("Dragged.\n");
}

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

void update_drag(sf::RenderWindow &window)
{
    if (!b2Joint_IsValid(mouseJoint))
        return;

    b2MouseJoint_SetTarget(mouseJoint, get_mouse_world(window));
}

void stop_drag()
{
    if (b2Joint_IsValid(mouseJoint))
    {
        b2DestroyJoint(mouseJoint);
        mouseJoint = b2_nullJointId;
    }
}

BoxEntity *find_object_to_drag(sf::RenderWindow &window)
{
    sf::Vector2i mousePixelPos = sf::Mouse::getPosition(window);         // mouse in pixels
    sf::Vector2f mouseWorldPos = window.mapPixelToCoords(mousePixelPos); // world coords

    for (auto &[bodyId, entityPtr] : box_shapes_map)
    {
        printf("finding..\n");
        sf::FloatRect rect = entityPtr->shape.getGlobalBounds();

        // Mouse is hovering over item
        if (rect.contains(mouseWorldPos))
        {
            printf("Mouse x:%f y:%f", mouseWorldPos.x, mouseWorldPos.y);
            printf("Object x:%f y:%f", entityPtr->shape.getPosition().x, entityPtr->shape.getPosition().y);

            printf("Foudn entity\n");
            dragged_object = entityPtr.get();
            return entityPtr.get();
        }
    }
    return nullptr;
}

// Create new object on right click
void handle_spawn_object(const sf::Event::MouseButtonPressed *mouseEvent, b2WorldId world_id, DynamicBodyConfig object_config, sf::RenderWindow &window)
{
    if (mouseEvent->button == sf::Mouse::Button::Right)
    {
        create_new_box(world_id, window, object_config);
    }
}

void handle_find_and_drag_object(BoxEntity *dragged_object, b2WorldId world_id, b2BodyId groundId, const sf::Event::MouseButtonPressed *mouseEvent, sf::RenderWindow &window)
{
    if (dragged_object == nullptr)
    {
        printf("Button pressed..\n");
        fflush(stdout);

        // On left click, find object for dragging
        if (mouseEvent->button == sf::Mouse::Button::Left)
        {
            BoxEntity *dragged = find_object_to_drag(window);
            if (dragged)
            {
                start_drag(dragged, world_id, groundId, window);
                printf("Dragging..\n");
                dragged_object = dragged;
            }
            else
            {
                printf("No object dragged..\n");
            }
        }
    }
}

void handle_update_dragged_object(BoxEntity *dragged_object, sf::RenderWindow &window)
{
    if (dragged_object)
    {
        printf("Mouse moved..\n");
        update_drag(window);
    }
}

int main()
{
    const int width = 800;
    const int height = 600;

    // The ground is positioned correctly in Box2D meters relative to SCALE
    const float groundX = 400.f / SCALE;
    const float groundY = (600 - 20 / 2) / SCALE;

    DynamicBodyConfig box_config_1 = {
        position : {400.f / SCALE, 150.f / SCALE},
        halfSize : {0.5f, 0.5f},
        density : 1.f,
        friction : 0.3f,
        restitution : 0.1f,
        bullet : false
    };

    DynamicBodyConfig box_config_2 = {
        position : {400.f / SCALE, 150.f / SCALE},
        halfSize : {2.0f, 2.0f},
        density : 1.f,
        friction : 0.3f,
        restitution : 0.1f,
        bullet : false
    };

    // SFML: Create window
    sf::RenderWindow window(sf::VideoMode({width, height}), "2D Simulation");
    window.setFramerateLimit(60);
    window.setVerticalSyncEnabled(true);

    // Box2D: Create physics world with consistent gravity (e.g., standard Earth gravity 9.8 m/s^2)
    b2WorldDef world_def = b2DefaultWorldDef();
    // Use standard Earth gravity in meters/second^2
    world_def.gravity = (b2Vec2){0.0f, 9.8f};
    b2WorldId world_id = b2CreateWorld(&world_def);

    // Create ground body
    b2BodyDef groundBodyDef = b2DefaultBodyDef();
    groundBodyDef.position = {groundX, groundY}; // near bottom center in meters
    // groundBodyDef.type = b2_dynamicBody;
    b2BodyId groundId = b2CreateBody(world_id, &groundBodyDef);
    // Box2D shapes expect half-widths for boxes
    b2Polygon groundBox = b2MakeBox((800.f / SCALE) / 2.0f, (20.f / SCALE) / 2.0f);
    b2ShapeDef groundShapeDef = b2DefaultShapeDef();

    b2CreatePolygonShape(groundId, &groundShapeDef, &groundBox);

    // b2BodyId box1_id = create_dynamic_body(world_id);
    // b2BodyId box2_id = create_dynamic_body(world_id);

    // b2RevoluteJointDef jointDef = b2DefaultRevoluteJointDef();
    // jointDef.bodyIdA = box1_id;
    // jointDef.bodyIdB = box2_id;
    // jointDef.localAnchorA = (b2Vec2){0.0f, 0.0f};
    // jointDef.localAnchorB = (b2Vec2){1.0f, 2.0f};

    // b2JointId joint1 = b2CreateRevoluteJoint(world_id, &jointDef);

    float timeStep = 1.0f / 60.0f;
    int subStepCount = 3;

    // SFML rendering setup (remains the same)
    sf::RectangleShape groundShape({800.f, 20.f});
    groundShape.setOrigin({400.f, 10.f});
    groundShape.setFillColor(sf::Color::Green);

    // sf::RectangleShape box({10.0f * SCALE, 10.0f * SCALE});
    // box.setOrigin({0.5f * SCALE, 0.5f * SCALE});
    // box.setFillColor(sf::Color::Red);

    // sf::RectangleShape box2({10.0f * SCALE, 10.0f * SCALE});
    // box2.setOrigin({0.5f * SCALE, 0.3f * SCALE});
    // box2.setFillColor(sf::Color::Blue);

    const float amplitude = 100.f;
    const float frequency = 0.02f;

    std::vector<sf::RectangleShape> boxes;

    create_new_box(world_id, window, box_config_2);

    // Main loop
    while (window.isOpen())
    {
        // Process events
        while (const std::optional<sf::Event> event = window.pollEvent())
        {
            // Handle mouse released event
            if (event->is<sf::Event::MouseButtonReleased>())
            {
                fflush(stdout);
                printf("Released.\n");
                if (dragged_object)
                {
                    stop_drag();
                    dragged_object = nullptr;
                }
            }

            // Handle Mouse Moved events
            if (event->is<sf::Event::MouseMoved>())
            {
                fflush(stdout);
                const auto &mouseEvent = event->getIf<sf::Event::MouseMoved>();

                // Move the dragged object
                handle_update_dragged_object(dragged_object, window);
            }

            // Handle Mouse press events
            if (event->is<sf::Event::MouseButtonPressed>())
            {
                const auto &mouseEvent = event->getIf<sf::Event::MouseButtonPressed>();

                // Drag an object
                handle_find_and_drag_object(dragged_object, world_id, groundId, mouseEvent, window);

                // Spawn a new object on right click
                handle_spawn_object(mouseEvent, world_id, box_config_1, window);
            }

            if (event->is<sf::Event::Closed>())
                window.close();
        }

        // Update physics
        b2World_Step(world_id, timeStep, subStepCount);

        // Render
        window.clear(sf::Color::Black);

        render_boxes(window);
        render_ground(groundId, groundShape, window);
        window.display();
    }

    // Clean up Box2D world when done
    b2DestroyWorld(world_id);

    return 0;
}
