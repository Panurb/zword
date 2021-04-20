#pragma once

#include <stdbool.h>

#include <SFML/Graphics.h>
#include <SFML/Audio.h>

#include "util.h"

#define MAX_ENTITIES 10000
#define MAX_NEIGHBORS 50
#define MAX_PATH_LENGTH 50


typedef struct {
    sfVector2f position;
    float angle;
    int parent;
} CoordinateComponent;

typedef struct {
    bool texture_changed;
    float outline;
    Filename filename;
    float width;
    float height;
    sfSprite* sprite;
    float shine;
    int layer;
    sfVector2f scale;
    float alpha;
} ImageComponent;

typedef struct {
    sfVector2f velocity;
    sfVector2f acceleration;
    struct {
        bool collided;
        sfVector2f overlap;
        sfVector2f velocity;
    } collision;
    float angular_velocity;
    float angular_acceleration;
    float mass;
    float friction;
    float bounce;
    float drag;
    float drag_sideways;
    float speed;
    float max_speed;
    float angular_drag;
    float max_angular_speed;
} PhysicsComponent;

typedef enum {
    CIRCLE,
    RECTANGLE
} ColliderType;

typedef enum {
    WALLS,
    ITEMS,
    PLAYERS,
    ENEMIES,
    VEHICLES,
    TREES,
    ROADS,
    RIVERS,
    BULLETS,
    LIGHTS
} ColliderGroup;

typedef struct {
    bool enabled;
    ColliderGroup group;
    ColliderType type;
    int last_collision;
    float radius;
    float width;
    float height;
} ColliderComponent;

typedef enum {
    ON_FOOT,
    SHOOT,
    RELOAD,
    DRIVE,
    MENU,
    MENU_GRAB,
    MENU_DROP,
    PLAYER_DEAD
} PlayerState;

typedef struct {
    int joystick;
    int buttons[12];
    bool buttons_down[12];
    bool buttons_pressed[12];
    bool buttons_released[12];
    int axes[8];
    sfVector2f left_stick;
    sfVector2f right_stick;
    sfVector2f dpad;
    float left_trigger;
    float right_trigger;
} Controller;

typedef struct {
    int target;
    float acceleration;
    int vehicle;
    int item;
    int inventory_size;
    int inventory[4];
    int grabbed_item;
    PlayerState state;
    sfConvexShape* shape;
    sfRectangleShape* line;
    Controller controller;
} PlayerComponent;

typedef struct {
    bool enabled;
    float range;
    float angle;
    int rays;
    sfColor color;
    float brightness;
    float max_brightness;
    sfVertexArray* verts;
    sfCircleShape* shine;
    float flicker;
    float speed;
    float time;
} LightComponent;

typedef enum {
    IDLE,
    INVESTIGATE,
    CHASE,
    DEAD
} EnemyState;

typedef struct {
    EnemyState state;
    float acceleration;
    int target;
    int path[MAX_PATH_LENGTH];
    float fov;
    float vision_range;
} EnemyComponent;

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
    float start_size;
    float end_size;
    sfColor start_color;
    sfColor end_color;
    sfCircleShape* shape;
    float rate;
    float timer;
    sfVector2f origin;
} ParticleComponent;

typedef struct {
    bool on_road;
    float fuel;
    float max_fuel;
    float acceleration;
    float max_speed;
    float turning;
    int size;
    int riders[4];
    sfVector2f seats[4];
} VehicleComponent;

typedef struct {
    float cooldown;
    float fire_rate;
    float recoil;
    float recoil_up;
    float recoil_down;
    float max_recoil;
    int magazine;
    int max_magazine;
    int damage;
    float reload_time;
    bool reloading;
    float range;
    float sound_range;
    float spread;
    sfConvexShape* shape;   // move to player?
} WeaponComponent;

typedef struct {
    int attachments[5];
    int size;
} ItemComponent;

typedef struct {
    int neighbors[MAX_NEIGHBORS];
    int neighbors_size;
    int weights[MAX_NEIGHBORS];
    int came_from;
    float f_score;
    float g_score;
    float range;
} WaypointComponent;

typedef struct {
    int health;
} HealthComponent;

typedef struct {
    sfVector2i resolution;
    float zoom;
    sfShader* shaders[10];
} CameraComponent;

typedef struct {
    bool texture_changed;
    int prev;
    int next;
    float curve;
    float width;
    Filename filename;
    sfConvexShape* shape;
    int points;
} RoadComponent;

typedef struct {
    bool loop;
    int channel;
    float volume;
    float pitch;
    Filename filename;
} SoundEvent;

typedef struct {
    int size;
    SoundEvent* events[4];
    Filename hit_sound;
} SoundComponent;

typedef struct {
    int size;
    void* array[MAX_ENTITIES];
    int order[MAX_ENTITIES];
} OrderedArray;

typedef struct {
    int entities;
    CoordinateComponent* coordinate[MAX_ENTITIES];
    OrderedArray image;
    PhysicsComponent* physics[MAX_ENTITIES];
    ColliderComponent* collider[MAX_ENTITIES];
    OrderedArray player;
    LightComponent* light[MAX_ENTITIES];
    EnemyComponent* enemy[MAX_ENTITIES];
    ParticleComponent* particle[MAX_ENTITIES];
    VehicleComponent* vehicle[MAX_ENTITIES];
    WeaponComponent* weapon[MAX_ENTITIES];
    ItemComponent* item[MAX_ENTITIES];
    WaypointComponent* waypoint[MAX_ENTITIES];
    HealthComponent* health[MAX_ENTITIES];
    CameraComponent* camera[MAX_ENTITIES];
    RoadComponent* road[MAX_ENTITIES];
    SoundComponent* sound[MAX_ENTITIES];
} ComponentData;

ComponentData* ComponentData_create();

CoordinateComponent* CoordinateComponent_add(ComponentData* components, int entity, sfVector2f pos, float angle);
CoordinateComponent* CoordinateComponent_get(ComponentData* components, int entity);
void CoordinateComponent_remove(ComponentData* components, int entity);

ImageComponent* ImageComponent_add(ComponentData* components, int entity, Filename filename, float width, float height, int layer);
ImageComponent* ImageComponent_get(ComponentData* components, int entity);
void CoordinateComponent_remove(ComponentData* components, int entity);

PhysicsComponent* PhysicsComponent_add(ComponentData* components, int entity, float mass, float friction, float bounce, float drag, float angular_drag);
PhysicsComponent* PhysicsComponent_get(ComponentData* components, int entity);
void PhysicsComponent_remove(ComponentData* components, int entity);

ColliderComponent* ColliderComponent_add_circle(ComponentData* components, int entity, float radius, ColliderGroup group);
ColliderComponent* ColliderComponent_add_rectangle(ComponentData* components, int entity, float width, float height, ColliderGroup group);
ColliderComponent* ColliderComponent_get(ComponentData* components, int entity);
void ColliderComponent_remove(ComponentData* components, int entity);

PlayerComponent* PlayerComponent_add(ComponentData* components, int entity, int joystick);
PlayerComponent* PlayerComponent_get(ComponentData* components, int entity);
void PlayerComponent_remove(ComponentData* components, int entity);

LightComponent* LightComponent_add(ComponentData* components, int entity, float range, float angle, sfColor color, float brightness, float speed);
LightComponent* LightComponent_get(ComponentData* components, int entity);
void LightComponent_remove(ComponentData* components, int entity);

EnemyComponent* EnemyComponent_add(ComponentData* components, int entity);
EnemyComponent* EnemyComponent_get(ComponentData* components, int entity);
void EnemyComponent_remove(ComponentData* components, int entity);

ParticleComponent* ParticleComponent_add(ComponentData* components, int entity, float angle, float spread, float max_size, float min_size, float speed, float rate, sfColor color, sfColor inner_color);
ParticleComponent* ParticleComponent_get(ComponentData* components, int entity);
void ParticleComponent_remove(ComponentData* components, int entity);

VehicleComponent* VehicleComponent_add(ComponentData* components, int entity, float max_fuel);
VehicleComponent* VehicleComponent_get(ComponentData* components, int entity);
void VehicleComponent_remove(ComponentData* components, int entity);

WeaponComponent* WeaponComponent_add(ComponentData* components, int entity, float fire_rate, int damage, int magazine, float recoil_up, float recoil_down, float max_recoil, float range, float reload_time);
WeaponComponent* WeaponComponent_get(ComponentData* components, int entity);
void WeaponComponent_remove(ComponentData* components, int entity);

ItemComponent* ItemComponent_add(ComponentData* components, int entity, int size);
ItemComponent* ItemComponent_get(ComponentData* components, int entity);
void ItemComponent_remove(ComponentData* components, int entity);

WaypointComponent* WaypointComponent_add(ComponentData* components, int entity);
WaypointComponent* WaypointComponent_get(ComponentData* components, int entity);
void WaypointComponent_remove(ComponentData* components, int entity);

HealthComponent* HealthComponent_add(ComponentData* components, int entity, int health);
HealthComponent* HealthComponent_get(ComponentData* components, int entity);
void HealthComponent_remove(ComponentData* components, int entity);

CameraComponent* CameraComponent_add(ComponentData* components, int entity, sfVector2i resolution);
CameraComponent* CameraComponent_get(ComponentData* components, int entity);
void CameraComponent_remove(ComponentData* components, int entity);

RoadComponent* RoadComponent_add(ComponentData* components, int entity, float width, Filename filename);
RoadComponent* RoadComponent_get(ComponentData* components, int entity);
void RoadComponent_remove(ComponentData* components, int entity);

SoundComponent* SoundComponent_add(ComponentData* components, int entity, Filename hit_sound);
SoundComponent* SoundComponent_get(ComponentData* components, int entity);
void SoundComponent_remove(ComponentData* components, int entity);

int create_entity(ComponentData* components);
void destroy_entity(ComponentData* components, int i);

sfVector2f get_position(ComponentData* components, int i);
float get_angle(ComponentData* components, int i);
