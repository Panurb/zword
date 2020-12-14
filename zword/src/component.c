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


ComponentData* ComponentData_create() {
    ComponentData* components = malloc(sizeof(ComponentData));
    components->entities = 0;

    components->image.size = 0;
    components->image.max_index = -1;

    for (int i = 0; i < MAX_ENTITIES; i++) {
        components->image.array[i] = NULL;

        components->coordinate[i] = NULL;
        components->physics[i] = NULL;
        components->collider[i] = NULL;
        components->player[i] = NULL;
        components->light[i] = NULL;
        components->enemy[i] = NULL;
        components->particle[i] = NULL;
        components->vehicle[i] = NULL;
        components->weapon[i] = NULL;
        components->item[i] = NULL;
        components->waypoint[i] = NULL;
        components->health[i] = NULL;
    }
    return components;
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


void CoordinateComponent_remove(ComponentData* components, int entity) {
    free(CoordinateComponent_get(components, entity));
    components->coordinate[entity] = NULL;
}


ImageComponent* ImageComponent_add(ComponentData* components, int entity, Filename filename, float width, float height, int layer) {
    ImageComponent* image = malloc(sizeof(ImageComponent));
    image->texture_changed = true;
    image->outline = 0.0;
    strcpy(image->filename, filename);
    image->width = width;
    image->height = height;
    image->sprite = sfSprite_create();
    image->shine = 1.0;
    image->layer = layer;
    image->scale = ones();
    image->alpha = 1.0;
    
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


void ImageComponent_remove(ComponentData* components, int entity) {
    free(ImageComponent_get(components, entity));
    components->image.array[entity] = NULL;
    // TODO
}


PhysicsComponent* PhysicsComponent_add(ComponentData* components, int entity, float mass, float friction, float bounce, float drag, float angular_drag) {
    PhysicsComponent* phys = malloc(sizeof(PhysicsComponent));
    phys->velocity = zeros();
    phys->acceleration = zeros();
    phys->collision.collided = false;
    phys->collision.overlap = zeros();
    phys->collision.velocity = zeros();
    phys->angular_velocity = 0.0;
    phys->angular_acceleration = 0.0;
    phys->mass = mass;
    phys->friction = friction;
    phys->bounce = bounce;
    phys->drag = drag;
    phys->max_speed = 20.0;
    phys->angular_drag = angular_drag;
    phys->max_angular_speed = 20.0 * M_PI;

    components->physics[entity] = phys;

    return phys;
}


PhysicsComponent* PhysicsComponent_get(ComponentData* components, int entity) {
    if (entity == -1) return NULL;
    return components->physics[entity];
}


void PhysicsComponent_remove(ComponentData* components, int entity) {
    free(PhysicsComponent_get(components, entity));
    components->physics[entity] = NULL;
}


ColliderComponent* ColliderComponent_add_circle(ComponentData* components, int entity, float radius, ColliderGroup group) {
    ColliderComponent* col = malloc(sizeof(ColliderComponent));
    col->enabled = true;
    col->type = CIRCLE;
    col->group = group;
    col->last_collision = -1;
    col->radius = radius;
    col->width = 2 * radius;
    col->height = 2 * radius;

    components->collider[entity] = col;

    return col;
}


ColliderComponent* ColliderComponent_add_rectangle(ComponentData* components, int entity, float width, float height, ColliderGroup group) {
    ColliderComponent* col = malloc(sizeof(ColliderComponent));
    col->enabled = true;
    col->type = RECTANGLE;
    col->group = group;
    col->last_collision = -1;
    col->radius = sqrtf(width * width + height * height);
    col->width = width;
    col->height = height;

    components->collider[entity] = col;

    return col;
}


ColliderComponent* ColliderComponent_get(ComponentData* components, int entity) {
    if (entity == -1) return NULL;
    return components->collider[entity];
}


void ColliderComponent_remove(ComponentData* components, int entity) {
    free(ColliderComponent_get(components, entity));
    components->collider[entity] = NULL;
    // TODO: clear grid
}


PlayerComponent* PlayerComponent_add(ComponentData* components, int entity) {
    PlayerComponent* player = malloc(sizeof(PlayerComponent));
    player->target = -1;
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

    components->player[entity] = player;

    return player;
}


PlayerComponent* PlayerComponent_get(ComponentData* components, int entity) {
    if (entity == -1) return NULL;
    return components->player[entity];
}


void PlayerComponent_remove(ComponentData* components, int entity) {
    free(PlayerComponent_get(components, entity));
    components->player[entity] = NULL;
}


LightComponent* LightComponent_add(ComponentData* components, int entity, float range, float angle, int rays, sfColor color, float brightness, float speed) {
    LightComponent* light = malloc(sizeof(LightComponent));
    light->enabled = true;
    light->range = range;
    light->angle = angle;
    light->rays = rays;
    light->brightness = 0.0;
    light->max_brightness = brightness;
    light->smoothing = 0;

    light->color = color;

    light->verts = sfVertexArray_create();
    sfVertexArray_setPrimitiveType(light->verts, sfTriangleFan);
    sfVertexArray_resize(light->verts, light->rays + 1);

    light->shine = sfCircleShape_create();
    light->flicker = 0.0;
    light->speed = speed;
    light->time = 0.0;

    components->light[entity] = light;

    return light;
}


LightComponent* LightComponent_get(ComponentData* components, int entity) {
    if (entity == -1) return NULL;
    return components->light[entity];
}


void LightComponent_remove(ComponentData* components, int entity) {
    free(LightComponent_get(components, entity));
    components->light[entity] = NULL;
}


EnemyComponent* EnemyComponent_add(ComponentData* components, int entity) {
    EnemyComponent* enemy = malloc(sizeof(EnemyComponent));
    enemy->state = IDLE;
    enemy->acceleration = 15.0;
    enemy->target = -1;
    for (int i = 0; i < MAX_PATH_LENGTH; i++) {
        enemy->path[i] = -1;
    }
    enemy->fov = 0.25 * M_PI;
    enemy->vision_range = 15.0;

    components->enemy[entity] = enemy;

    return enemy;
}


EnemyComponent* EnemyComponent_get(ComponentData* components, int entity) {
    if (entity == -1) return NULL;
    return components->enemy[entity];
}


void EnemyComponent_remove(ComponentData* components, int entity) {
    free(EnemyComponent_get(components, entity));
    components->enemy[entity] = NULL;
}


ParticleComponent* ParticleComponent_add(ComponentData* components, int entity, float angle, float spread, float max_size, 
                                         float min_size, float speed, float rate, sfColor color, sfColor inner_color) {
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

    components->particle[entity] = particle;

    return particle;
}


ParticleComponent* ParticleComponent_get(ComponentData* components, int entity) {
    if (entity == -1) return NULL;
    return components->particle[entity];
}


void ParticleComponent_remove(ComponentData* components, int entity) {
    free(ParticleComponent_get(components, entity));
    components->particle[entity] = NULL;
}


VehicleComponent* VehicleComponent_add(ComponentData* components, int entity) {
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

    components->vehicle[entity] = vehicle;

    return vehicle;
}


VehicleComponent* VehicleComponent_get(ComponentData* components, int entity) {
    if (entity == -1) return NULL;
    return components->vehicle[entity];
}


void VehicleComponent_remove(ComponentData* components, int entity) {
    free(VehicleComponent_get(components, entity));
    components->vehicle[entity] = NULL;
}


WeaponComponent* WeaponComponent_add(ComponentData* components, int entity, float fire_rate, int damage, 
                                     int magazine, float recoil_up, float recoil_down, float max_recoil) {
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

    components->weapon[entity] = weapon;

    return weapon;
}


WeaponComponent* WeaponComponent_get(ComponentData* components, int entity) {
    if (entity == -1) return NULL;
    return components->weapon[entity];
}


void WeaponComponent_remove(ComponentData* components, int entity) {
    free(WeaponComponent_get(components, entity));
    components->weapon[entity] = NULL;
}


ItemComponent* ItemComponent_add(ComponentData* components, int entity, int size) {
    ItemComponent* item = malloc(sizeof(ItemComponent));
    item->size = size;
    for (int i = 0; i < size; i++) {
        item->attachments[i]= -1;
    }

    components->item[entity] = item;

    return item;
}


ItemComponent* ItemComponent_get(ComponentData* components, int entity) {
    if (entity == -1) return NULL;
    return components->item[entity];
}


void ItemComponent_remove(ComponentData* components, int entity) {
    free(ItemComponent_get(components, entity));
    components->item[entity] = NULL;
}


WaypointComponent* WaypointComponent_add(ComponentData* components, int entity) {
    WaypointComponent* waypoint = malloc(sizeof(WaypointComponent));
    waypoint->came_from = -1;
    waypoint->g_score = INFINITY;
    waypoint->f_score = INFINITY;
    for (int i = 0; i < MAX_NEIGHBORS; i++) {
        waypoint->neighbors[i] = -1;
        waypoint->weights[i] = 0.0;
    }
    waypoint->neighbors_size = 0;
    waypoint->range = 20.0;

    components->waypoint[entity] = waypoint;

    return waypoint;
}


WaypointComponent* WaypointComponent_get(ComponentData* components, int entity) {
    if (entity == -1) return NULL;
    return components->waypoint[entity];
}


void WaypointComponent_remove(ComponentData* components, int entity) {
    free(WaypointComponent_get(components, entity));
    components->waypoint[entity] = NULL;
}


HealthComponent* HealthComponent_add(ComponentData* components, int entity, int health) {
    HealthComponent* comp = malloc(sizeof(HealthComponent));
    comp->health = health;

    components->health[entity] = comp;

    return comp;
}


HealthComponent* HealthComponent_get(ComponentData* components, int entity) {
    if (entity == -1) return NULL;
    return components->health[entity];
}


void HealthComponent_remove(ComponentData* components, int entity) {
    free(HealthComponent_get(components, entity));
    components->health[entity] = NULL;
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


void destroy_entity(ComponentData* components, int entity) {
    CoordinateComponent_remove(components, entity);
    CoordinateComponent_remove(components, entity);
    PhysicsComponent_remove(components, entity);
    ColliderComponent_remove(components, entity);
    PlayerComponent_remove(components, entity);
    LightComponent_remove(components, entity);
    EnemyComponent_remove(components, entity);
    ParticleComponent_remove(components, entity);
    VehicleComponent_remove(components, entity);
    WeaponComponent_remove(components, entity);
    ItemComponent_remove(components, entity);
    WaypointComponent_remove(components, entity);
    HealthComponent_remove(components, entity);

    if (entity == components->entities - 1) {
        components->entities--;
    }
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
