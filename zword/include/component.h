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
    float max_speed;
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
    float acceleration;
    float cooldown;
    float fire_rate;
    float recoil;
    float recoil_reduction;
} PlayerComponent;

PlayerComponent* PlayerComponent_create();

typedef struct {
    float range;
    float angle;
    int rays;
    int color[3];
    float brightness;
    int smoothing;
    sfConvexShape* shape;
} LightComponent;

LightComponent* LightComponent_create(float range, float angle, int rays, float brightness);

typedef struct {
    int health;
    float acceleration;
    int target;
} EnemyComponent;

EnemyComponent* EnemyComponent_create();

typedef struct {
    bool enabled;
    bool loop;
    float angle;
    int particles;
    int max_particles;
    int iterator;
    sfVector2f position[100];
    sfVector2f velocity[100];
    float size[100];
    float max_size;
    sfCircleShape* shape;
    float rate;
    float timer;
} ParticleComponent;

ParticleComponent* ParticleComponent_create(float angle, float size, float rate, sfColor color);

typedef struct {
    int entities;
    CoordinateComponent* coordinate[MAX_ENTITIES];
    ImageComponent* image[MAX_ENTITIES];
    PhysicsComponent* physics[MAX_ENTITIES];
    CircleColliderComponent* circle_collider[MAX_ENTITIES];
    RectangleColliderComponent* rectangle_collider[MAX_ENTITIES];
    PlayerComponent* player[MAX_ENTITIES];
    LightComponent* light[MAX_ENTITIES];
    EnemyComponent* enemy[MAX_ENTITIES];
    ParticleComponent* particle[MAX_ENTITIES];
} Component;

void destroy_entity(Component* component, int i);
