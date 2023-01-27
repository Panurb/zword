#include <stdio.h>
#include <stdlib.h>

#include <cJSON.h>

#include "game.h"
#include "component.h"


void serialize_int(cJSON* json, char* name, int value, int default_value) {
    if (value != default_value) {
        cJSON_AddNumberToObject(json, name, value);
    }
}


float deserialize_int(cJSON* json, char* name, int value) {
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


void CoordinateComponent_serialize(cJSON* entity_json, ComponentData* components, int entity) {
    CoordinateComponent* coord = CoordinateComponent_get(components, entity);
    if (!coord) return;

    cJSON* json = cJSON_CreateObject();
    cJSON_AddItemToObject(entity_json, "Coordinate", json);
    cJSON_AddNumberToObject(json, "x", coord->position.x);
    cJSON_AddNumberToObject(json, "y", coord->position.y);
    serialize_float(json, "angle", coord->angle, 0.0f);
    serialize_int(json, "parent", coord->angle, -1);
}


void CoordinateComponent_deserialize(cJSON* entity_json, ComponentData* components, int entity) {
    cJSON* coord_json = cJSON_GetObjectItem(entity_json, "Coordinate");
    sfVector2f pos;
    pos.x = cJSON_GetObjectItem(coord_json, "x")->valuedouble;
    pos.y = cJSON_GetObjectItem(coord_json, "y")->valuedouble;
    float angle = deserialize_float(coord_json, "angle", 0.0f);
    CoordinateComponent_add(components, entity, pos, angle);

    cJSON* parent_json = cJSON_GetObjectItem(coord_json, "parent");
    if (parent_json) {
        add_child(components, parent_json->valueint, entity);
    }

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
}


void ImageComponent_deserialize(cJSON* entity_json, ComponentData* components, int entity) {
    cJSON* image_json = cJSON_GetObjectItem(entity_json, "Image");
    if (!image_json) return;

    char* filename = cJSON_GetObjectItem(image_json, "filename")->valuestring;
    float width = cJSON_GetObjectItem(image_json, "width")->valuedouble;
    float height = cJSON_GetObjectItem(image_json, "height")->valuedouble;
    float layer = cJSON_GetObjectItem(image_json, "layer")->valueint;
    ImageComponent_add(components, entity, filename, width, height, layer);
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


void serialize_entity(cJSON* entities_json, ComponentData* components, int entity) {
    if (WidgetComponent_get(components, entity)) return;
    if (CameraComponent_get(components, entity)) return;

    cJSON* json = cJSON_CreateObject();
    cJSON_AddItemToArray(entities_json, json);
    cJSON_AddNumberToObject(json, "id", entity);

    CoordinateComponent_serialize(json, components, entity);
    ImageComponent_serialize(json, components, entity);
    ColliderComponent_serialize(json, components, entity);
    PhysicsComponent_serialize(json, components, entity);
}


void deserialize_entity(cJSON* entity_json, ComponentData* components) {
    int entity = cJSON_GetObjectItem(entity_json, "id")->valueint;
    if (CoordinateComponent_get(components, entity)) {
        destroy_entity(components, entity);
    }

    CoordinateComponent_deserialize(entity_json, components, entity);
    ColliderComponent_deserialize(entity_json, components, entity);
}


char* serialize_game(GameData* data) {
    cJSON* json = cJSON_CreateObject();

    cJSON* entities = cJSON_CreateArray();
    cJSON_AddItemToObject(json, "entities", entities);
    for (int i = 0; i < data->components->entities; i++) {
        if (CoordinateComponent_get(data->components, i)) {
            serialize_entity(entities, data->components, i);
        }
    }

    char* string = cJSON_Print(json);
    cJSON_Delete(json);
    return string;
}


void deserialize_game(GameData* data, char* string) {
    cJSON* json = cJSON_Parse(string);

    cJSON* entities = cJSON_GetObjectItem(json, "entities");
    cJSON* entity;
    cJSON_ArrayForEach(entity, entities) {
        deserialize_entity(entity, data->components);
    }

    cJSON_Delete(json);
}


void save_game(GameData* data) {
    FILE* file = fopen("data/maps/map.json", "w");
    if (!file) {
        printf("Could not open file.\n");
        return;
    }

    char* string = serialize_game(data);
    fputs(string, file);

    fclose(file);
    free(string);
}


void load_game(GameData* data) {
    FILE* file = fopen("data/maps/map.json", "r");
    if (!file) {
        printf("Could not open file.\n");
        return;
    }

    fseek(file, 0, SEEK_END);
    long fsize = ftell(file);
    rewind(file);

    char* string = malloc(fsize + 1);
    fread(string, fsize, 1, file);
    fclose(file);

    string[fsize] = 0;

    deserialize_game(data, string);

    free(string);
}
