#pragma once

#include <stdbool.h>

#include <SFML/Graphics.h>

#define MAX_ENTITIES 1000


typedef struct {
    sfVector2f position;
    float angle;
} CoordinateComponent;

CoordinateComponent* CoordinateComponent_create(sfVector2f pos, float angle);

typedef struct {
    sfVector2f scale;
    sfSprite* sprite;
} ImageComponent;

ImageComponent* ImageComponent_create(char filename[20], float scale);

typedef struct {
    sfVector2f velocity;
    sfVector2f acceleration;
    struct {
        sfVector2f overlap;
        sfVector2f velocity;
    } collision;
    float angular_velocity;
    float angular_acceleration;
    float mass;
    float friction;
    float bounce;
    float drag;
} PhysicsComponent;

PhysicsComponent* PhysicsComponent_create(float mass, float friction, float bounce, float drag);

typedef struct {
    float radius;
    sfCircleShape* shape;
} CircleColliderComponent;

CircleColliderComponent* CircleColliderComponent_create(float radius);

typedef struct {
    float width;
    float height;
    sfRectangleShape* shape;
} RectangleColliderComponent;

RectangleColliderComponent* RectangleColliderComponent_create(float width, float height);

typedef struct {
    int health;
    float max_speed;
    float acceleration;
    float cooldown;
} PlayerComponent;

PlayerComponent* PlayerComponent_create();

typedef struct {
    float range;
    float angle;
    int rays;
    float brightness;
} LightComponent;

LightComponent* LightComponent_create(float range, float angle, int rays, float brightness);

typedef struct {
    int entities;
    CoordinateComponent* coordinate[MAX_ENTITIES];
    ImageComponent* image[MAX_ENTITIES];
    PhysicsComponent* physics[MAX_ENTITIES];
    CircleColliderComponent* circle_collider[MAX_ENTITIES];
    RectangleColliderComponent* rectangle_collider[MAX_ENTITIES];
    PlayerComponent* player[MAX_ENTITIES];
    LightComponent* light[MAX_ENTITIES];
} Component;

void destroy_entity(Component* component, int i);
