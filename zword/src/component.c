#define _USE_MATH_DEFINES

#include <stdbool.h>
#include <stdlib.h>
#include <math.h>

#include <SFML/Graphics.h>
#include <SFML/System/Vector2.h>

#include "component.h"
#include "image.h"
#include "util.h"


CoordinateComponent* CoordinateComponent_create(sfVector2f pos, float angle) {
    CoordinateComponent* coord = malloc(sizeof(CoordinateComponent));
    coord->position = pos;
    coord->angle = angle;
    coord->parent = -1;
    return coord;
}


ImageComponent* ImageComponent_create(char filename[20], float scale) {
    ImageComponent* image = malloc(sizeof(ImageComponent));
    image->scale = (sfVector2f) { scale, scale };
    image->sprite = load_sprite(filename);
    image->shine = 0.0;
    return image;
}


PhysicsComponent* PhysicsComponent_create(float mass, float friction, float bounce, float drag) {
    PhysicsComponent* phys = malloc(sizeof(PhysicsComponent));
    phys->velocity = (sfVector2f) { 0, 0 };
    phys->acceleration = (sfVector2f) { 0, 0 };
    phys->collision.overlap = (sfVector2f) { 0, 0 };
    phys->collision.velocity = (sfVector2f) { 0.0, 0.0} ;
    phys->angular_velocity = 0.0;
    phys->angular_acceleration = 0.0;
    phys->mass = mass;
    phys->friction = friction;
    phys->bounce = bounce;
    phys->drag = drag;
    phys->max_speed = 20.0;
    phys->angular_drag = 2 * drag;
    phys->max_angular_speed = 2.5;
    return phys;
}


CircleColliderComponent* CircleColliderComponent_create(float radius) {
    CircleColliderComponent* col = malloc(sizeof(CircleColliderComponent));
    col->enabled = true;
    col->radius = radius;
    col->shape = sfCircleShape_create();
    sfCircleShape_setFillColor(col->shape, sfColor_fromRGB(150, 0, 150));
    return col;
}


RectangleColliderComponent* RectangleColliderComponent_create(float width, float height) {
    RectangleColliderComponent* col = malloc(sizeof(RectangleColliderComponent));
    col->enabled = true;
    col->width = width;
    col->height = height;
    col->shape = sfRectangleShape_create();
    sfRectangleShape_setFillColor(col->shape, sfColor_fromRGB(50, 50, 50));
    return col;
}


PlayerComponent* PlayerComponent_create() {
    PlayerComponent* player = malloc(sizeof(PlayerComponent));
    player->health = 100;
    player->acceleration = 20.0;
    player->vehicle = -1;
    player->item = 0;
    player->inventory_size = 4;
    for (int i = 0; i < player->inventory_size; i++) {
        player->inventory[i] = -1;
    }
    return player;
}


LightComponent* LightComponent_create(float range, float angle, int rays, sfColor color, float brightness) {
    LightComponent* light = malloc(sizeof(LightComponent));
    light->enabled = true;
    light->range = range;
    light->angle = angle;
    light->rays = rays;
    light->brightness = 0.0;
    light->max_brightness = brightness;
    light->smoothing = 0;
    light->shape = sfConvexShape_create();
    sfConvexShape_setFillColor(light->shape, color);
    sfConvexShape_setPointCount(light->shape, 3);
    light->shine = sfCircleShape_create();
    light->flicker = 0.1;
    return light;
}


EnemyComponent* EnemyComponent_create() {
    EnemyComponent* enemy = malloc(sizeof(EnemyComponent));
    enemy->health = 100;
    enemy->acceleration = 15.0;
    enemy->target = -1;
    return enemy;
}


ParticleComponent* ParticleComponent_create(float angle, float spread, float max_size, float min_size, float speed, float rate, sfColor color, sfColor inner_color) {
    ParticleComponent* particle = malloc(sizeof(ParticleComponent));
    particle->enabled = false;
    particle->loop = false;
    particle->angle = angle;
    particle->spread = spread;
    particle->particles = 0;
    particle->max_particles = 100;
    particle->iterator = 0;
    particle->max_size = max_size;
    particle->min_size = min_size;
    particle->speed = speed;
    particle->speed_spread = 0.5;
    particle->max_time = 0.5;
    for (int i = 0; i < particle->max_particles; i++) {
        particle->position[i] = (sfVector2f) { 0.0, 0.0 };
        particle->velocity[i] = (sfVector2f) { 0.0, 0.0 };
        particle->time[i] = 0.0;
    }
    particle->shape = sfCircleShape_create();
    particle->rate = rate;
    particle->timer = 0.0;
    particle->color = color;
    particle->inner_color = inner_color;
    return particle;
}


VehicleComponent* VehicleComponent_create() {
    VehicleComponent* vehicle = malloc(sizeof(VehicleComponent));
    vehicle->acceleration = 20.0;
    vehicle->max_speed = 10.0;
    vehicle->driver = -1;
    vehicle->turning = 50.0;
    return vehicle;
}


WeaponComponent* WeaponComponent_create(float fire_rate, float recoil_up, float recoil_down) {
    WeaponComponent* weapon = malloc(sizeof(WeaponComponent));
    weapon->cooldown = 0.0;
    weapon->fire_rate = fire_rate;
    weapon->recoil = 0.0;
    weapon->recoil_up = recoil_up;
    weapon->recoil_down = recoil_down;
    return weapon;
}


void destroy_entity(Component* component, int i) {
    if (component->coordinate[i]) {
        free(component->coordinate[i]);
        component->coordinate[i] = NULL;
    }
    if (component->image[i]) {
        free(component->image[i]);
        component->image[i] = NULL;
    }
    if (component->physics[i]) {
        free(component->physics[i]);
        component->physics[i] = NULL;
    }
    if (component->circle_collider[i]) {
        free(component->circle_collider[i]);
        component->circle_collider[i] = NULL;
    }
    if (component->rectangle_collider[i]) {
        free(component->rectangle_collider[i]);
        component->rectangle_collider[i] = NULL;
    }
    if (component->player[i]) {
        free(component->player[i]);
        component->player[i] = NULL;
    }
    if (component->light[i]) {
        free(component->light[i]);
        component->light[i] = NULL;
    }
    if (component->enemy[i]) {
        free(component->enemy[i]);
        component->enemy[i] = NULL;
    }
    if (component->particle[i]) {
        free(component->particle[i]);
        component->particle[i] = NULL;
    }
    if (component->weapon[i]) {
        free(component->weapon[i]);
        component->weapon[i] = NULL;
    }

    if (i == component->entities - 1) {
        component->entities--;
    }
}


sfVector2f get_position(Component* component, int i) {
    sfVector2f position = component->coordinate[i]->position;

    int parent = component->coordinate[i]->parent;
    if (parent != -1) {
        position = sum(get_position(component, parent), rotate(position, component->coordinate[parent]->angle));
    }
    return position;
}


float get_angle(Component* component, int i) {
    float angle = component->coordinate[i]->angle;

    int parent = component->coordinate[i]->parent;
    if (parent != -1) {
        angle += get_angle(component, parent);
    }
    return angle;
}
