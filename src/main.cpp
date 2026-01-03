#include <SFML/Graphics.hpp>
#include <box2d/box2d.h>
#include <unordered_map>
#include <ToolMode.h>

const float SCALE = 100.f;
struct ChainState
{
    b2BodyId *lastBody = nullptr;
    bool active = false;
};
struct Wall
{
    b2BodyId bodyId;
    sf::RectangleShape shape;
};
struct DynamicBodyConfig
{
    b2Vec2 position = {400.f / SCALE, 150.f / SCALE};
    b2Vec2 halfSize = {0.05f, 0.05f};
    float density = 1.f;
    float friction = 0.3f;
    float restitution = 0.1f;
    bool bullet = false;
    bool isStatic = false;
    bool isRounded = false;
    bool isCircle = false;
    sf::Color color = sf::Color::Red;
};

struct BoxEntity
{
    b2BodyId body;
    // sf::RectangleShape shape;
    std::unique_ptr<sf::Shape> shape;
};

std::unordered_map<uint32_t, std::unique_ptr<BoxEntity>> box_shapes_map;
BoxEntity *dragged_object = nullptr;
b2JointId mouseJoint = b2_nullJointId;
const int width = 800;
const int height = 600;

/** Function prototypes */
b2BodyId create_dynamic_body(b2WorldId world_id, const DynamicBodyConfig &cfg);
std::unique_ptr<sf::Shape> create_circle_shape_ptr(const DynamicBodyConfig &cfg);
sf::RectangleShape create_box_shape(const DynamicBodyConfig &cfg);
std::unique_ptr<sf::Shape> create_box_shape_ptr(const DynamicBodyConfig &cfg);
std::unique_ptr<sf::Shape> create_shape(const DynamicBodyConfig &cfg);
sf::CircleShape create_circle_shape(const DynamicBodyConfig &cfg);
void create_new_box(b2WorldId world_id, sf::RenderWindow &window, const DynamicBodyConfig &cfg);

/** Drag logic */
b2Vec2 get_mouse_world(sf::RenderWindow &window);
void follow_mouse(b2BodyId bodyId, sf::RenderWindow &window);
bool is_mouse_down();
void drag_object(BoxEntity *entity, sf::RenderWindow &window);
void start_drag(BoxEntity *entity, b2WorldId world_id, b2BodyId groundBodyId, sf::RenderWindow &window);
void update_drag(sf::RenderWindow &window);
void stop_drag();
BoxEntity *find_object_to_drag(sf::RenderWindow &window);

/** Spawning */
void handle_spawn_object(const sf::Event::MouseButtonPressed *mouseEvent, b2WorldId world_id, DynamicBodyConfig object_config, bool &is_spawning, sf::RenderWindow &window);
void handle_find_and_drag_object(BoxEntity *dragged_object, b2WorldId world_id, b2BodyId groundId, const sf::Event::MouseButtonPressed *mouseEvent, sf::RenderWindow &window);
void handle_update_dragged_object(BoxEntity *dragged_object, sf::RenderWindow &window);
Wall create_wall(b2WorldId world_id, float x, float y, int direction, bool isGround);

/** Rendering objects */
void render_box(b2BodyId bodyId, sf::Shape &box, sf::RenderWindow &window);
void render_ground(b2BodyId groundId, sf::RectangleShape &ground, sf::RenderWindow &window);
void render_boxes(sf::RenderWindow &window);
/** End function prototypes */

int main()
{
    bool is_spawning = false;
    sf::Clock spawnClock;
    const sf::Time spawnInterval = sf::milliseconds(50);

    ToolMode currentTool = ToolMode::Drag;
    ChainState chain;
    ToolMenu menu(font);

    // The ground is positioned correctly in Box2D meters relative to SCALE
    const float groundX = 300.f / SCALE;
    const float groundY = (height - 10) / SCALE;

    /** Define Body configurations */
    DynamicBodyConfig box_config_1 = {
        position : {400.f / SCALE, 150.f / SCALE},
        halfSize : {0.02f, 0.02f},
        density : 10.0f,
        friction : 0.3f,
        restitution : 0.1f,
        bullet : false,
        isRounded : false,
        isCircle : true,
        color : sf::Color::White
    };

    DynamicBodyConfig box_config_2 = {
        position : {400.f / SCALE, 150.f / SCALE},
        halfSize : {0.5f, 0.5f},
        density : 1.f,
        friction : 0.3f,
        restitution : 0.1f,
        bullet : false,
        isRounded : false,
        isCircle : true,
        color : sf::Color::Blue

    };

    DynamicBodyConfig wall_config = {
        position : {100.f / SCALE, 150.f / SCALE},
        halfSize : {0.05f, 0.5f},
        density : 1.f,
        friction : 0.3f,
        restitution : 0.1f,
        bullet : true,
        isCircle : false,
        color : sf::Color::Magenta

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
    // b2BodyDef groundBodyDef = b2DefaultBodyDef();
    // groundBodyDef.position = {groundX, groundY}; // near bottom center in meters
    // // groundBodyDef.type = b2_dynamicBody;
    // b2BodyId groundId = b2CreateBody(world_id, &groundBodyDef);
    // // Box2D shapes expect half-widths for boxes
    // b2Polygon groundBox = b2MakeBox((width / SCALE) / 1.0f, (20.f / SCALE) / 2.0f);
    // b2ShapeDef groundShapeDef = b2DefaultShapeDef();

    // b2CreatePolygonShape(groundId, &groundShapeDef, &groundBox);

    // Create a box
    create_new_box(world_id, window, wall_config);

    Wall leftWall = create_wall(world_id, 0, 0, 0, false);
    Wall rightWall = create_wall(world_id, width, 0, 0, false);
    Wall topWall = create_wall(world_id, 0, 0, 1, false);
    Wall bottomWall = create_wall(world_id, 0, height, 1, true);

    // b2BodyId box1_id = create_dynamic_body(world_id);
    // b2BodyId box2_id = create_dynamic_body(world_id);

    // b2RevoluteJointDef jointDef = b2DefaultRevoluteJointDef();
    // jointDef.bodyIdA = box1_id;
    // jointDef.bodyIdB = box2_id;
    // jointDef.localAnchorA = (b2Vec2){0.0f, 0.0f};
    // jointDef.localAnchorB = (b2Vec2){1.0f, 2.0f};

    // b2JointId joint1 = b2CreateRevoluteJoint(world_id, &jointDef);

    float timeStep = 1.0f / 60.0f;
    int subStepCount = 8;

    // SFML rendering setup (remains the same)
    sf::RectangleShape groundShape({width, 20.f});
    groundShape.setOrigin({height / 2, 10.f});
    groundShape.setFillColor(sf::Color::Green);

    std::vector<sf::RectangleShape> boxes;

    // Create main object
    create_new_box(world_id, window, box_config_2);

    // Main loop
    while (window.isOpen())
    {

        if (is_spawning && spawnClock.getElapsedTime() >= spawnInterval)
        {
            create_new_box(world_id, window, box_config_1);
            spawnClock.restart();
        }

        // Process events
        while (const std::optional<sf::Event> event = window.pollEvent())
        {
            if (const auto *keyPressed = event->getIf<sf::Event::KeyPressed>())
            {
                if (keyPressed->code == sf::Keyboard::Key::Num1)
                {
                    currentTool = ToolMode::Drag;
                    chain.active = false;
                    menu.setActive(currentTool);
                }
                else if (keyPressed->code == sf::Keyboard::Key::Num2)
                {
                    currentTool = ToolMode::Spawn;
                    chain.active = false;
                    menu.setActive(currentTool);
                }
                else if (keyPressed->code == sf::Keyboard::Key::Num3)
                {
                    currentTool = ToolMode::CreateChain;
                    chain.lastBody = nullptr;
                    chain.active = true;
                    menu.setActive(currentTool);
                }
            }

            // Handle mouse released event
            if (event->is<sf::Event::MouseButtonReleased>())
            {
                fflush(stdout);
                printf("Released.\n");

                // Handle continuous spawning
                const auto &mouseEvent = event->getIf<sf::Event::MouseButtonReleased>();
                if (mouseEvent->button == sf::Mouse::Button::Right)
                {
                    is_spawning = false;
                }
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
                printf("pressed..");
                const auto &mouseEvent = event->getIf<sf::Event::MouseButtonPressed>();
                // Drag an object
                handle_find_and_drag_object(dragged_object, world_id, bottomWall.bodyId, mouseEvent, window);

                // Spawn a new object on right click
                handle_spawn_object(mouseEvent, world_id, box_config_1, is_spawning, window);
            }

            if (event->is<sf::Event::Closed>())
                window.close();
        }

        // Update physics
        b2World_Step(world_id, timeStep, subStepCount);

        // Render
        window.clear(sf::Color::Black);

        render_boxes(window);
        // render_ground(groundId, groundShape, window);
        render_ground(bottomWall.bodyId, bottomWall.shape, window);
        render_ground(leftWall.bodyId, leftWall.shape, window);
        render_ground(rightWall.bodyId, rightWall.shape, window);
        render_ground(topWall.bodyId, topWall.shape, window);
        window.display();
    }

    // Clean up Box2D world when done
    b2DestroyWorld(world_id);

    return 0;
}

/** Mouse */

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
    if (cfg.isStatic)
    {
        bodyDef.type = b2_staticBody;
    }
    else
    {
        bodyDef.type = b2_dynamicBody;
    }
    bodyDef.position = cfg.position;
    b2BodyId bodyId = b2CreateBody(world_id, &bodyDef);

    b2Polygon dynamicBody;
    b2ShapeDef boxShapeDef = b2DefaultShapeDef();
    boxShapeDef.density = cfg.density;
    boxShapeDef.material.friction = cfg.friction;
    boxShapeDef.material.restitution = cfg.restitution;
    if (cfg.isCircle)
    {
        b2Circle circle{};
        circle.radius = cfg.halfSize.x * 2;
        b2CreateCircleShape(bodyId, &boxShapeDef, &circle);
    }
    else
    {
        dynamicBody = b2MakeBox(cfg.halfSize.x, cfg.halfSize.y);

        b2CreatePolygonShape(bodyId, &boxShapeDef, &dynamicBody);

        return bodyId;
    }
}

std::unique_ptr<sf::Shape> create_circle_shape_ptr(const DynamicBodyConfig &cfg)
{
    sf::Vector2f size{
        cfg.halfSize.x * 2.f * SCALE,
        cfg.halfSize.y * 2.f * SCALE};

    std::unique_ptr<sf::Shape> shape = std::make_unique<sf::CircleShape>(size.x);
    shape->setOrigin(size * 0.5f);
    shape->setFillColor(sf::Color::Red);
    return shape;
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

std::unique_ptr<sf::Shape> create_box_shape_ptr(const DynamicBodyConfig &cfg)
{
    sf::Vector2f size{
        cfg.halfSize.x * 2.f * SCALE,
        cfg.halfSize.y * 2.f * SCALE};

    std::unique_ptr<sf::Shape> shape = std::make_unique<sf::RectangleShape>(size);
    shape->setOrigin(size * 0.5f);
    shape->setFillColor(sf::Color::Red);
    return shape;
}

std::unique_ptr<sf::Shape> create_shape(const DynamicBodyConfig &cfg)
{
    if (cfg.isCircle)
    {
        sf::Vector2f size{
            cfg.halfSize.x * 2.f * SCALE,
            cfg.halfSize.y * 2.f * SCALE};
        std::unique_ptr<sf::Shape> shape = create_circle_shape_ptr(cfg);
        shape->setOrigin(size);
        shape->setFillColor(cfg.color);
        return shape;
    }
    else
    {
        sf::Vector2f size{
            cfg.halfSize.x * 2.f * SCALE,
            cfg.halfSize.y * 2.f * SCALE};
        std::unique_ptr<sf::Shape> shape = create_box_shape_ptr(cfg);
        shape->setOrigin(size * 0.5f);
        shape->setFillColor(sf::Color::Red);
        return shape;
    }
}

sf::CircleShape create_circle_shape(const DynamicBodyConfig &cfg)
{
    sf::Vector2f size{
        cfg.halfSize.x * 2.f * SCALE,
        cfg.halfSize.y * 2.f * SCALE};

    sf::CircleShape shape(size.x);
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

    std::unique_ptr<sf::Shape> shape = create_shape(cfgWithMouse);
    BoxEntity *entity = new BoxEntity{.body = body_id, .shape = std::move(shape)};

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

void render_box(b2BodyId bodyId, sf::Shape &box, sf::RenderWindow &window)
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

        entityPtr->shape->setPosition({pos.x * SCALE, pos.y * SCALE});

        float angle = atan2f(rot.s, rot.c) * 180.f / B2_PI;
        entityPtr->shape->setRotation(sf::degrees(angle));

        window.draw(*entityPtr->shape);
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
    entity->shape->setPosition(mouseWorldPos);

    // Update Box2D body position
    b2Body_SetTransform(entity->body, {mouseWorldPos.x / SCALE, mouseWorldPos.y / SCALE}, b2Body_GetRotation(entity->body));

    printf("Mouse x:%f y:%f", mouseWorldPos.x, mouseWorldPos.y);
    printf("Object x:%f y:%f", entity->shape->getPosition().x, entity->shape->getPosition().y);
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
        sf::FloatRect rect = entityPtr->shape->getGlobalBounds();

        // Mouse is hovering over item
        if (rect.contains(mouseWorldPos))
        {
            printf("Mouse x:%f y:%f", mouseWorldPos.x, mouseWorldPos.y);
            printf("Object x:%f y:%f", entityPtr->shape->getPosition().x, entityPtr->shape->getPosition().y);

            printf("Foudn entity\n");
            dragged_object = entityPtr.get();
            return entityPtr.get();
        }
    }
    return nullptr;
}

// Create new object on right click
void handle_spawn_object(const sf::Event::MouseButtonPressed *mouseEvent, b2WorldId world_id, DynamicBodyConfig object_config, bool &is_spawning, sf::RenderWindow &window)
{

    if (mouseEvent->button == sf::Mouse::Button::Right)
    {
        is_spawning = true;
        // create_new_box(world_id, window, object_config);
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

Wall create_wall(b2WorldId world_id, float x, float y, int direction, bool isGround)
{
    /** Walls */
    // Create ground body
    b2BodyDef wallBodyDef = b2DefaultBodyDef();
    wallBodyDef.position = {x / SCALE, y / SCALE}; // near bottom center in meters
    if (isGround)
    {
        // wallBodyDef.type = b2_dynamicBody;
    }

    b2BodyId wallLeftId = b2CreateBody(world_id, &wallBodyDef);

    float boxWidth;
    float boxHalfWidth;
    float boxHeight;
    float boxHalfHeight;
    float boxOriginX;
    float boxOriginY;

    // 0 = horizontal, 1 = vertical
    if (direction == 0)
    {
        boxWidth = 20.f;
        boxHalfWidth = boxWidth / 2;
        boxHeight = height;
        boxHalfHeight = boxHeight;
        boxOriginX = 10.f;
        boxOriginY = (height / 2) / SCALE;
    }
    else
    {
        boxWidth = width;
        boxHalfWidth = boxWidth;
        boxHeight = 20.f;
        boxHalfHeight = boxHeight / 2;
        boxOriginX = (boxWidth / 2) / SCALE;
        boxOriginY = boxHeight / 2;
    }
    // Box2D shapes expect half-widths for boxes
    b2Polygon wallLeftBox = b2MakeBox((boxHalfWidth / SCALE) / 1.0f, (boxHalfHeight / SCALE) / 1.0f);
    b2ShapeDef wallLeftShapeDef = b2DefaultShapeDef();

    b2CreatePolygonShape(wallLeftId, &wallLeftShapeDef, &wallLeftBox);

    sf::RectangleShape wallLeftShape({boxWidth, boxHeight});
    wallLeftShape.setOrigin({boxOriginX, boxOriginY});
    wallLeftShape.setFillColor(sf::Color::Green);

    Wall newWall = {bodyId : wallLeftId, shape : wallLeftShape};

    return newWall;
}