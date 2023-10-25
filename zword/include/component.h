#pragma once

#include <stdbool.h>

#include <SFML/Graphics.h>
#include <SFML/Audio.h>

#include "util.h"
#include "list.h"

#define MAX_ENTITIES 10000


typedef struct {
    sfVector2f position;
    float angle;
    int parent;
    List* children;
} CoordinateComponent;

typedef enum {
    LAYER_GROUND,
    LAYER_ROADS,
    LAYER_FLOOR,
    LAYER_DECALS,
    LAYER_SHADOWS,
    LAYER_WALLS,
    LAYER_CORPSES,
    LAYER_ITEMS,
    LAYER_VEHICLES,
    LAYER_ENEMIES,
    LAYER_WEAPONS,
    LAYER_PLAYERS,
    LAYER_TREES,
    LAYER_PARTICLES,
    LAYER_ROOFS,
    LAYER_MENU
} Layer;

typedef struct {
    bool texture_changed;
    Filename filename;
    float width;
    float height;
    sfSprite* sprite;
    float shine;
    Layer layer;
    sfVector2f scale;
    float alpha;
} ImageComponent;

typedef struct {
    sfVector2f velocity;
    sfVector2f acceleration;
    struct {
        List* entities;
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
    float lifetime;
} PhysicsComponent;

typedef enum {
    COLLIDER_CIRCLE,
    COLLIDER_RECTANGLE
} ColliderType;

typedef enum {
    GROUP_ALL,
    GROUP_WALLS,
    GROUP_ITEMS,
    GROUP_PLAYERS,
    GROUP_ENEMIES,
    GROUP_VEHICLES,
    GROUP_TREES,
    GROUP_ROADS,
    GROUP_BARRIERS,
    GROUP_BULLETS,
    GROUP_LIGHTS,
    GROUP_CORPSES,
    GROUP_FLOORS,
    GROUP_WAYPOINTS,
    GROUP_RAYS,
    GROUP_DEBRIS,
    GROUP_DOORS,
    GROUP_ENERGY
} ColliderGroup;

typedef struct {
    bool enabled;
    ColliderGroup group;
    ColliderType type;
    int last_collision;
    float radius;
    float width;
    float height;
    sfVertexArray* verts;
    int verts_size;
} ColliderComponent;

typedef enum {
    PLAYER_ON_FOOT,
    PLAYER_SHOOT,
    PLAYER_RELOAD,
    PLAYER_ENTER,
    PLAYER_DRIVE,
    PLAYER_PASSENGER,
    PLAYER_MENU,
    PLAYER_MENU_GRAB,
    PLAYER_MENU_DROP,
    PLAYER_AMMO_MENU,
    PLAYER_DEAD,
    PLAYER_PICK_UP
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
    int ammo_size;
    int ammo[4];
    sfCircleShape* crosshair;
    int arms;
    int money;
    float use_timer;
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
    ENEMY_IDLE,
    ENEMY_INVESTIGATE,
    ENEMY_CHASE,
    ENEMY_ATTACK,
    ENEMY_DEAD
} EnemyState;

typedef struct {
    bool spawner;
    EnemyState state;
    float acceleration;
    int target;
    List* path;
    float fov;
    float vision_range;
    float idle_speed;
    float walk_speed;
    float run_speed;
    int weapon;
    float desired_angle;
    float attack_delay;
    float attack_timer;
    float turn_speed;
    int bounty;
} EnemyComponent;

typedef enum ParticleType {
    PARTICLE_NONE,
    PARTICLE_BULLET,
    PARTICLE_BLOOD,
    PARTICLE_SPARKS,
    PARTICLE_DIRT,
    PARTICLE_ROCK,
    PARTICLE_SPLINTER,
    PARTICLE_FIRE,
    PARTICLE_ENERGY
} ParticleType;

typedef struct {
    ParticleType type;
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
    sfColor outer_color;
    sfColor inner_color;
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

typedef enum {
    AMMO_MELEE,
    AMMO_PISTOL,
    AMMO_RIFLE,
    AMMO_SHOTGUN,
    AMMO_ENERGY,
    AMMO_ROPE
} AmmoType;

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
    int shots;
    bool automatic;
    AmmoType ammo_type;
    Filename sound;
} WeaponComponent;

typedef enum ItemType {
    ITEM_HEAL,
    ITEM_LIGHT
} ItemType;

typedef struct {
    int attachments[5];
    int size;
    int price;
    ButtonText name;
    float use_time;
    ItemType type;
    int value;
} ItemComponent;

typedef struct {
    List* neighbors;
    int came_from;
    float f_score;
    float g_score;
    float range;
} WaypointComponent;

typedef struct {
    int health;
    Filename dead_image;
    Filename decal;
    Filename die_sound;
} HealthComponent;

typedef struct {
    sfVector2i resolution;
    float zoom;
    float zoom_target;
    sfShader* shaders[10];
    sfFont* fonts[1];
    Matrix2f matrix;
    Matrix2f inv_matrix;
    sfView* view;
    struct {
        sfVector2f position;
        sfVector2f velocity;
    } shake;
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
    AmmoType type;
    int size;
} AmmoComponent;

typedef struct {
    int frames;
    int current_frame;
    float timer;
    float framerate;
    bool play_once;
} AnimationComponent;

typedef struct {
    bool locked;
    sfVector2f anchor;
    sfVector2f direction;
} DoorComponent;

typedef struct {
    int parent;
    float min_length;
    float max_length;
    float strength;
    float max_angle;
} JointComponent;

typedef enum {
    WIDGET_WINDOW,
    WIDGET_CONTAINER,
    WIDGET_LABEL,
    WIDGET_BUTTON,
    WIDGET_SPINBOX,
    WIDGET_SLIDER,
    WIDGET_DROPDOWN,
    WIDGET_SCROLLBAR,
    WIDGET_TEXTBOX
} WidgetType;

typedef struct ComponentData ComponentData;

typedef void (*OnClick)(ComponentData*, int);
typedef void (*OnChange)(ComponentData*, int, int);

typedef struct {
    bool enabled;
    WidgetType type;
    bool selected;
    ButtonText string;
    sfText* text;
    OnClick on_click;
    OnChange on_change;
    int value;
    int min_value;
    int max_value;
    bool cyclic;
    ButtonText* strings;
} WidgetComponent;

typedef struct {
    void* array[MAX_ENTITIES];
    List* order;
} OrderedArray;

struct ComponentData {
    int entities;
    List* added_entities;
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
    AmmoComponent* ammo[MAX_ENTITIES];
    AnimationComponent* animation[MAX_ENTITIES];
    DoorComponent* door[MAX_ENTITIES];
    JointComponent* joint[MAX_ENTITIES];
    OrderedArray widget;
};

ComponentData* ComponentData_create();

CoordinateComponent* CoordinateComponent_add(ComponentData* components, int entity, sfVector2f pos, float angle);
CoordinateComponent* CoordinateComponent_get(ComponentData* components, int entity);
void CoordinateComponent_remove(ComponentData* components, int entity);

ImageComponent* ImageComponent_add(ComponentData* components, int entity, Filename filename, float width, float height, Layer layer);
ImageComponent* ImageComponent_get(ComponentData* components, int entity);
void ImageComponent_remove(ComponentData* components, int entity);

PhysicsComponent* PhysicsComponent_add(ComponentData* components, int entity, float mass);
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

ParticleComponent* ParticleComponent_add(ComponentData* components, int entity, float angle, float spread, float start_size, float end_size, float speed, float rate, sfColor outer_color, sfColor inner_color);
ParticleComponent* ParticleComponent_get(ComponentData* components, int entity);
void ParticleComponent_remove(ComponentData* components, int entity);

VehicleComponent* VehicleComponent_add(ComponentData* components, int entity, float max_fuel);
VehicleComponent* VehicleComponent_get(ComponentData* components, int entity);
void VehicleComponent_remove(ComponentData* components, int entity);

WeaponComponent* WeaponComponent_add(ComponentData* components, int entity, float fire_rate, int damage, int shots, float spread, int magazine, float recoil, float range, float reload_time, AmmoType ammo_type, Filename sound);
WeaponComponent* WeaponComponent_get(ComponentData* components, int entity);
void WeaponComponent_remove(ComponentData* components, int entity);

ItemComponent* ItemComponent_add(ComponentData* components, int entity, int size, int price, ButtonText name);
ItemComponent* ItemComponent_get(ComponentData* components, int entity);
void ItemComponent_remove(ComponentData* components, int entity);

WaypointComponent* WaypointComponent_add(ComponentData* components, int entity);
WaypointComponent* WaypointComponent_get(ComponentData* components, int entity);
void WaypointComponent_remove(ComponentData* components, int entity);

HealthComponent* HealthComponent_add(ComponentData* components, int entity, int health, Filename dead_image, Filename decal, Filename die_sound);
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

AmmoComponent* AmmoComponent_add(ComponentData* components, int entity, AmmoType type);
AmmoComponent* AmmoComponent_get(ComponentData* components, int entity);
void AmmoComponent_remove(ComponentData* components, int entity);

AnimationComponent* AnimationComponent_add(ComponentData* components, int entity, int frames);
AnimationComponent* AnimationComponent_get(ComponentData* components, int entity);
void AnimationComponent_remove(ComponentData* components, int entity);

DoorComponent* DoorComponent_add(ComponentData* components, int entity, sfVector2f anchor);
DoorComponent* DoorComponent_get(ComponentData* components, int entity);
void DoorComponent_remove(ComponentData* components, int entity);

JointComponent* JointComponent_add(ComponentData* components, int entity, int parent, float min_length, float max_length, float strength);
JointComponent* JointComponent_get(ComponentData* components, int entity);
void JointComponent_remove(ComponentData* components, int entity);

WidgetComponent* WidgetComponent_add(ComponentData* components, int entity, ButtonText text, WidgetType type);
WidgetComponent* WidgetComponent_get(ComponentData* components, int entity);
void WidgetComponent_remove(ComponentData* components, int entity);

int create_entity(ComponentData* components);
void destroy_entity(ComponentData* components, int i);
void destroy_entity_recursive(ComponentData* components, int entity);
int get_root(ComponentData* components, int entity);
void add_child(ComponentData* components, int parent, int child);
void remove_children(ComponentData* components, int parent);

void ComponentData_clear(ComponentData* components);

sfVector2f get_position(ComponentData* components, int i);
float get_angle(ComponentData* components, int i);

bool entity_exists(ComponentData* components, int entity);
