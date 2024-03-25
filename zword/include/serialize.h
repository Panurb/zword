#pragma once

#include <cJSON.h>

bool serialize_entity(cJSON* entities_json, int entity, int id);

int deserialize_entity(cJSON* entity_json, bool preserve_id);

void save_json(cJSON* json, Filename directory, Filename filename);

cJSON* load_json(Filename directory, Filename filename);

cJSON* serialize_entities(List* entities, sfVector2f offset);

void serialize_map(cJSON* json, bool preserve_id);

void deserialize_entities(cJSON* json, sfVector2f offset, float rotation);

void save_game(ButtonText map_name);

void load_game(ButtonText map_name);
