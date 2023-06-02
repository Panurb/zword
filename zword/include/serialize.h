#pragma once

#include <cJSON.h>

bool serialize_entity(cJSON* entities_json, ComponentData* components, int entity, int id);

int deserialize_entity(cJSON* entity_json, ComponentData* components, bool preserve_id);

void save_json(cJSON* json, Filename directory, Filename filename);

cJSON* load_json(Filename directory, Filename filename);

cJSON* serialize_entities(ComponentData* components, List* entities, sfVector2f offset);

void serialize_map(cJSON* json, ComponentData* components, bool preserve_id);

void deserialize_entities(cJSON* json, GameData* data, sfVector2f offset, float rotation);

void save_game(GameData* data, ButtonText map_name);

void load_game(GameData* data, ButtonText map_name);
