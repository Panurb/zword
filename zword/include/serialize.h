#pragma once

#include <stdbool.h>

#include <cJSON.h>

#include "util.h"
#include "list.h"

void save_json(cJSON* json, Filename directory, Filename filename);

cJSON* load_json(Filename directory, Filename filename);

cJSON* serialize_entities(List* entities, Vector2f offset);

void serialize_map(cJSON* json, bool preserve_id);

void save_state(ButtonText map_name);

void load_state(ButtonText map_name);

void save_map(ButtonText map_name);

void load_map(ButtonText map_name);

void save_prefab(Filename filename, List* entities);

int load_prefab(Filename filename, Vector2f position, float angle, Vector2f scale);
