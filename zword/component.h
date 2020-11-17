#pragma once

#include <SFML/Graphics.h>


#define MAX_ENTITIES 100


typedef struct {
    sfVector2f position;
    float angle;
} CoordinateComponent;


typedef struct {
    sfVector2f scale;
    sfSprite* sprite;
} ImageComponent;


typedef struct {
    sfVector2f velocity;
    sfVector2f acceleration;
    float angular_velocity;
    float friction;
} PhysicsComponent;


typedef struct {
    sfVector2f position;
    float radius;
    sfCircleShape* shape;
} CircleColliderComponent;


typedef struct {
    sfVector2f position;
    float width;
    float height;
} RectangleColliderComponent;


typedef struct {
    int health;
    float speed;
} PlayerComponent;


typedef struct {
    int entities;
    CoordinateComponent* coordinate[MAX_ENTITIES];
    ImageComponent* image[MAX_ENTITIES];
    PhysicsComponent* physics[MAX_ENTITIES];
    CircleColliderComponent* circle_collider[MAX_ENTITIES];
    RectangleColliderComponent* rectangle_collider[MAX_ENTITIES];
    PlayerComponent* player[MAX_ENTITIES];
} Component;
