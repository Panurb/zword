#pragma once

#include <SFML/Graphics.h>


#define MAX_ENTITIES 100


typedef struct {
    sfVector2f position;
    float angle;
} CoordinateComponent;

CoordinateComponent* CoordinateComponent_create(float x, float y, float angle);

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
    float friction;
    float bounce;
} PhysicsComponent;

PhysicsComponent* PhysicsComponent_create(float friction, float bounce);

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
} PlayerComponent;

PlayerComponent* PlayerComponent_create();

typedef struct {
    int entities;
    CoordinateComponent* coordinate[MAX_ENTITIES];
    ImageComponent* image[MAX_ENTITIES];
    PhysicsComponent* physics[MAX_ENTITIES];
    CircleColliderComponent* circle_collider[MAX_ENTITIES];
    RectangleColliderComponent* rectangle_collider[MAX_ENTITIES];
    PlayerComponent* player[MAX_ENTITIES];
} Component;
