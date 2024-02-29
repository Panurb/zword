#define _USE_MATH_DEFINES

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>

#include <cJSON.h>

#include "game.h"
#include "component.h"
#include "particle.h"


static cJSON* serialized_ids[MAX_ENTITIES] = { 0 };
static int* deserialized_ids[MAX_ENTITIES] = { 0 };


void serialize_id(cJSON* json, char* name, int id) {
    if (id != -1) {
        cJSON* id_json = cJSON_AddNumberToObject(json, name, id);
        for (int i = 0; i < MAX_ENTITIES; i++) {
            if (!serialized_ids[i]) {
                serialized_ids[i] = id_json;
                break;
            }
        }
    }
}


void serialize_id_array(cJSON* json, char* name, int* array, int size) {
    if (size == 0) return;

    cJSON* array_json = cJSON_AddArrayToObject(json, name);
    for (int i = 0; i < size; i++) {
        cJSON* item = cJSON_CreateNumber(array[i]);
        cJSON_AddItemToArray(array_json, item);
        for (int j = 0; j < MAX_ENTITIES; j++) {
            if (!serialized_ids[j]) {
                serialized_ids[j] = item;
                break;
            }
        }
    }
}


void deserialize_id_array(cJSON* json, char* name, int* array) {
    cJSON* json_array = cJSON_GetObjectItem(json, name);
    if (!json_array) return;

    int i = 0;
    cJSON* item;
    cJSON_ArrayForEach(item, json_array) {
        array[i] = item->valueint;
        for (int j = 0; j < MAX_ENTITIES; j++) {
            if (!deserialized_ids[j]) {
                deserialized_ids[j] = &array[i];
                break;
            }
        }
        i++;
    }
}


void deserialize_id(cJSON* json, char* name, int* id_ptr) {
    cJSON* id_json = cJSON_GetObjectItem(json, name);
    if (id_json) {
        int value = id_json->valueint;
        for (int i = 0; i < MAX_ENTITIES; i++) {
            if (!deserialized_ids[i]) {
                deserialized_ids[i] = id_ptr;
                break;
            }
        }
        *id_ptr = value;
    } else {
        *id_ptr = -1;
    }
}


void serialize_int(cJSON* json, char* name, int value, int default_value) {
    if (value != default_value) {
        cJSON_AddNumberToObject(json, name, value);
    }
}


int deserialize_int(cJSON* json, char* name, int value) {
    cJSON* val = cJSON_GetObjectItem(json, name);
    if (val) {
        return val->valueint;
    }
    return value;
}


void serialize_float(cJSON* json, char* name, float value, float default_value) {
    if (value != default_value) {
        cJSON_AddNumberToObject(json, name, value);
    }
}


float deserialize_float(cJSON* json, char* name, float value) {
    cJSON* val = cJSON_GetObjectItem(json, name);
    if (val) {
        return val->valuedouble;
    }
    return value;
}


void serialize_string(cJSON* json, char* name, char* value, char* default_value) {
    if (strcmp(value, default_value) != 0) {
        cJSON_AddStringToObject(json, name, value);
    }
}


char* deserialize_string(cJSON* json, char* name, char* value) {
    cJSON* val = cJSON_GetObjectItem(json, name);
    if (val) {
        return val->valuestring;
    }
    return value;
}


int updated_id(int id, int ids[MAX_ENTITIES]) {
    if (id == -1) {
        return -1;
    }
    return ids[id];
}


void CoordinateComponent_serialize(cJSON* entity_json, ComponentData* components, int entity,
        sfVector2f offset) {
    CoordinateComponent* coord = CoordinateComponent_get(components, entity);
    if (!coord) return;

    cJSON* json = cJSON_CreateObject();
    cJSON_AddItemToObject(entity_json, "Coordinate", json);
    cJSON_AddNumberToObject(json, "x", coord->position.x + offset.x);
    cJSON_AddNumberToObject(json, "y", coord->position.y + offset.y);
    serialize_float(json, "angle", coord->angle, 0.0f);
    serialize_id(json, "parent", coord->parent);
}


void CoordinateComponent_deserialize(cJSON* entity_json, ComponentData* components, int entity,
        sfVector2f offset, float rotation) {
    cJSON* coord_json = cJSON_GetObjectItem(entity_json, "Coordinate");
    sfVector2f pos;
    pos.x = cJSON_GetObjectItem(coord_json, "x")->valuedouble + offset.x;
    pos.y = cJSON_GetObjectItem(coord_json, "y")->valuedouble + offset.y;
    float angle = deserialize_float(coord_json, "angle", 0.0f) + rotation;
    CoordinateComponent* coord = CoordinateComponent_add(components, entity, pos, angle);
    deserialize_id(coord_json, "parent", &coord->parent);
}


void ImageComponent_serialize(cJSON* entity_json, ComponentData* components, int entity) {
    ImageComponent* image = ImageComponent_get(components, entity);
    if (!image) return;

    cJSON* json = cJSON_CreateObject();
    cJSON_AddItemToObject(entity_json, "Image", json);
    cJSON_AddStringToObject(json, "filename", image->filename);
    cJSON_AddNumberToObject(json, "width", image->width);
    cJSON_AddNumberToObject(json, "height", image->height);
    cJSON_AddNumberToObject(json, "layer", image->layer);
    serialize_float(json, "scale_x", image->scale.x, 1.0f);
    serialize_float(json, "scale_y", image->scale.y, 1.0f);
    serialize_float(json, "alpha", image->alpha, 1.0f);
}


void ImageComponent_deserialize(cJSON* entity_json, ComponentData* components, int entity) {
    cJSON* json = cJSON_GetObjectItem(entity_json, "Image");
    if (!json) return;

    char* filename = cJSON_GetObjectItem(json, "filename")->valuestring;
    float width = cJSON_GetObjectItem(json, "width")->valuedouble;
    float height = cJSON_GetObjectItem(json, "height")->valuedouble;
    float layer = cJSON_GetObjectItem(json, "layer")->valueint;
    ImageComponent* image = ImageComponent_add(components, entity, filename, width, height, layer);
    image->scale.x = deserialize_float(json, "scale_x", image->scale.x);
    image->scale.y = deserialize_float(json, "scale_y", image->scale.y);
    image->alpha = deserialize_float(json, "alpha", 1.0f);
}


void PhysicsComponent_serialize(cJSON* entity_json, ComponentData* components, int entity) {
    PhysicsComponent* physics = PhysicsComponent_get(components, entity);
    if (!physics) return;

    cJSON* json = cJSON_CreateObject();
    cJSON_AddItemToObject(entity_json, "Physics", json);
    cJSON_AddNumberToObject(json, "mass", physics->mass);
    serialize_float(json, "vx", physics->velocity.x, 0.0f);
    serialize_float(json, "vy", physics->velocity.y, 0.0f);
    serialize_float(json, "ax", physics->acceleration.x, 0.0f);
    serialize_float(json, "ay", physics->acceleration.y, 0.0f);
    serialize_float(json, "av", physics->angular_velocity, 0.0f);
    serialize_float(json, "aa", physics->angular_acceleration, 0.0f);
    serialize_float(json, "bounce", physics->bounce, 0.5f);
    serialize_float(json, "max_speed", physics->max_speed, 20.0f);
}


void PhysicsComponent_deserialize(cJSON* entity_json, ComponentData* components, int entity) {
    cJSON* json = cJSON_GetObjectItem(entity_json, "Physics");
    if (!json) return;

    float mass = cJSON_GetObjectItem(json, "mass")->valuedouble;
    PhysicsComponent* physics = PhysicsComponent_add(components, entity, mass);
    physics->velocity.x = deserialize_float(json, "vx", physics->velocity.x);
    physics->velocity.y = deserialize_float(json, "vy", physics->velocity.y);
    physics->acceleration.x = deserialize_float(json, "ax", physics->acceleration.x);
    physics->acceleration.y = deserialize_float(json, "ay", physics->acceleration.y);
    physics->angular_velocity = deserialize_float(json, "av", physics->angular_velocity);
    physics->angular_acceleration = deserialize_float(json, "aa", physics->angular_acceleration);
    physics->bounce = deserialize_float(json, "bounce", physics->bounce);
    physics->max_speed = deserialize_float(json, "max_speed", physics->max_speed);
}


void ColliderComponent_serialize(cJSON* entity_json, ComponentData* components, int entity) {
    ColliderComponent* collider = ColliderComponent_get(components, entity);
    if (!collider) return;

    cJSON* json = cJSON_CreateObject();
    cJSON_AddItemToObject(entity_json, "Collider", json);
    cJSON_AddNumberToObject(json, "type", collider->type);
    cJSON_AddNumberToObject(json, "width", collider->width);
    cJSON_AddNumberToObject(json, "height", collider->height);
    cJSON_AddNumberToObject(json, "group", collider->group);
    serialize_int(json, "enabled", collider->enabled, true);
    serialize_int(json, "trigger_type", collider->trigger_type, TRIGGER_NONE);
}


void ColliderComponent_deserialize(cJSON* entity_json, ComponentData* components, int entity) {
    cJSON* json = cJSON_GetObjectItem(entity_json, "Collider");
    if (!json) return;

    int type = cJSON_GetObjectItem(json, "type")->valueint;
    float width = cJSON_GetObjectItem(json, "width")->valuedouble;
    float height = cJSON_GetObjectItem(json, "height")->valuedouble;
    int group = cJSON_GetObjectItem(json, "group")->valueint;
    ColliderComponent* collider;
    if (type == COLLIDER_CIRCLE) {
        collider = ColliderComponent_add_circle(components, entity, 0.5f * width, group);
    } else if (type == COLLIDER_RECTANGLE) {
        collider = ColliderComponent_add_rectangle(components, entity, width, height, group);
    }
    collider->enabled = deserialize_int(json, "enabled", collider->enabled);
    collider->trigger_type = deserialize_int(json, "trigger_type", collider->trigger_type);
}


void LightComponent_serialize(cJSON* entity_json, ComponentData* components, int entity) {
    LightComponent* light = LightComponent_get(components, entity);
    if (!light) return;

    cJSON* json = cJSON_CreateObject();
    cJSON_AddItemToObject(entity_json, "Light", json);
    serialize_int(json, "enabled", light->enabled, true);
    cJSON_AddNumberToObject(json, "range", light->range);
    cJSON_AddNumberToObject(json, "angle", light->angle);
    cJSON_AddNumberToObject(json, "color_r", light->color.r);
    cJSON_AddNumberToObject(json, "color_g", light->color.g);
    cJSON_AddNumberToObject(json, "color_b", light->color.b);
    cJSON_AddNumberToObject(json, "brightness", light->max_brightness);
    cJSON_AddNumberToObject(json, "speed", light->speed);
    serialize_float(json, "flicker", light->flicker, 0.0f);
}


void LightComponent_deserialize(cJSON* entity_json, ComponentData* components, int entity) {
    cJSON* json = cJSON_GetObjectItem(entity_json, "Light");
    if (!json) return;

    float range = cJSON_GetObjectItem(json, "range")->valuedouble;
    float angle = cJSON_GetObjectItem(json, "angle")->valuedouble;
    sfColor color = sfWhite;
    color.r = cJSON_GetObjectItem(json, "color_r")->valuedouble;
    color.g = cJSON_GetObjectItem(json, "color_g")->valuedouble;
    color.b = cJSON_GetObjectItem(json, "color_b")->valuedouble;
    float brightness = cJSON_GetObjectItem(json, "brightness")->valuedouble;
    float speed = cJSON_GetObjectItem(json, "speed")->valuedouble;
    LightComponent* light = LightComponent_add(components, entity, range, angle, color, brightness, speed);
    light->enabled = deserialize_int(json, "enabled", light->enabled);
    light->flicker = deserialize_float(json, "flicker", light->flicker);
}


void ParticleComponent_serialize(cJSON* entity_json, ComponentData* components, int entity) {
    ParticleComponent* particle = ParticleComponent_get(components, entity);
    if (!particle) return;

    cJSON* json = cJSON_CreateObject();
    cJSON_AddItemToObject(entity_json, "Particle", json);
    if (particle->type == PARTICLE_NONE) {
        cJSON_AddNumberToObject(json, "angle", particle->angle);
        cJSON_AddNumberToObject(json, "spread", particle->spread);
        cJSON_AddNumberToObject(json, "start_size", particle->start_size);
        cJSON_AddNumberToObject(json, "end_size", particle->end_size);
        cJSON_AddNumberToObject(json, "speed", particle->speed);
        cJSON_AddNumberToObject(json, "rate", particle->rate);
        cJSON_AddNumberToObject(json, "outer_r", particle->outer_color.r);
        cJSON_AddNumberToObject(json, "outer_g", particle->outer_color.g);
        cJSON_AddNumberToObject(json, "outer_b", particle->outer_color.b);
        cJSON_AddNumberToObject(json, "inner_r", particle->inner_color.r);
        cJSON_AddNumberToObject(json, "inner_g", particle->inner_color.g);
        cJSON_AddNumberToObject(json, "inner_b", particle->inner_color.b);
    } else {
        cJSON_AddNumberToObject(json, "type", particle->type);
        cJSON_AddNumberToObject(json, "start_size", particle->start_size);
    }
    serialize_int(json, "loop", particle->loop, false);
}


void ParticleComponent_deserialize(cJSON* entity_json, ComponentData* components, int entity) {
    cJSON* json = cJSON_GetObjectItem(entity_json, "Particle");
    if (!json) return;

    ParticleType type = deserialize_int(json, "type", PARTICLE_NONE);
    if (type == PARTICLE_NONE) {
        float angle = cJSON_GetObjectItem(json, "angle")->valuedouble;
        float spread = cJSON_GetObjectItem(json, "spread")->valuedouble;
        float start_size = cJSON_GetObjectItem(json, "start_size")->valuedouble;
        float end_size = cJSON_GetObjectItem(json, "end_size")->valuedouble;
        float speed = cJSON_GetObjectItem(json, "speed")->valuedouble;
        float rate = cJSON_GetObjectItem(json, "rate")->valuedouble;
        sfColor outer_color = sfWhite;
        outer_color.r = cJSON_GetObjectItem(json, "outer_r")->valueint;
        outer_color.g = cJSON_GetObjectItem(json, "outer_g")->valueint;
        outer_color.b = cJSON_GetObjectItem(json, "outer_b")->valueint;
        sfColor inner_color = sfWhite;
        inner_color.r = cJSON_GetObjectItem(json, "inner_r")->valueint;
        inner_color.g = cJSON_GetObjectItem(json, "inner_g")->valueint;
        inner_color.b = cJSON_GetObjectItem(json, "inner_b")->valueint;
        ParticleComponent* particle = ParticleComponent_add(components, entity, angle, spread, start_size, end_size, speed, 
            rate, outer_color, inner_color);
        particle->loop = deserialize_int(json, "loop", particle->loop);
        if (particle->loop) {
        particle->enabled = true;
        }
    } else {
        float size = cJSON_GetObjectItem(json, "start_size")->valuedouble;
        ParticleComponent_add_type(components, entity, type, size);
    }
}


void JointComponent_serialize(cJSON* entity_json, ComponentData* components, int entity) {
    JointComponent* joint = JointComponent_get(components, entity);
    if (!joint) return;

    cJSON* json = cJSON_CreateObject();
    cJSON_AddItemToObject(entity_json, "Joint", json);
    serialize_id(json, "parent", joint->parent);
    cJSON_AddNumberToObject(json, "min_length", joint->min_length);
    cJSON_AddNumberToObject(json, "max_length", joint->max_length);
    cJSON_AddNumberToObject(json, "strength", joint->strength);
    serialize_float(json, "max_angle", joint->max_angle, M_PI);
}


void JointComponent_deserialize(cJSON* entity_json, ComponentData* components, int entity) {
    cJSON* json = cJSON_GetObjectItem(entity_json, "Joint");
    if (!json) return;

    float min_length = cJSON_GetObjectItem(json, "min_length")->valuedouble;
    float max_length = cJSON_GetObjectItem(json, "max_length")->valuedouble;
    // FIXME: strength INFINITY -> null
    float strength = cJSON_GetObjectItem(json, "strength")->valuedouble;
    JointComponent* joint = JointComponent_add(components, entity, -1, min_length, max_length, strength);
    deserialize_id(json, "parent", &joint->parent);
    joint->max_angle = deserialize_float(json, "max_angle", joint->max_angle);
}


void WaypointComponent_serialize(cJSON* entity_json, ComponentData* components, int entity) {
    WaypointComponent* waypoint = WaypointComponent_get(components, entity);
    if (!waypoint) return;

    cJSON* json = cJSON_CreateObject();
    cJSON_AddItemToObject(entity_json, "Waypoint", json);
}


void WaypointComponent_deserialize(cJSON* entity_json, ComponentData* components, int entity) {
    cJSON* json = cJSON_GetObjectItem(entity_json, "Waypoint");
    if (!json) return;

    WaypointComponent_add(components, entity);
}

void PlayerComponent_serialize(cJSON* entity_json, ComponentData* components, int entity) {
    PlayerComponent* player = PlayerComponent_get(components, entity);
    if (!player) return;

    cJSON* json = cJSON_CreateObject();
    cJSON_AddItemToObject(entity_json, "Player", json);
    serialize_int(json, "vehicle", player->vehicle, -1);
    serialize_int(json, "item", player->item, 0);
    serialize_id_array(json, "inventory", player->inventory, player->inventory_size);
    serialize_id_array(json, "ammo", player->ammo, player->ammo_size);
    serialize_int(json, "state", player->state, PLAYER_ON_FOOT);
    serialize_id(json, "arms", player->arms);
    serialize_int(json, "money", player->money, 0);
}

void PlayerComponent_deserialize(cJSON* entity_json, ComponentData* components, int entity) {
    cJSON* json = cJSON_GetObjectItem(entity_json, "Player");
    if (!json) return;

    PlayerComponent* player = PlayerComponent_add(components, entity, -1);
    player->vehicle = deserialize_int(json, "vehicle", player->vehicle);
    player->item = deserialize_int(json, "item", player->item);
    deserialize_id_array(json, "inventory", player->inventory);
    deserialize_id_array(json, "ammo", player->ammo);
    player->state = deserialize_int(json, "state", player->state);
    deserialize_id(json, "arms", &player->arms);
    player->money = deserialize_int(json, "money", player->money);
}


void EnemyComponent_serialize(cJSON* entity_json, ComponentData* components, int entity) {
    EnemyComponent* enemy = EnemyComponent_get(components, entity);
    if (!enemy) return;

    cJSON* json = cJSON_CreateObject();
    cJSON_AddItemToObject(entity_json, "Enemy", json);
    serialize_int(json, "state", enemy->state, ENEMY_IDLE);
    serialize_id(json, "target", enemy->target);
    serialize_float(json, "fov", enemy->fov, 0.5f * M_PI);
    serialize_float(json, "idle_speed", enemy->idle_speed, 1.0f);
    serialize_float(json, "walk_speed", enemy->walk_speed, 2.0f);
    serialize_float(json, "run_speed", enemy->run_speed, 6.0f);
    serialize_id(json, "weapon", enemy->weapon);
    serialize_float(json, "attack_delay", enemy->attack_delay, 0.1f);
    serialize_float(json, "turn_speed", enemy->turn_speed, 5.0f);
    serialize_int(json, "spawner", enemy->spawner, false);
}


void EnemyComponent_deserialize(cJSON* entity_json, ComponentData* components, int entity) {
    cJSON* json = cJSON_GetObjectItem(entity_json, "Enemy");
    if (!json) return;

    EnemyComponent* enemy = EnemyComponent_add(components, entity);
    enemy->state = deserialize_int(json, "state", enemy->state);
    deserialize_id(json, "target", &enemy->target);
    enemy->fov = deserialize_float(json, "fov", enemy->fov);
    enemy->idle_speed = deserialize_float(json, "idle_speed", enemy->idle_speed);
    enemy->walk_speed = deserialize_float(json, "walk_speed", enemy->walk_speed);
    enemy->run_speed = deserialize_float(json, "run_speed", enemy->run_speed);
    deserialize_id(json, "weapon", &enemy->weapon);
    enemy->attack_delay = deserialize_float(json, "attack_delay", enemy->attack_delay);
    enemy->turn_speed = deserialize_float(json, "turn_speed", enemy->turn_speed);
    enemy->spawner = deserialize_int(json, "spawner", enemy->spawner);
}


void HealthComponent_serialize(cJSON* entity_json, ComponentData* components, int entity) {
    HealthComponent* health = HealthComponent_get(components, entity);
    if (!health) return;

    cJSON* json = cJSON_CreateObject();
    cJSON_AddItemToObject(entity_json, "Health", json);
    serialize_int(json, "health", health->health, 0);
    cJSON_AddStringToObject(json, "dead_image", health->dead_image);
    cJSON_AddStringToObject(json, "decal", health->decal);
    cJSON_AddStringToObject(json, "die_sound", health->die_sound);
}


void HealthComponent_deserialize(cJSON* entity_json, ComponentData* components, int entity) {
    cJSON* json = cJSON_GetObjectItem(entity_json, "Health");
    if (!json) return;

    int hp = deserialize_int(json, "health", 0);
    char* dead_image = cJSON_GetObjectItem(json, "dead_image")->valuestring;
    char* decal = cJSON_GetObjectItem(json, "decal")->valuestring;
    char* die_sound = cJSON_GetObjectItem(json, "die_sound")->valuestring;
    HealthComponent_add(components, entity, hp, dead_image, decal, die_sound);
}


void SoundComponent_serialize(cJSON* entity_json, ComponentData* components, int entity) {
    SoundComponent* sound = SoundComponent_get(components, entity);
    if (!sound) return;

    cJSON* json = cJSON_CreateObject();
    cJSON_AddItemToObject(entity_json, "Sound", json);
    if (sound->hit_sound[0] != '\0') {
        cJSON_AddStringToObject(json, "hit_sound", sound->hit_sound);
    }
    if (sound->loop_sound[0] != '\0') {
        cJSON_AddStringToObject(json, "loop_sound", sound->loop_sound);
    }
}


void SoundComponent_deserialize(cJSON* entity_json, ComponentData* components, int entity) {
    cJSON* json = cJSON_GetObjectItem(entity_json, "Sound");
    if (!json) return;

    char hit_sound[256] = "";
    char loop_sound[256] = "";
    cJSON* hit_sound_json = cJSON_GetObjectItem(json, "hit_sound");
    if (hit_sound_json) {
        strcpy(hit_sound, hit_sound_json->valuestring);
    }
    cJSON* loop_sound_json = cJSON_GetObjectItem(json, "loop_sound");
    if (loop_sound_json) {
        strcpy(loop_sound, loop_sound_json->valuestring);
    }
    SoundComponent* sound = SoundComponent_add(components, entity, hit_sound);
    strcpy(sound->loop_sound, loop_sound);
}


void WeaponComponent_serialize(cJSON* entity_json, ComponentData* components, int entity) {
    WeaponComponent* weapon = WeaponComponent_get(components, entity);
    if (!weapon) return;

    cJSON* json = cJSON_CreateObject();
    cJSON_AddItemToObject(entity_json, "Weapon", json);
    cJSON_AddNumberToObject(json, "fire_rate", weapon->fire_rate);
    cJSON_AddNumberToObject(json, "recoil", weapon->recoil_up);
    cJSON_AddNumberToObject(json, "damage", weapon->damage);
    cJSON_AddNumberToObject(json, "max_magazine", weapon->max_magazine);
    cJSON_AddNumberToObject(json, "magazine", weapon->magazine);
    cJSON_AddNumberToObject(json, "max_recoil", weapon->max_recoil);
    cJSON_AddNumberToObject(json, "reload_time", weapon->reload_time);
    cJSON_AddNumberToObject(json, "range", weapon->range);
    cJSON_AddNumberToObject(json, "sound_range", weapon->sound_range);
    cJSON_AddNumberToObject(json, "spread", weapon->spread);
    cJSON_AddNumberToObject(json, "shots", weapon->shots);
    cJSON_AddNumberToObject(json, "ammo_type", weapon->ammo_type);
    cJSON_AddStringToObject(json, "sound", weapon->sound);
    cJSON_AddBoolToObject(json, "automatic", weapon->automatic);
}


void WeaponComponent_deserialize(cJSON* entity_json, ComponentData* components, int entity) {
    cJSON* json = cJSON_GetObjectItem(entity_json, "Weapon");
    if (!json) return;

    float fire_rate = cJSON_GetObjectItem(json, "fire_rate")->valuedouble;
    float damage = cJSON_GetObjectItem(json, "damage")->valuedouble;
    int shots = cJSON_GetObjectItem(json, "shots")->valueint;
    float spread = cJSON_GetObjectItem(json, "spread")->valuedouble;
    int max_magazine = cJSON_GetObjectItem(json, "max_magazine")->valueint;
    int magazine = cJSON_GetObjectItem(json, "magazine")->valueint;
    float recoil = cJSON_GetObjectItem(json, "recoil")->valuedouble;
    float range = cJSON_GetObjectItem(json, "range")->valuedouble;
    float reload_time = cJSON_GetObjectItem(json, "reload_time")->valuedouble;
    int ammo_type = cJSON_GetObjectItem(json, "ammo_type")->valueint;
    char* sound = cJSON_GetObjectItem(json, "sound")->valuestring;
    bool automatic = cJSON_GetObjectItem(json, "automatic")->valueint;
    WeaponComponent* weapon = WeaponComponent_add(components, entity, fire_rate, damage, shots, spread, max_magazine, 
        recoil, range, reload_time, ammo_type, sound);
    weapon->magazine = magazine;
    weapon->automatic = automatic;
}


void ItemComponent_serialize(cJSON* entity_json, ComponentData* components, int entity) {
    ItemComponent* item = ItemComponent_get(components, entity);
    if (!item) return;

    cJSON* json = cJSON_CreateObject();
    cJSON_AddItemToObject(entity_json, "Item", json);
    cJSON_AddNumberToObject(json, "size", item->size);
    serialize_id_array(json, "attachments", item->attachments, item->size);
    serialize_int(json, "price", item->price, 0);
    if (item->name[0] != '\0') {
        cJSON_AddStringToObject(json, "name", item->name);
    }
    cJSON_AddNumberToObject(json, "type", item->type);
    serialize_int(json, "value", item->value, 0);
    serialize_float(json, "use_time", item->use_time, 0.0f);
}


void ItemComponent_deserialize(cJSON* entity_json, ComponentData* components, int entity) {
    cJSON* json = cJSON_GetObjectItem(entity_json, "Item");
    if (!json) return;

    int size = cJSON_GetObjectItem(json, "size")->valueint;
    int price = deserialize_int(json, "price", 0);
    char* name = "";
    cJSON* name_json = cJSON_GetObjectItem(json, "name");
    if (name_json) {
        name = name_json->valuestring;
    }
    ItemComponent* item = ItemComponent_add(components, entity, size, price, name);
    deserialize_id_array(json, "attachments", item->attachments);
    item->type = deserialize_int(json, "type", item->type);
    item->value = deserialize_int(json, "value", item->value);
    item->use_time = deserialize_float(json, "use_time", item->use_time);
}


void AnimationComponent_serialize(cJSON* entity_json, ComponentData* components, int entity) {
    AnimationComponent* animation = AnimationComponent_get(components, entity);
    if (!animation) return;

    cJSON* json = cJSON_CreateObject();
    cJSON_AddItemToObject(entity_json, "Animation", json);
    cJSON_AddNumberToObject(json, "frames", animation->frames);
    serialize_int(json, "current_frame", animation->current_frame, 0);
    serialize_float(json, "framerate", animation->framerate, 10.0f);
    serialize_float(json, "timer", animation->timer, 0.0f);
    serialize_int(json, "play_once", animation->play_once, false);
}


void AnimationComponent_deserialize(cJSON* entity_json, ComponentData* components, int entity) {
    cJSON* json = cJSON_GetObjectItem(entity_json, "Animation");
    if (!json) return;

    int frames = cJSON_GetObjectItem(json, "frames")->valueint;
    AnimationComponent* animation = AnimationComponent_add(components, entity, frames);
    animation->current_frame = deserialize_int(json, "current_frame", animation->current_frame);
    animation->framerate = deserialize_float(json, "framerate", animation->framerate);
    animation->timer = deserialize_float(json, "timer", animation->timer);
    animation->play_once = deserialize_int(json, "play_once", animation->play_once);
}


void AmmoComponent_serialize(cJSON* entity_json, ComponentData* components, int entity) {
    AmmoComponent* ammo = AmmoComponent_get(components, entity);
    if (!ammo) return;

    cJSON* json = cJSON_CreateObject();
    cJSON_AddItemToObject(entity_json, "Ammo", json);
    cJSON_AddNumberToObject(json, "type", ammo->type);
    cJSON_AddNumberToObject(json, "size", ammo->size);
}


void AmmoComponent_deserialize(cJSON* entity_json, ComponentData* components, int entity) {
    cJSON* json = cJSON_GetObjectItem(entity_json, "Ammo");
    if (!json) return;

    AmmoType type = cJSON_GetObjectItem(json, "type")->valueint;
    AmmoComponent* ammo = AmmoComponent_add(components, entity, type);
    ammo->size = cJSON_GetObjectItem(json, "size")->valueint;
}


void DoorComponent_serialize(cJSON* entity_json, ComponentData* components, int entity) {
    DoorComponent* door = DoorComponent_get(components, entity);
    if (!door) return;

    cJSON* json = cJSON_CreateObject();
    cJSON_AddItemToObject(entity_json, "Door", json);
    cJSON_AddNumberToObject(json, "price", door->price);
    serialize_int(json, "locked", door->locked, false);
}


void DoorComponent_deserialize(cJSON* entity_json, ComponentData* components, int entity) {
    cJSON* json = cJSON_GetObjectItem(entity_json, "Ammo");
    if (!json) return;

    int price = cJSON_GetObjectItem(json, "price")->valueint;
    DoorComponent* door = DoorComponent_add(components, entity, price);
    door->locked = deserialize_int(json, "locked", door->locked);
}


void TextComponent_serialize(cJSON* entity_json, ComponentData* components, int entity) {
    TextComponent* text = TextComponent_get(components, entity);
    if (!text) return;

    cJSON* json = cJSON_CreateObject();
    cJSON_AddItemToObject(entity_json, "Text", json);
    cJSON_AddStringToObject(json, "source_string", text->source_string);
    cJSON_AddNumberToObject(json, "size", text->size);
    serialize_int(json, "color_r", text->color.r, 255);
    serialize_int(json, "color_g", text->color.g, 255);
    serialize_int(json, "color_b", text->color.b, 255);
}


void TextComponent_deserialize(cJSON* entity_json, ComponentData* components, int entity) {
    cJSON* json = cJSON_GetObjectItem(entity_json, "Text");
    if (!json) return;

    char* source_string = cJSON_GetObjectItem(json, "source_string")->valuestring;
    int size = cJSON_GetObjectItem(json, "size")->valueint;
    sfColor color = sfWhite;
    color.r = deserialize_int(json, "color_r", 255);
    color.g = deserialize_int(json, "color_g", 255);
    color.b = deserialize_int(json, "color_b", 255);
    TextComponent_add(components, entity, source_string, size, color);
}


bool serialize_entity(cJSON* entities_json, ComponentData* components, int entity, int id,
        sfVector2f offset) {
    if (!CoordinateComponent_get(components, entity)) return false;
    if (WidgetComponent_get(components, entity)) return false;
    if (CameraComponent_get(components, entity)) return false;

    cJSON* json = cJSON_CreateObject();
    cJSON_AddItemToArray(entities_json, json);
    cJSON_AddNumberToObject(json, "id", id);

    CoordinateComponent_serialize(json, components, entity, offset);
    ImageComponent_serialize(json, components, entity);
    PhysicsComponent_serialize(json, components, entity);
    ColliderComponent_serialize(json, components, entity);
    PlayerComponent_serialize(json, components, entity);
    LightComponent_serialize(json, components, entity);
    EnemyComponent_serialize(json, components, entity);
    ParticleComponent_serialize(json, components, entity);
    // TODO: vehicle
    WeaponComponent_serialize(json, components, entity);
    ItemComponent_serialize(json, components, entity);
    WaypointComponent_serialize(json, components, entity);
    HealthComponent_serialize(json, components, entity);
    // TODO: road
    SoundComponent_serialize(json, components, entity);
    AmmoComponent_serialize(json, components, entity);
    AnimationComponent_serialize(json, components, entity);
    JointComponent_serialize(json, components, entity);
    TextComponent_serialize(json, components, entity);

    return true;
}


int deserialize_entity(cJSON* entity_json, ComponentData* components, bool preserve_id,
        sfVector2f offset, float rotation) {
    int entity;
    if (preserve_id) {
        entity = cJSON_GetObjectItem(entity_json, "id")->valueint;
        if (CoordinateComponent_get(components, entity)) {
            destroy_entity(components, entity);
        }
        components->entities = max(entity, components->entities);
    } else {
        entity = create_entity(components);
    }

    CoordinateComponent_deserialize(entity_json, components, entity, offset, rotation);
    ImageComponent_deserialize(entity_json, components, entity);
    PhysicsComponent_deserialize(entity_json, components, entity);
    ColliderComponent_deserialize(entity_json, components, entity);
    PlayerComponent_deserialize(entity_json, components, entity);
    LightComponent_deserialize(entity_json, components, entity);
    EnemyComponent_deserialize(entity_json, components, entity);
    ParticleComponent_deserialize(entity_json, components, entity);
    WeaponComponent_deserialize(entity_json, components, entity);
    ItemComponent_deserialize(entity_json, components, entity);
    WaypointComponent_deserialize(entity_json, components, entity);
    HealthComponent_deserialize(entity_json, components, entity);
    SoundComponent_deserialize(entity_json, components, entity);
    AmmoComponent_deserialize(entity_json, components, entity);
    AnimationComponent_deserialize(entity_json, components, entity);
    JointComponent_deserialize(entity_json, components, entity);
    TextComponent_deserialize(entity_json, components, entity);

    return entity;
}


void update_serialized_ids(int ids[MAX_ENTITIES]) {
    for (int i = 0; i < LENGTH(serialized_ids); i++) {
        cJSON* id_json = serialized_ids[i];
        if (!id_json) {
            break;
        }
        printf("%d -> %d\n", id_json->valueint, updated_id(id_json->valueint, ids));
        cJSON_SetNumberValue(id_json, updated_id(id_json->valueint, ids));
    }
}


void update_deserialized_ids(int ids[MAX_ENTITIES]) {
    for (int i = 0; i < LENGTH(deserialized_ids); i++) {
        int* id_ptr = deserialized_ids[i];
        if (!id_ptr) break;
        printf("%d -> %d\n", *id_ptr, updated_id(*id_ptr, ids));
        *id_ptr = updated_id(*id_ptr, ids);
    }
}


cJSON* serialize_entities(GameData* data, List* entities, sfVector2f offset) {
    for (int i = 0; i < MAX_ENTITIES; i++) {
        serialized_ids[i] = NULL;
    }

    cJSON* json = cJSON_CreateObject();

    cJSON* entities_json = cJSON_CreateArray();
    cJSON_AddItemToObject(json, "entities", entities_json);

    int ids[MAX_ENTITIES];
    int id = 0;
    ListNode* node;
    FOREACH(node, entities) {
        int i = node->value;
        if (serialize_entity(entities_json, data->components, i, id, offset)) {
            ids[i] = id;
            id++;
        }
    }

    update_serialized_ids(ids);

    return json;
}


void serialize_map(cJSON* json, ComponentData* components, bool preserve_id) {
    for (int i = 0; i < MAX_ENTITIES; i++) {
        serialized_ids[i] = NULL;
    }

    cJSON* entities_json = cJSON_CreateArray();
    cJSON_AddItemToObject(json, "entities", entities_json);

    int ids[MAX_ENTITIES];
    int id = 0;
    sfVector2f offset = zeros();
    for (int i = 0; i < components->entities; i++) {
        if (preserve_id) {
            serialize_entity(entities_json, components, i, i, offset);
        } else {
            if (serialize_entity(entities_json, components, i, id, offset)) {
                ids[i] = id;
                id++;
            }
        }
    }

    if (!preserve_id) {
        update_serialized_ids(ids);
    }
}


void deserialize_map(cJSON* json, ComponentData* components, bool preserve_id) {
    for (int i = 0; i < MAX_ENTITIES; i++) {
        deserialized_ids[i] = NULL;
    }

    cJSON* entities = cJSON_GetObjectItem(json, "entities");
    cJSON* entity;
    int ids[MAX_ENTITIES];
    for (int i = 0; i < MAX_ENTITIES; i++) {
        ids[i] = -1;
    }
    int i = 0;
    sfVector2f offset = zeros();
    cJSON_ArrayForEach(entity, entities) {
        if (!preserve_id) {
            i = cJSON_GetObjectItem(entity, "id")->valueint;
        }
        ids[i] = deserialize_entity(entity, components, preserve_id, offset, 0.0f);
    }

    if (!preserve_id) {
        update_deserialized_ids(ids);
    }

    for (int i = 0; i < MAX_ENTITIES; i++) {
        CoordinateComponent* coord = CoordinateComponent_get(components, i);
        if (!coord) continue;
        if (coord->parent != -1) {
            add_child(components, coord->parent, i);
        }
    }
}


cJSON* serialize_game(GameData* data, bool preserve_id) {
    cJSON* json = cJSON_CreateObject();

    cJSON_AddNumberToObject(json, "game_mode", data->game_mode);
    cJSON_AddNumberToObject(json, "ambient_light", data->ambient_light);

    serialize_map(json, data->components, preserve_id);

    return json;
}


void deserialize_entities(cJSON* json, GameData* data, sfVector2f offset, float rotation) {
    for (int i = 0; i < MAX_ENTITIES; i++) {
        deserialized_ids[i] = NULL;
    }

    cJSON* entities = cJSON_GetObjectItem(json, "entities");
    cJSON* entity;
    int ids[MAX_ENTITIES];
    int n = 0;
    cJSON_ArrayForEach(entity, entities) {
        ids[n] = deserialize_entity(entity, data->components, false, offset, rotation);
        update_grid(data->components, data->grid, ids[n]);
        n++;
    }

    update_deserialized_ids(ids);
}


void deserialize_game(cJSON* json, GameData* data, bool preserve_id) {
    game_data->game_mode = deserialize_int(json, "game_mode", game_data->game_mode);
    printf("Game mode: %d\n", game_data->game_mode);
    game_data->ambient_light = deserialize_float(json, "ambient_light", game_data->ambient_light);
    printf("Ambient light: %f\n", game_data->ambient_light);
    deserialize_map(json, data->components, preserve_id);
}


void save_json(cJSON* json, Filename directory, Filename filename) {
    Filename path;
    snprintf(path, 128, "%s/%s/%s%s", "data", directory, filename, ".json");
    FILE* file = fopen(path, "w");
    if (!file) {
        printf("Could not open file: %s\n", path);
        return;
    }

    char* string = cJSON_Print(json);
    fputs(string, file);

    fclose(file);
    free(string);
}


cJSON* load_json(Filename directory, Filename filename) {
    Filename path;
    snprintf(path, 128, "%s/%s/%s%s", "data", directory, filename, ".json");
    FILE* file = fopen(path, "r");
    if (!file) {
        printf("Could not open file: %s\n", path);
        return NULL;
    }

    fseek(file, 0, SEEK_END);
    long fsize = ftell(file);
    rewind(file);

    char* string = malloc(fsize + 1);
    fread(string, fsize, 1, file);
    fclose(file);

    string[fsize] = 0;

    cJSON* json = cJSON_Parse(string);
    free(string);
    return json;
}


void save_game(GameData* data, ButtonText map_name) {
    cJSON* json = serialize_game(data, false);
    save_json(json, "maps", map_name);
    cJSON_Delete(json);
}


void load_game(GameData* data, ButtonText map_name) {
    cJSON* json = load_json("maps", map_name);
    if (json) {
        deserialize_game(json, data, false);
        cJSON_Delete(json);
        strcpy(data->map_name, map_name);
    }
}
