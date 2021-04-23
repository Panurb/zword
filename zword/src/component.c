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
#include "grid.h"


ComponentData* ComponentData_create() {
    ComponentData* components = malloc(sizeof(ComponentData));
    components->entities = 0;

    components->image.size = 0;
    components->player.size = 0;

    for (int i = 0; i < MAX_ENTITIES; i++) {
        components->image.array[i] = NULL;
        components->coordinate[i] = NULL;
        components->physics[i] = NULL;
        components->collider[i] = NULL;
        components->player.array[i] = NULL;
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

    int i = components->image.size - 1;
    while (i >= 0 && ImageComponent_get(components, components->image.order[i])->layer > layer) {
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
    sfSprite_destroy(ImageComponent_get(components, entity)->sprite);

    free(ImageComponent_get(components, entity));
    components->image.array[entity] = NULL;

    int i = find(entity, components->image.order, components->image.size);
    while (i < components->image.size - 2) {
        components->image.order[i] = components->image.order[i + 1];
        i++;
    }
    components->image.size--;
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
    phys->drag_sideways = drag;
    phys->speed = 0.0;
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
}


PlayerComponent* PlayerComponent_add(ComponentData* components, int entity, int joystick) {
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
    player->ammo_size = 3;
    for (int i = 0; i < player->ammo_size; i++) {
        player->ammo[i] = 0;
    }

    player->shape = sfConvexShape_create();
    sfConvexShape_setPointCount(player->shape, 4);

    player->line = sfRectangleShape_create();
    player->controller.joystick = joystick;

    if (joystick == -1) {
        int buttons[12] = { sfKeyE, sfKeyQ, sfKeyR, sfKeyF, sfKeyLShift, -1, sfKeyEscape, sfKeyEnter, sfKeyZ, sfKeyX, sfKeySpace, -1 };
        memcpy(player->controller.buttons, buttons, sizeof(buttons));
    } else if (strstr(sfJoystick_getIdentification(joystick).name, "Xbox")) {
        int buttons[12] = { 0, 1, 2, 3, 4, 5, 6, 7, 10, 11, 8, 9 };
        memcpy(player->controller.buttons, buttons, sizeof(buttons));
        int axes[8] = { 0, 1, 4, 5, 2, 2, 6, 7 };
        memcpy(player->controller.axes, axes, sizeof(axes));
    } else {
        int buttons[12] = { 1, 2, 0, 3, 4, 5, 9, 8, 10, 11, 6, 7 };
        memcpy(player->controller.buttons, buttons, sizeof(buttons));
        int axes[8] = { 0, 1, 2, 3, 4, 5, 6, 7 };
        memcpy(player->controller.axes, axes, sizeof(axes));
    }

    for (int i = 0; i < 12; i++) {
        player->controller.buttons_down[i] = false;
        player->controller.buttons_pressed[i] = false;
        player->controller.buttons_released[i] = false;
    }

    components->player.array[entity] = player;
    components->player.order[components->player.size] = entity;
    components->player.size++;

    return player;
}


PlayerComponent* PlayerComponent_get(ComponentData* components, int entity) {
    if (entity == -1) return NULL;
    return components->player.array[entity];
}


void PlayerComponent_remove(ComponentData* components, int entity) {
    PlayerComponent* player = PlayerComponent_get(components, entity);
    sfConvexShape_destroy(player->shape);
    sfRectangleShape_destroy(player->line);
    free(player);
    components->player.array[entity] = NULL;
    // TODO
}


LightComponent* LightComponent_add(ComponentData* components, int entity, float range, float angle, sfColor color, float brightness, float speed) {
    LightComponent* light = malloc(sizeof(LightComponent));
    light->enabled = true;
    light->range = range;
    light->angle = angle;
    light->rays = angle * 50;
    light->brightness = 0.0;
    light->max_brightness = brightness;

    light->color = color;

    light->verts = sfVertexArray_create();
    sfVertexArray_setPrimitiveType(light->verts, sfTriangleFan);
    sfVertexArray_resize(light->verts, light->rays + 1);

    light->shine = sfCircleShape_create();
    light->flicker = 0.0;
    light->speed = speed;
    light->time = randf(0.0, 1.0);

    components->light[entity] = light;

    return light;
}


LightComponent* LightComponent_get(ComponentData* components, int entity) {
    if (entity == -1) return NULL;
    return components->light[entity];
}


void LightComponent_remove(ComponentData* components, int entity) {
    LightComponent* light = LightComponent_get(components, entity);
    sfVertexArray_destroy(light->verts);
    sfCircleShape_destroy(light->shine);
    free(light);
    components->light[entity] = NULL;
}


EnemyComponent* EnemyComponent_add(ComponentData* components, int entity) {
    EnemyComponent* enemy = malloc(sizeof(EnemyComponent));
    enemy->state = IDLE;
    enemy->acceleration = 15.0;
    enemy->target = -1;
    enemy->path = List_create();
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
    EnemyComponent* enemy = EnemyComponent_get(components, entity);
    List_delete(enemy->path);
    free(enemy);
    components->enemy[entity] = NULL;
}


ParticleComponent* ParticleComponent_add(ComponentData* components, int entity, float angle, float spread, float max_size, 
                                         float min_size, float speed, float rate, sfColor start_color, sfColor end_color) {
    ParticleComponent* particle = malloc(sizeof(ParticleComponent));
    particle->enabled = false;
    particle->loop = false;
    particle->angle = angle;
    particle->spread = spread;
    particle->particles = 0;
    particle->max_particles = 100;
    particle->iterator = 0;
    particle->start_size = max_size;
    particle->end_size = min_size;
    particle->speed = speed;
    particle->speed_spread = 0.5;
    particle->max_time = 0.5;
    for (int i = 0; i < particle->max_particles; i++) {
        particle->position[i] = zeros();
        particle->velocity[i] = zeros();
        particle->time[i] = 0.0;
    }
    particle->shape = sfCircleShape_create();
    particle->rate = rate;
    particle->timer = 0.0;
    particle->start_color = start_color;
    particle->end_color = end_color;
    particle->origin = zeros();

    components->particle[entity] = particle;

    return particle;
}


ParticleComponent* ParticleComponent_get(ComponentData* components, int entity) {
    if (entity == -1) return NULL;
    return components->particle[entity];
}


void ParticleComponent_remove(ComponentData* components, int entity) {
    ParticleComponent* particle = ParticleComponent_get(components, entity);
    sfCircleShape_destroy(particle->shape);
    free(particle);
    components->particle[entity] = NULL;
}


VehicleComponent* VehicleComponent_add(ComponentData* components, int entity, float max_fuel) {
    VehicleComponent* vehicle = malloc(sizeof(VehicleComponent));
    vehicle->on_road = false;
    vehicle->max_fuel = max_fuel;
    vehicle->fuel = max_fuel;
    vehicle->acceleration = 20.0;
    vehicle->max_speed = 10.0;
    vehicle->turning = 50.0;
    vehicle->size = 4;
    for (int i = 0; i < vehicle->size; i++) {
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


WeaponComponent* WeaponComponent_add(ComponentData* components, int entity, float fire_rate, int damage, int shots,
                                     float spread, int magazine, float recoil, float range, float reload_time, AmmoType ammo_type, Filename sound) {
    WeaponComponent* weapon = malloc(sizeof(WeaponComponent));
    weapon->cooldown = 0.0;
    weapon->fire_rate = fire_rate;
    weapon->recoil = 0.0;
    weapon->recoil_up = recoil;
    weapon->recoil_down = 3.0f * recoil;
    weapon->damage = damage;
    weapon->max_magazine = magazine;
    weapon->magazine = magazine;
    if (magazine == -1) {
        weapon->magazine = 1;
    }
    weapon->max_recoil = recoil * M_PI;
    weapon->reload_time = reload_time;
    weapon->reloading = false;
    weapon->range = range;
    weapon->sound_range = range;
    weapon->spread = spread;
    weapon->shots = shots;
    weapon->ammo_type = ammo_type;
    strcpy(weapon->sound, sound);
    weapon->automatic = false;

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
    waypoint->neighbors = List_create();
    waypoint->range = 12.0;

    components->waypoint[entity] = waypoint;

    return waypoint;
}


WaypointComponent* WaypointComponent_get(ComponentData* components, int entity) {
    if (entity == -1) return NULL;
    return components->waypoint[entity];
}


void WaypointComponent_remove(ComponentData* components, int entity) {
    WaypointComponent* waypoint = WaypointComponent_get(components, entity);
    for (ListNode* current = waypoint->neighbors->head; current; current = current->next) {
        int n = current->value;
        List_remove(WaypointComponent_get(components, n)->neighbors, entity);
    }
    List_delete(waypoint->neighbors);
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


CameraComponent* CameraComponent_add(ComponentData* components, int entity, sfVector2i resolution) {
    CameraComponent* camera = malloc(sizeof(CameraComponent));

    camera->resolution = resolution;
    camera->zoom = 25.0 * camera->resolution.y / 720.0;

    camera->shaders[0] = NULL;
    camera->shaders[1] = sfShader_createFromFile(NULL, NULL, "outline.frag");

    camera->fonts[0] = sfFont_createFromFile("data/Helvetica.ttf");

    components->camera[entity] = camera;
    return camera;
}


CameraComponent* CameraComponent_get(ComponentData* components, int entity) {
    if (entity == -1) return NULL;
    return components->camera[entity];
}


void CameraComponent_remove(ComponentData* components, int entity) {
    free(CameraComponent_get(components, entity));
    components->camera[entity] = NULL;
}


RoadComponent* RoadComponent_add(ComponentData* components, int entity, float width, Filename filename) {
    RoadComponent* road = malloc(sizeof(RoadComponent));
    road->prev = -1;
    road->next = -1;
    road->curve = 0.0;
    road->width = width;
    road->shape = sfConvexShape_create();
    road->points = 12;
    sfConvexShape_setPointCount(road->shape, road->points);
    strcpy(road->filename, filename);
    road->texture_changed = true;

    components->road[entity] = road;
    return road;
}


RoadComponent* RoadComponent_get(ComponentData* components, int entity) {
    if (entity == -1) return NULL;
    return components->road[entity];
}


void RoadComponent_remove(ComponentData* components, int entity) {
    RoadComponent* road = RoadComponent_get(components, entity);
    sfConvexShape_destroy(road->shape);
    free(road);
    components->road[entity] = NULL;
}


SoundComponent* SoundComponent_add(ComponentData* components, int entity, Filename hit_sound) {
    SoundComponent* sound = malloc(sizeof(SoundComponent));
    sound->size = 4;
    for (int i = 0; i < sound->size; i++) {
        sound->events[i] = NULL;
    }
    strcpy(sound->hit_sound, hit_sound);
    components->sound[entity] = sound;
    return sound;
}


SoundComponent* SoundComponent_get(ComponentData* components, int entity) {
    if (entity == -1) return NULL;
    return components->sound[entity];
}


void SoundComponent_remove(ComponentData* components, int entity) {
    free(SoundComponent_get(components, entity));
    components->sound[entity] = NULL;
}


AmmoComponent* AmmoComponent_add(ComponentData* components, int entity, AmmoType type) {
    AmmoComponent* ammo = malloc(sizeof(AmmoComponent));
    ammo->type = type;
    switch (type) {
        case AMMO_PISTOL:
            ammo->size = 12;
            break;
        case AMMO_RIFLE:
            ammo->size = 30;
            break;
        case AMMO_SHOTGUN:
            ammo->size = 8;
            break;
        default:
            ammo->size = 0;
            break;
    }

    components->ammo[entity] = ammo;
    return ammo;
}


AmmoComponent* AmmoComponent_get(ComponentData* components, int entity) {
    if (entity == -1) return NULL;
    return components->ammo[entity];
}


void AmmoComponent_remove(ComponentData* components, int entity) {
    AmmoComponent* ammo = AmmoComponent_get(components, entity);
    if (ammo) {
        free(ammo);
        components->ammo[entity] = NULL;
    }
}


int create_entity(ComponentData* components) {
    for (int i = 0; i < components->entities; i++) {
        if (!components->coordinate[i]) {
            return i;
        }
    }

    components->entities++;
    return components->entities - 1;
}


void destroy_entity(ComponentData* components, int entity) {
    if (CoordinateComponent_get(components, entity))
        CoordinateComponent_remove(components, entity);
    if (ImageComponent_get(components, entity))
        ImageComponent_remove(components, entity);
    if (PhysicsComponent_get(components, entity))
        PhysicsComponent_remove(components, entity);
    if (ColliderComponent_get(components, entity))
        ColliderComponent_remove(components, entity);
    if (PlayerComponent_get(components, entity))
        PlayerComponent_remove(components, entity);
    if (LightComponent_get(components, entity))
        LightComponent_remove(components, entity);
    if (EnemyComponent_get(components, entity))
        EnemyComponent_remove(components, entity);
    if (ParticleComponent_get(components, entity))
        ParticleComponent_remove(components, entity);
    if (VehicleComponent_get(components, entity))
        VehicleComponent_remove(components, entity);
    if (WeaponComponent_get(components, entity))
        WeaponComponent_remove(components, entity);
    if (ItemComponent_get(components, entity))
        ItemComponent_remove(components, entity);
    if (WaypointComponent_get(components, entity))
        WaypointComponent_remove(components, entity);
    if (HealthComponent_get(components, entity))
        HealthComponent_remove(components, entity);
    if (CameraComponent_get(components, entity))
        CameraComponent_remove(components, entity);
    if (RoadComponent_get(components, entity))
        RoadComponent_remove(components, entity);
    if (SoundComponent_get(components, entity))
        SoundComponent_remove(components, entity);
    AmmoComponent_remove(components, entity);

    if (entity == components->entities - 1) {
        components->entities--;
    }
}


void ComponentData_clear(ComponentData* components) {
    for (int i = 0; i < components->entities; i++) {
        destroy_entity(components, i);
    }
    components->entities = 0;
}


sfVector2f get_position(ComponentData* components, int entity) {
    sfVector2f position = components->coordinate[entity]->position;

    int parent = components->coordinate[entity]->parent;
    if (parent != -1) {
        position = sum(get_position(components, parent), rotate(position, components->coordinate[parent]->angle));
    }
    return position;
}


float get_angle(ComponentData* components, int entity) {
    float angle = components->coordinate[entity]->angle;

    int parent = components->coordinate[entity]->parent;
    if (parent != -1) {
        angle += get_angle(components, parent);
    }

    return angle;
}
