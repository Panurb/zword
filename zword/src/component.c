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
    components->added_entities = NULL;

    components->image.order = List_create();
    components->player.order = List_create();
    components->widget.order = List_create();

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
        components->door[i] = NULL;
        components->joint[i] = NULL;
        components->widget.array[i] = NULL;
    }
    return components;
}


CoordinateComponent* CoordinateComponent_add(ComponentData* components, int entity, sfVector2f pos, float angle) {
    CoordinateComponent* coord = malloc(sizeof(CoordinateComponent));
    coord->position = pos;
    coord->angle = mod(angle, 2.0f * M_PI);
    coord->parent = -1;
    coord->children = List_create();

    components->coordinate[entity] = coord;

    return coord;
}


CoordinateComponent* CoordinateComponent_get(ComponentData* components, int entity) {
    if (entity == -1) return NULL;
    return components->coordinate[entity];
}


void CoordinateComponent_remove(ComponentData* components, int entity) {
    CoordinateComponent* coord = CoordinateComponent_get(components, entity);
    if (coord) {
        // if (coord->parent != -1) {
        //     List_remove(CoordinateComponent_get(components, coord->parent)->children, entity);
        // }
        for (ListNode* node = coord->children->head; node; node = node->next) {
            CoordinateComponent* child = CoordinateComponent_get(components, node->value);
            if (child) {
                child->parent = -1;
            }
        }
        List_delete(coord->children);
        free(coord);
        components->coordinate[entity] = NULL;
    }
}


ImageComponent* ImageComponent_add(ComponentData* components, int entity, Filename filename, float width, float height, Layer layer) {
    ImageComponent* image = malloc(sizeof(ImageComponent));
    image->texture_changed = true;
    strcpy(image->filename, filename);
    image->width = width;
    image->height = height;
    image->sprite = sfSprite_create();
    image->shine = 1.0;
    image->layer = layer;
    image->scale = ones();
    image->alpha = 1.0;
    
    components->image.array[entity] = image;

    change_layer(components, entity, image->layer);

    return image;
}


ImageComponent* ImageComponent_get(ComponentData* components, int entity) {
    if (entity == -1) return NULL;
    return components->image.array[entity];
}


void ImageComponent_remove(ComponentData* components, int entity) {
    ImageComponent* image = ImageComponent_get(components, entity);
    if (image) {
        sfSprite_destroy(image->sprite);

        free(image);
        components->image.array[entity] = NULL;

        List_remove(components->image.order, entity);
    }
}


PhysicsComponent* PhysicsComponent_add(ComponentData* components, int entity, float mass) {
    PhysicsComponent* phys = malloc(sizeof(PhysicsComponent));
    phys->velocity = zeros();
    phys->acceleration = zeros();
    phys->collision.entities = List_create();
    phys->collision.overlap = zeros();
    phys->collision.velocity = zeros();
    phys->angular_velocity = 0.0f;
    phys->angular_acceleration = 0.0f;
    phys->mass = mass;
    phys->friction = 0.0f;
    phys->bounce = 0.5f;
    phys->drag = 10.0f;
    phys->drag_sideways = 10.0f;
    phys->speed = 0.0;
    phys->max_speed = 20.0;
    phys->angular_drag = 10.0f;
    phys->max_angular_speed = 20.0 * M_PI;
    phys->lifetime = INFINITY;

    components->physics[entity] = phys;

    return phys;
}


PhysicsComponent* PhysicsComponent_get(ComponentData* components, int entity) {
    if (entity == -1) return NULL;
    return components->physics[entity];
}


void PhysicsComponent_remove(ComponentData* components, int entity) {
    PhysicsComponent* phys = PhysicsComponent_get(components, entity);
    if (phys) {
        List_delete(phys->collision.entities);
        free(phys);
        components->physics[entity] = NULL;
    }
}


ColliderComponent* ColliderComponent_add_circle(ComponentData* components, int entity, float radius, ColliderGroup group) {
    ColliderComponent* col = malloc(sizeof(ColliderComponent));
    col->enabled = true;
    col->type = COLLIDER_CIRCLE;
    col->group = group;
    col->last_collision = -1;
    col->radius = radius;
    col->width = 2 * radius;
    col->height = 2 * radius;

    col->verts_size = 21;
    col->verts = sfVertexArray_create();
    sfVertexArray_setPrimitiveType(col->verts, sfTriangleFan);
    sfVertexArray_resize(col->verts, col->verts_size);

    components->collider[entity] = col;

    return col;
}


ColliderComponent* ColliderComponent_add_rectangle(ComponentData* components, int entity, float width, float height, ColliderGroup group) {
    ColliderComponent* col = malloc(sizeof(ColliderComponent));
    col->enabled = true;
    col->type = COLLIDER_RECTANGLE;
    col->group = group;
    col->last_collision = -1;
    col->radius = sqrtf(width * width + height * height);
    col->width = width;
    col->height = height;

    col->verts_size = 6;
    col->verts = sfVertexArray_create();
    sfVertexArray_setPrimitiveType(col->verts, sfTriangleFan);
    sfVertexArray_resize(col->verts, col->verts_size);

    components->collider[entity] = col;

    return col;
}


ColliderComponent* ColliderComponent_get(ComponentData* components, int entity) {
    if (entity == -1) return NULL;
    return components->collider[entity];
}


void ColliderComponent_remove(ComponentData* components, int entity) {
    ColliderComponent* col = ColliderComponent_get(components, entity);
    if (col) {
        free(col);
        components->collider[entity] = NULL;
    }
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
    player->state = PLAYER_ON_FOOT;
    player->ammo_size = 4;
    for (int i = 0; i < player->ammo_size; i++) {
        player->ammo[i] = -1;
    }
    player->arms = -1;
    player->money = 0;
    player->use_timer = 0.0f;

    player->shape = sfConvexShape_create();
    sfConvexShape_setPointCount(player->shape, 4);

    player->line = sfRectangleShape_create();
    player->controller.joystick = joystick;

    if (joystick == -1) {
        int buttons[12] = { sfKeyE, sfKeyQ, sfKeyR, sfKeyF, sfKeyLShift, -1, sfKeyEscape, -1, sfKeyLAlt, -1, -1, -1 };
        memcpy(player->controller.buttons, buttons, sizeof(buttons));
        int axes[8] = { sfKeyA, sfKeyD, sfKeyS, sfKeyW, -1, -1, -1, -1 };
        memcpy(player->controller.axes, axes, sizeof(axes));
    } else if (strstr(sfJoystick_getIdentification(joystick).name, "Xbox")) {
        int buttons[12] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11 };
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

    player->crosshair = sfCircleShape_create();
    sfCircleShape_setOutlineThickness(player->crosshair, 1.0f);

    components->player.array[entity] = player;
    List_add(components->player.order, entity);

    return player;
}


PlayerComponent* PlayerComponent_get(ComponentData* components, int entity) {
    if (entity == -1) return NULL;
    return components->player.array[entity];
}


void PlayerComponent_remove(ComponentData* components, int entity) {
    PlayerComponent* player = PlayerComponent_get(components, entity);
    if (player) {
        sfConvexShape_destroy(player->shape);
        sfRectangleShape_destroy(player->line);
        free(player);
        components->player.array[entity] = NULL;
        List_remove(components->player.order, entity);
    }
}


LightComponent* LightComponent_add(ComponentData* components, int entity, float range, float angle, sfColor color, float brightness, float speed) {
    LightComponent* light = malloc(sizeof(LightComponent));
    light->enabled = true;
    light->range = range;
    light->angle = angle;
    light->rays = angle * 100;
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
    if (light) {
        sfVertexArray_destroy(light->verts);
        sfCircleShape_destroy(light->shine);
        free(light);
        components->light[entity] = NULL;
    }
}


EnemyComponent* EnemyComponent_add(ComponentData* components, int entity) {
    EnemyComponent* enemy = malloc(sizeof(EnemyComponent));
    enemy->state = ENEMY_IDLE;
    enemy->acceleration = 15.0f;
    enemy->target = -1;
    enemy->path = List_create();
    enemy->fov = 0.5f * M_PI;
    enemy->vision_range = 15.0f;
    enemy->idle_speed = 1.0f;
    enemy->walk_speed = 2.0f;
    enemy->run_speed = 6.0f;
    enemy->weapon = -1;
    enemy->desired_angle = rand_angle();
    enemy->attack_delay = 0.1f;
    enemy->attack_timer = enemy->attack_delay;
    enemy->turn_speed = 5.0f;
    enemy->spawner = false;
    enemy->bounty = 100;

    components->enemy[entity] = enemy;

    return enemy;
}


EnemyComponent* EnemyComponent_get(ComponentData* components, int entity) {
    if (entity == -1) return NULL;
    return components->enemy[entity];
}


void EnemyComponent_remove(ComponentData* components, int entity) {
    EnemyComponent* enemy = EnemyComponent_get(components, entity);
    if (enemy) {
        List_delete(enemy->path);
        free(enemy);
        components->enemy[entity] = NULL;
    }
}


ParticleComponent* ParticleComponent_add(ComponentData* components, int entity, float angle, float spread, 
        float start_size, float end_size, float speed, float rate, sfColor outer_color, sfColor inner_color) {
    ParticleComponent* particle = malloc(sizeof(ParticleComponent));
    particle->type = PARTICLE_NONE;
    particle->enabled = false;
    particle->loop = false;
    particle->angle = angle;
    particle->spread = spread;
    particle->particles = 0;
    particle->max_particles = 100;
    particle->iterator = 0;
    particle->start_size = start_size;
    particle->end_size = end_size;
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
    particle->outer_color = outer_color;
    particle->inner_color = inner_color;
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
    if (particle) {
        sfCircleShape_destroy(particle->shape);
        free(particle);
        components->particle[entity] = NULL;
    }
}


VehicleComponent* VehicleComponent_add(ComponentData* components, int entity, float max_fuel) {
    VehicleComponent* vehicle = malloc(sizeof(VehicleComponent));
    vehicle->on_road = false;
    vehicle->max_fuel = max_fuel;
    vehicle->fuel = max_fuel;
    vehicle->acceleration = 30.0f;
    vehicle->max_speed = 10.0f;
    vehicle->turning = 0.25f * M_PI;
    vehicle->size = 4;
    for (int i = 0; i < vehicle->size; i++) {
        vehicle->riders[i] = -1;
    }
    vehicle->seats[0] = (sfVector2f) { 1.5f, 0.75f };
    vehicle->seats[1] = (sfVector2f) { 1.5f, -0.75f };
    vehicle->seats[2] = (sfVector2f) { 0.0f, 0.75f };
    vehicle->seats[3] = (sfVector2f) { 0.0f, -0.75f };

    components->vehicle[entity] = vehicle;

    return vehicle;
}


VehicleComponent* VehicleComponent_get(ComponentData* components, int entity) {
    if (entity == -1) return NULL;
    return components->vehicle[entity];
}


void VehicleComponent_remove(ComponentData* components, int entity) {
    VehicleComponent* vehicle = VehicleComponent_get(components, entity);
    if (vehicle) {
        free(vehicle);
        components->vehicle[entity] = NULL;
    }
}


WeaponComponent* WeaponComponent_add(ComponentData* components, int entity, float fire_rate, int damage, int shots,
                                     float spread, int magazine, float recoil, float range, float reload_time, 
                                     AmmoType ammo_type, Filename sound) {
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
    WeaponComponent* weapon = WeaponComponent_get(components, entity);
    if (weapon) {
        free(weapon);
        components->weapon[entity] = NULL;
    }
}


ItemComponent* ItemComponent_add(ComponentData* components, int entity, int size, int price, ButtonText name) {
    ItemComponent* item = malloc(sizeof(ItemComponent));
    item->size = size;
    for (int i = 0; i < size; i++) {
        item->attachments[i]= -1;
    }
    item->price = price;
    strcpy(item->name, name);
    item->use_time = 0.0f;
    item->type = ITEM_HEAL;
    item->value = 0;

    components->item[entity] = item;
    return item;
}


ItemComponent* ItemComponent_get(ComponentData* components, int entity) {
    if (entity == -1) return NULL;
    return components->item[entity];
}


void ItemComponent_remove(ComponentData* components, int entity) {
    ItemComponent* item = ItemComponent_get(components, entity);
    if (item) {
        free(item);
        components->item[entity] = NULL;
    }
}


WaypointComponent* WaypointComponent_add(ComponentData* components, int entity) {
    WaypointComponent* waypoint = malloc(sizeof(WaypointComponent));
    waypoint->came_from = -1;
    waypoint->g_score = INFINITY;
    waypoint->f_score = INFINITY;
    waypoint->neighbors = List_create();
    waypoint->range = 16.0f;

    components->waypoint[entity] = waypoint;

    return waypoint;
}


WaypointComponent* WaypointComponent_get(ComponentData* components, int entity) {
    if (entity == -1) return NULL;
    return components->waypoint[entity];
}


void WaypointComponent_remove(ComponentData* components, int entity) {
    WaypointComponent* waypoint = WaypointComponent_get(components, entity);
    if (waypoint) {
        for (ListNode* node = waypoint->neighbors->head; node; node = node->next) {
            int n = node->value;
            WaypointComponent* neighbor = WaypointComponent_get(components, n);
            if (neighbor) {
                List_remove(neighbor->neighbors, entity);
            }
        }
        List_delete(waypoint->neighbors);
        free(waypoint);
        components->waypoint[entity] = NULL;
    }
}


HealthComponent* HealthComponent_add(ComponentData* components, int entity, int health, Filename dead_image, Filename decal, Filename die_sound) {
    HealthComponent* comp = malloc(sizeof(HealthComponent));
    comp->health = health;
    comp->max_health = health;
    strcpy(comp->dead_image, dead_image);
    strcpy(comp->decal, decal);
    strcpy(comp->die_sound, die_sound);

    components->health[entity] = comp;

    return comp;
}


HealthComponent* HealthComponent_get(ComponentData* components, int entity) {
    if (entity == -1) return NULL;
    return components->health[entity];
}


void HealthComponent_remove(ComponentData* components, int entity) {
    HealthComponent* health = HealthComponent_get(components, entity);
    if (health) {
        free(health);
        components->health[entity] = NULL;
    }
}


CameraComponent* CameraComponent_add(ComponentData* components, int entity, sfVector2i resolution) {
    CameraComponent* camera = malloc(sizeof(CameraComponent));

    camera->resolution = resolution;
    camera->zoom_target = 25.0f;
    camera->zoom = camera->zoom_target * camera->resolution.y / 720.0;

    camera->shaders[0] = NULL;
    camera->shaders[1] = sfShader_createFromFile(NULL, NULL, "outline.frag");

    camera->fonts[0] = sfFont_createFromFile("data/Helvetica.ttf");

    camera->matrix = (Matrix2f) { 0.0f, 0.0f, 0.0f, 0.0f };
    camera->inv_matrix = (Matrix2f) { 0.0f, 0.0f, 0.0f, 0.0f };

    camera->shake.position = zeros();
    camera->shake.velocity = rand_vector();

    components->camera[entity] = camera;
    return camera;
}


CameraComponent* CameraComponent_get(ComponentData* components, int entity) {
    if (entity == -1) return NULL;
    return components->camera[entity];
}


void CameraComponent_remove(ComponentData* components, int entity) {
    CameraComponent* camera = CameraComponent_get(components, entity);
    if (camera) {
        free(camera);
        components->camera[entity] = NULL;
    }
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
    if (road) {
        sfConvexShape_destroy(road->shape);
        free(road);
        components->road[entity] = NULL;
    }
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
    SoundComponent* sound = SoundComponent_get(components, entity);
    if (sound) {
        free(sound);
        components->sound[entity] = NULL;
    }
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


AnimationComponent* AnimationComponent_add(ComponentData* components, int entity, int frames) {
    AnimationComponent* anim = malloc(sizeof(AnimationComponent));
    anim->frames = frames;
    anim->current_frame = 0;
    anim->framerate = 10.0f;
    anim->timer = 0.0f;
    anim->play_once = false;

    components->animation[entity] = anim;
    return anim;
}


AnimationComponent* AnimationComponent_get(ComponentData* components, int entity) {
    if (entity == -1) return NULL;
    return components->animation[entity];
}


void AnimationComponent_remove(ComponentData* components, int entity) {
    AnimationComponent* anim = AnimationComponent_get(components, entity);
    if (anim) {
        free(anim);
        components->animation[entity] = NULL;
    }
}


DoorComponent* DoorComponent_add(ComponentData* components, int entity, sfVector2f anchor) {
    DoorComponent* door = malloc(sizeof(DoorComponent));
    door->locked = false;
    door->anchor = anchor;
    door->direction = diff(get_position(components, entity), anchor);

    components->door[entity] = door;
    return door;
}


DoorComponent* DoorComponent_get(ComponentData* components, int entity) {
    if (entity == -1) return NULL;
    return components->door[entity];
}


void DoorComponent_remove(ComponentData* components, int entity) {
    DoorComponent* door = DoorComponent_get(components, entity);
    if (door) {
        free(door);
        components->door[entity] = NULL;
    }
}


JointComponent* JointComponent_add(ComponentData* components, int entity, int parent, float min_length, float max_length, float strength) {
    JointComponent* joint = malloc(sizeof(JointComponent));
    joint->parent = parent;
    joint->min_length = min_length;
    joint->max_length = max_length;
    joint->strength = strength;
    joint->max_angle = M_PI;

    components->joint[entity] = joint;
    return joint;
}


JointComponent* JointComponent_get(ComponentData* components, int entity) {
    if (entity == -1) return NULL;
    return components->joint[entity];
}


void JointComponent_remove(ComponentData* components, int entity) {
    JointComponent* joint = JointComponent_get(components, entity);
    if (joint) {
        free(joint);
        components->joint[entity] = NULL;
    }
}


WidgetComponent* WidgetComponent_add(ComponentData* components, int entity, ButtonText string, WidgetType type) {
    WidgetComponent* widget = malloc(sizeof(WidgetComponent));
    widget->enabled = true;
    widget->type = type;
    widget->selected = false;
    strcpy(widget->string, string);
    widget->text = sfText_create();
    widget->on_click = NULL;
    widget->on_change = NULL;
    widget->value = 0;
    widget->max_value = 0;
    widget->min_value = 0;
    widget->cyclic = false;
    widget->strings = NULL;
    
    components->widget.array[entity] = widget;
    List_append(components->widget.order, entity);
    return widget;
}


WidgetComponent* WidgetComponent_get(ComponentData* components, int entity) {
    if (entity == -1) return NULL;
    return components->widget.array[entity];
}


void WidgetComponent_remove(ComponentData* components, int entity) {
    WidgetComponent* widget = WidgetComponent_get(components, entity);
    if (widget) {
        free(widget);
        components->widget.array[entity] = NULL;
        List_remove(components->widget.order, entity);
    }
}


int create_entity(ComponentData* components) {
    for (int i = 0; i < components->entities; i++) {
        if (!components->coordinate[i]) {
            if (components->added_entities) {
                List_add(components->added_entities, i);
            }
            return i;
        }
    }

    components->entities++;
    if (components->added_entities) {
        List_add(components->added_entities, components->entities - 1);
    }
    return components->entities - 1;
}


int get_root(ComponentData* components, int entity) {
    int root = entity;
    CoordinateComponent* coord = CoordinateComponent_get(components, entity);
    while (coord->parent != -1) {
        root = coord->parent;
        coord = CoordinateComponent_get(components, coord->parent);
    }
    return root;
}


void add_child(ComponentData* components, int parent, int child) {
    CoordinateComponent_get(components, child)->parent = parent;
    List_append(CoordinateComponent_get(components, parent)->children, child);
}


void remove_children(ComponentData* components, int parent) {
    CoordinateComponent* coord = CoordinateComponent_get(components, parent);
    for (ListNode* node = coord->children->head; node; node = node->next) {
        CoordinateComponent_get(components, node->value)->parent = -1;
    }
    List_clear(coord->children);
}


void destroy_entity(ComponentData* components, int entity) {
    if (entity == -1) return;

    CoordinateComponent_remove(components, entity);
    ImageComponent_remove(components, entity);
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
    CameraComponent_remove(components, entity);
    RoadComponent_remove(components, entity);
    SoundComponent_remove(components, entity);
    AmmoComponent_remove(components, entity);
    AnimationComponent_remove(components, entity);
    DoorComponent_remove(components, entity);
    JointComponent_remove(components, entity);
    WidgetComponent_remove(components, entity);

    if (entity == components->entities - 1) {
        components->entities--;
    }
}


void destroy_entity_recursive(ComponentData* components, int entity) {
    CoordinateComponent* coord = CoordinateComponent_get(components, entity);
    for (ListNode* node = coord->children->head; node; node = node->next) {
        destroy_entity_recursive(components, node->value);
    }
    List_clear(coord->children);
    destroy_entity(components, entity);
}


void ComponentData_clear(ComponentData* components) {
    for (int i = 0; i < components->entities; i++) {
        destroy_entity(components, i);
    }
    components->entities = 0;
}


sfVector2f get_position(ComponentData* components, int entity) {
    CoordinateComponent* coord = CoordinateComponent_get(components, entity);
    sfVector2f position = coord->position;

    int parent = coord->parent;
    if (parent != -1) {
        position = sum(get_position(components, parent), rotate(position, get_angle(components, parent)));
    }
    return position;
}


float get_angle(ComponentData* components, int entity) {
    CoordinateComponent* coord = CoordinateComponent_get(components, entity);
    float angle = coord->angle;

    int parent = coord->parent;
    if (parent != -1) {
        angle += get_angle(components, parent);
    }

    return mod(angle, 2.0f * M_PI);
}


bool entity_exists(ComponentData* components, int entity) {
    CoordinateComponent* coord = CoordinateComponent_get(components, entity);
    if (coord) {
        return false;
    }
    return true;
}
