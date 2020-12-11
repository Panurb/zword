#define _USE_MATH_DEFINES

#include <stdbool.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>

#include <SFML/Graphics.h>
#include <SFML/System/Vector2.h>

#include "component.h"
#include "image.h"
#include "util.h"


PhysicsComponent* PhysicsComponent_create(float mass, float friction, float bounce, float drag, float angular_drag) {
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
    phys->angular_drag = angular_drag;
    phys->max_angular_speed = 20.0 * M_PI;
    return phys;
}


ColliderComponent* ColliderComponent_create_circle(float radius) {
    ColliderComponent* col = malloc(sizeof(ColliderComponent));
    col->enabled = true;
    col->type = CIRCLE;
    col->last_collision = -1;
    col->radius = radius;
    col->width = 2 * radius;
    col->height = 2 * radius;
    return col;
}


ColliderComponent* ColliderComponent_create_rectangle(float width, float height) {
    ColliderComponent* col = malloc(sizeof(ColliderComponent));
    col->enabled = true;
    col->type = RECTANGLE;
    col->last_collision = -1;
    col->radius = sqrtf(width * width + height * height);
    col->width = width;
    col->height = height;
    return col;
}


PlayerComponent* PlayerComponent_create() {
    PlayerComponent* player = malloc(sizeof(PlayerComponent));
    player->acceleration = 20.0;
    player->vehicle = -1;
    player->item = 0;
    player->inventory_size = 4;
    for (int i = 0; i < player->inventory_size; i++) {
        player->inventory[i] = -1;
    }
    player->grabbed_item = -1;
    player->state = ON_FOOT;

    player->shape = sfConvexShape_create();
    sfConvexShape_setPointCount(player->shape, 4);

    player->line = sfRectangleShape_create();

    return player;
}


LightComponent* LightComponent_create(float range, float angle, int rays, sfColor color, float brightness, float speed) {
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
    light->speed = speed;
    return light;
}


EnemyComponent* EnemyComponent_create() {
    EnemyComponent* enemy = malloc(sizeof(EnemyComponent));
    enemy->state = IDLE;
    enemy->acceleration = 15.0;
    enemy->target = -1;
    for (int i = 0; i < MAX_PATH_LENGTH; i++) {
        enemy->path[i] = -1;
    }
    enemy->fov = 0.25 * M_PI;
    enemy->vision_range = 15.0;
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
    vehicle->turning = 50.0;
    for (int i = 0; i < 4; i++) {
        vehicle->riders[i] = -1;
    }
    vehicle->seats[0] = (sfVector2f) { 0.0, 0.75 };
    vehicle->seats[1] = (sfVector2f) { 0.0, -0.75 };
    vehicle->seats[2] = (sfVector2f) { -1.5, 0.75 };
    vehicle->seats[3] = (sfVector2f) { -1.5, -0.75 };
    return vehicle;
}


WeaponComponent* WeaponComponent_create(float fire_rate, int damage, int magazine, float recoil_up, float recoil_down, float max_recoil) {
    WeaponComponent* weapon = malloc(sizeof(WeaponComponent));
    weapon->cooldown = 0.0;
    weapon->fire_rate = fire_rate;
    weapon->recoil = 0.0;
    weapon->recoil_up = recoil_up;
    weapon->recoil_down = recoil_down;
    weapon->damage = damage;
    weapon->max_magazine = magazine;
    weapon->magazine = 0;
    weapon->max_recoil = max_recoil;
    weapon->reload_time = 2.0;
    weapon->reloading = false;

    weapon->shape = sfConvexShape_create();
    sfConvexShape_setPointCount(weapon->shape, 20);
    sfConvexShape_setOutlineColor(weapon->shape, sfWhite);
    sfColor color = sfWhite;
    color.a = 0;
    sfConvexShape_setFillColor(weapon->shape, color);

    return weapon;
}


ItemComponent* ItemComponent_create(int size) {
    ItemComponent* item = malloc(sizeof(ItemComponent));
    item->size = size;
    for (int i = 0; i < size; i++) {
        item->attachments[i]= -1;
    }
    return item;
}


WaypointComponent* WaypointComponent_create() {
    WaypointComponent* waypoint = malloc(sizeof(WaypointComponent));
    waypoint->came_from = -1;
    waypoint->g_score = INFINITY;
    waypoint->f_score = INFINITY;
    for (int i = 0; i < MAX_NEIGHBORS; i++) {
        waypoint->neighbors[i] = -1;
        waypoint->weights[i] = 0.0;
    }
    waypoint->neighbors_size = 0;
    waypoint->range = 1.0;
    return waypoint;
}


HealthComponent* HealthComponent_create(int health) {
    HealthComponent* comp = malloc(sizeof(HealthComponent));
    comp->health = health;
    return comp;
}


ComponentData* ComponentData_create() {
    ComponentData* component = malloc(sizeof(ComponentData));
    component->entities = 0;

    component->image.size = 0;
    component->image.max_index = -1;

    for (int i = 0; i < MAX_ENTITIES; i++) {
        component->image.array[i] = NULL;

        component->coordinate[i] = NULL;
        component->physics[i] = NULL;
        component->collider[i] = NULL;
        component->player[i] = NULL;
        component->light[i] = NULL;
        component->enemy[i] = NULL;
        component->particle[i] = NULL;
        component->vehicle[i] = NULL;
        component->weapon[i] = NULL;
        component->item[i] = NULL;
        component->waypoint[i] = NULL;
        component->health[i] = NULL;
    }
    return component;
}


CoordinateComponent* CoordinateComponent_add(ComponentData* components, int entity, sfVector2f pos, float angle) {
    CoordinateComponent* coord = malloc(sizeof(CoordinateComponent));
    coord->position = pos;
    coord->angle = angle;
    coord->parent = -1;

    components->coordinate[entity] = coord;

    return coord;
}


CoordinateComponent* CoordinateComponent_get(ComponentData* components, int entity) {
    if (entity == -1) return NULL;
    return components->coordinate[entity];
}


ImageComponent* ImageComponent_add(ComponentData* components, int entity, Filename filename, float width, float height, int layer) {
    ImageComponent* image = malloc(sizeof(ImageComponent));
    image->visible = true;
    image->texture_changed = true;
    strcpy(image->filename, filename);
    image->width = width;
    image->height = height;
    image->sprite = sfSprite_create();
    image->shine = 0.0;
    image->layer = layer;
    
    components->image.array[entity] = image;
    if (entity > components->image.max_index) {
        components->image.max_index = entity;
    }

    int i = components->image.size - 1;
    while (i >= 0 && components->image.array[components->image.order[i]]->layer > components->image.array[entity]->layer) {
        components->image.order[i + 1] = components->image.order[i];
        i--;
    }
    components->image.order[i + 1] = entity;

    components->image.size++;

    return image;
}


ImageComponent* ImageComponent_get(ComponentData* components, int entity) {
    if (entity == -1) return NULL;
    return components->image.array[entity];
}


int get_index(ComponentData* component) {
    for (int i = 0; i < component->entities; i++) {
        if (!component->coordinate[i]) {
            return i;
        }
    }

    component->entities++;
    return component->entities - 1;
}


void destroy_entity(ComponentData* component, int i) {
    /*
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
    if (component->collider[i]) {
        free(component->collider[i]);
        component->collider[i] = NULL;
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
    if (component->vehicle[i]) {
        free(component->vehicle[i]);
        component->vehicle[i] = NULL;
    }
    if (component->weapon[i]) {
        free(component->weapon[i]);
        component->weapon[i] = NULL;
    }
    if (component->item[i]) {
        free(component->item[i]);
        component->item[i] = NULL;
    }
    if (component->waypoint[i]) {
        free(component->waypoint[i]);
        component->waypoint[i] = NULL;
    }
    if (component->health[i]) {
        free(component->health[i]);
        component->health[i] = NULL;
    }

    if (i == component->entities - 1) {
        component->entities--;
    }
    */
}


sfVector2f get_position(ComponentData* component, int i) {
    sfVector2f position = component->coordinate[i]->position;

    int parent = component->coordinate[i]->parent;
    if (parent != -1) {
        position = sum(get_position(component, parent), rotate(position, component->coordinate[parent]->angle));
    }
    return position;
}


float get_angle(ComponentData* component, int i) {
    float angle = component->coordinate[i]->angle;

    int parent = component->coordinate[i]->parent;
    if (parent != -1) {
        angle += get_angle(component, parent);
    }

    return angle;
}
