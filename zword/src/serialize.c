#define _USE_MATH_DEFINES

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>

#include <cJSON.h>

#include "game.h"
#include "component.h"
#include "particle.h"


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


void CoordinateComponent_serialize(cJSON* entity_json, ComponentData* components, int entity,
        sfVector2f offset) {
    CoordinateComponent* coord = CoordinateComponent_get(components, entity);
    if (!coord) return;

    cJSON* json = cJSON_CreateObject();
    cJSON_AddItemToObject(entity_json, "Coordinate", json);
    cJSON_AddNumberToObject(json, "x", coord->position.x + offset.x);
    cJSON_AddNumberToObject(json, "y", coord->position.y + offset.y);
    serialize_float(json, "angle", coord->angle, 0.0f);
    serialize_int(json, "parent", coord->parent, -1);
}


void CoordinateComponent_deserialize(cJSON* entity_json, ComponentData* components, int entity,
        sfVector2f offset, float rotation) {
    cJSON* coord_json = cJSON_GetObjectItem(entity_json, "Coordinate");
    sfVector2f pos;
    pos.x = cJSON_GetObjectItem(coord_json, "x")->valuedouble + offset.x;
    pos.y = cJSON_GetObjectItem(coord_json, "y")->valuedouble + offset.y;
    float angle = deserialize_float(coord_json, "angle", 0.0f) + rotation;
    CoordinateComponent* coord = CoordinateComponent_add(components, entity, pos, angle);
    coord->parent = deserialize_int(coord_json, "parent", coord->parent);
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
}


void LightComponent_serialize(cJSON* entity_json, ComponentData* components, int entity) {
    LightComponent* light = LightComponent_get(components, entity);
    if (!light) return;

    cJSON* json = cJSON_CreateObject();
    cJSON_AddItemToObject(entity_json, "Light", json);
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
    cJSON_AddNumberToObject(json, "parent", joint->parent);
    cJSON_AddNumberToObject(json, "min_length", joint->min_length);
    cJSON_AddNumberToObject(json, "max_length", joint->max_length);
    cJSON_AddNumberToObject(json, "strength", joint->strength);
    serialize_float(json, "max_angle", joint->max_angle, M_PI);
}


void JointComponent_deserialize(cJSON* entity_json, ComponentData* components, int entity) {
    cJSON* json = cJSON_GetObjectItem(entity_json, "Joint");
    if (!json) return;

    int parent = cJSON_GetObjectItem(json, "parent")->valueint;
    float min_length = cJSON_GetObjectItem(json, "min_length")->valuedouble;
    float max_length = cJSON_GetObjectItem(json, "max_length")->valuedouble;
    float strength = cJSON_GetObjectItem(json, "strength")->valuedouble;
    JointComponent* joint = JointComponent_add(components, entity, parent, min_length, max_length, strength);
    joint->max_angle = deserialize_float(json, "max_angle", joint->max_angle);
}


void WaypointComponent_serialize(cJSON* entity_json, ComponentData* components, int entity) {
    WaypointComponent* joint = WaypointComponent_get(components, entity);
    if (!joint) return;

    cJSON* json = cJSON_CreateObject();
    cJSON_AddItemToObject(entity_json, "Waypoint", json);
}


void WaypointComponent_deserialize(cJSON* entity_json, ComponentData* components, int entity) {
    cJSON* json = cJSON_GetObjectItem(entity_json, "Waypoint");
    if (!json) return;

    WaypointComponent_add(components, entity);
}


bool serialize_entity(cJSON* entities_json, ComponentData* components, int entity, int id,
        sfVector2f offset) {
    if (!CoordinateComponent_get(components, entity)) return false;
    if (WidgetComponent_get(components, entity)) return false;
    if (CameraComponent_get(components, entity)) return false;
    if (PlayerComponent_get(components, entity)) return false;
    if (ItemComponent_get(components, entity)) return false;

    cJSON* json = cJSON_CreateObject();
    cJSON_AddItemToArray(entities_json, json);
    cJSON_AddNumberToObject(json, "id", id);

    CoordinateComponent_serialize(json, components, entity, offset);
    ImageComponent_serialize(json, components, entity);
    ColliderComponent_serialize(json, components, entity);
    PhysicsComponent_serialize(json, components, entity);
    LightComponent_serialize(json, components, entity);
    ParticleComponent_serialize(json, components, entity);
    JointComponent_serialize(json, components, entity);
    WaypointComponent_serialize(json, components, entity);

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
    } else {
        entity = create_entity(components);
    }

    CoordinateComponent_deserialize(entity_json, components, entity, offset, rotation);
    ImageComponent_deserialize(entity_json, components, entity);
    ColliderComponent_deserialize(entity_json, components, entity);
    PhysicsComponent_deserialize(entity_json, components, entity);
    LightComponent_deserialize(entity_json, components, entity);
    ParticleComponent_deserialize(entity_json, components, entity);
    JointComponent_deserialize(entity_json, components, entity);
    WaypointComponent_deserialize(entity_json, components, entity);

    return entity;
}


void update_serialized_ids(cJSON* entities_json, int ids[MAX_ENTITIES]) {
    // Update parent ids to match new ids.
    cJSON* entity;
    cJSON_ArrayForEach(entity, entities_json) {
        cJSON* coord_json = cJSON_GetObjectItem(entity, "Coordinate");
        cJSON* parent_json = cJSON_GetObjectItem(coord_json, "parent");
        if (parent_json) {
            cJSON_SetNumberValue(parent_json, ids[parent_json->valueint]);
        }
        cJSON* joint_json = cJSON_GetObjectItem(entity, "Joint");
        if (joint_json) {
            parent_json = cJSON_GetObjectItem(joint_json, "parent");
            cJSON_SetNumberValue(parent_json, ids[parent_json->valueint]);
        }
    }
}


cJSON* serialize_entities(GameData* data, List* entities, sfVector2f offset) {
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

    update_serialized_ids(entities_json, ids);

    return json;
}


void serialize_map(cJSON* json, ComponentData* components, bool preserve_id) {
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
        update_serialized_ids(entities_json, ids);
    }
}


cJSON* serialize_game(GameData* data, bool preserve_id) {
    cJSON* json = cJSON_CreateObject();
    serialize_map(json, data->components, preserve_id);
    // TODO: ambient light etc.

    return json;
}


void update_deserialized_id(ComponentData* components, int ids[MAX_ENTITIES], int entity) {
    // Update parent ids to match new ids.
    int id = ids[entity];
    CoordinateComponent* coord = CoordinateComponent_get(components, id);
    if (coord->parent != -1) {
        coord->parent = ids[coord->parent];
        add_child(components, coord->parent, id);
    }
    JointComponent* joint = JointComponent_get(components, id);
    if (joint) {
        joint->parent = ids[joint->parent];
    }
}


void deserialize_entities(cJSON* json, GameData* data, sfVector2f offset, float rotation) {
    cJSON* entities = cJSON_GetObjectItem(json, "entities");
    cJSON* entity;
    int ids[MAX_ENTITIES];
    int n = 0;
    cJSON_ArrayForEach(entity, entities) {
        ids[n] = deserialize_entity(entity, data->components, false, offset, rotation);
        update_grid(data->components, data->grid, ids[n]);
        n++;
    }

    for (int i = 0; i < n; i++) {
        update_deserialized_id(data->components, ids, i);
    }
}


void deserialize_game(cJSON* json, GameData* data, bool preserve_id) {
    cJSON* entities = cJSON_GetObjectItem(json, "entities");
    cJSON* entity;
    int ids[MAX_ENTITIES];
    int i = 0;
    sfVector2f offset = zeros();
    cJSON_ArrayForEach(entity, entities) {
        ids[i] = deserialize_entity(entity, data->components, preserve_id, offset, 0.0f);
        i++;
    }

    if (!preserve_id) {
        for (int i = 0; i < data->components->entities; i++) {
            update_deserialized_id(data->components, ids, i);
        }
    }
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
        init_grid(data->components, data->grid);
        strcpy(data->map_name, map_name);
    }
}