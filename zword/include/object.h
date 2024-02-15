#pragma once

#include "component.h"


int create_bench(ComponentData* components, sfVector2f position, float angle);

int create_table(ComponentData* components, sfVector2f position);

int create_hay_bale(ComponentData* components, sfVector2f position, float angle);

int create_stove(ComponentData* components, sfVector2f position, float angle);

int create_sink(ComponentData* components, sfVector2f position, float angle);

int create_toilet(ComponentData* components, sfVector2f position, float angle);

int create_bed(ComponentData* components, sfVector2f position, float angle);

int create_candle(ComponentData* components, sfVector2f pos);

int create_lamp(ComponentData* components, sfVector2f position);

int create_desk(ComponentData* components, sfVector2f position, float angle);

int create_fire(ComponentData* components, sfVector2f pos);

int create_tree(ComponentData* components, sfVector2f position);

int create_rock(ComponentData* components, sfVector2f position);

int create_object(ComponentData* components, ButtonText object_name, sfVector2f position, float angle);

int create_uranium(ComponentData* components, sfVector2f position);
