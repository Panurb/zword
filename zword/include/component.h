#pragma once

#include <stdbool.h>

#include <SFML/Graphics.h>

#define MAX_ENTITIES 1000
#define MAX_NEIGHBORS 20
#define MAX_PATH_LENGTH 50


typedef struct {
    sfVector2f position;
    float angle;
    int parent;
} CoordinateComponent;

CoordinateComponent* CoordinateComponent_create(sfVector2f pos, float angle);

typedef struct {
    sfVector2f scale;
    sfSprite* sprite;
    float shine;
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
    float angular_drag;
    float max_angular_speed;
} PhysicsComponent;

PhysicsComponent* PhysicsComponent_create(float mass, float friction, float bounce, float drag, float angular_drag);

typedef struct {
    bool enabled;
    float radius;
    sfCircleShape* shape;
} CircleColliderComponent;

CircleColliderComponent* CircleColliderComponent_create(float radius);

typedef struct {
    bool enabled;
    float width;
    float height;
    sfRectangleShape* shape;
} RectangleColliderComponent;

RectangleColliderComponent* RectangleColliderComponent_create(float width, float height);

typedef struct {
    int health;
    float acceleration;
    int vehicle;
    int item;
    int inventory_size;
    int inventory[4];
    int grabbed_item;
} PlayerComponent;

PlayerComponent* PlayerComponent_create();

typedef struct {
    bool enabled;
    float range;
    float angle;
    int rays;
    sfColor color;
    float brightness;
    float max_brightness;
    int smoothing;
    sfConvexShape* shape;
    sfCircleShape* shine;
    float flicker;
    float speed;
} LightComponent;

LightComponent* LightComponent_create(float range, float angle, int rays, sfColor color, float brightness, float speed);

typedef struct {
    int health;
    float acceleration;
    int target;
    int path[MAX_PATH_LENGTH];
} EnemyComponent;

EnemyComponent* EnemyComponent_create();

typedef struct {
    bool enabled;
    bool loop;
    float angle;
    int particles;
    int max_particles;
    int iterator;
    float spread;
    float speed_spread;
    float speed;
    float max_time;
    sfVector2f position[100];
    sfVector2f velocity[100];
    float time[100];
    float max_size;
    float min_size;
    sfColor color;
    sfColor inner_color;
    sfCircleShape* shape;
    float rate;
    float timer;
} ParticleComponent;

ParticleComponent* ParticleComponent_create(float angle, float spread, float max_size, float min_size, float speed, float rate, sfColor color, sfColor inner_color);

typedef struct {
    int driver;
    float acceleration;
    float max_speed;
    float turning;
} VehicleComponent;

VehicleComponent* VehicleComponent_create();

typedef struct {
    float cooldown;
    float fire_rate;
    float recoil;
    float recoil_up;
    float recoil_down;
    sfConvexShape* shape;
} WeaponComponent;

WeaponComponent* WeaponComponent_create(float fire_rate, float recoil_up, float recoil_down);

typedef struct {
    int size;
} ItemComponent;

ItemComponent* ItemComponent_create(int size);

typedef struct {
    int neighbors[MAX_NEIGHBORS];
    int weights[MAX_NEIGHBORS];
    int came_from;
    float f_score;
    float g_score;
} WaypointComponent;

WaypointComponent* WaypointComponent_create();

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
    VehicleComponent* vehicle[MAX_ENTITIES];
    WeaponComponent* weapon[MAX_ENTITIES];
    ItemComponent* item[MAX_ENTITIES];
    WaypointComponent* waypoint[MAX_ENTITIES];
} Component;

Component* Component_create();

int get_index(Component* component);

void destroy_entity(Component* component, int i);

sfVector2f get_position(Component* component, int i);

float get_angle(Component* component, int i);
