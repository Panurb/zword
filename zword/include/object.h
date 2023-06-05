#pragma once

extern ButtonText object_names[];

void create_bench(ComponentData* components, sfVector2f position, float angle);

void create_table(ComponentData* components, sfVector2f position, float angle);

void create_hay_bale(ComponentData* components, sfVector2f position, float angle);

void create_stove(ComponentData* components, sfVector2f position, float angle);

void create_sink(ComponentData* components, sfVector2f position, float angle);

void create_toilet(ComponentData* components, sfVector2f position, float angle);

void create_bed(ComponentData* components, sfVector2f position, float angle);

void create_candle(ComponentData* components, sfVector2f pos, float angle);

void create_lamp(ComponentData* components, sfVector2f position, float angle);

void create_desk(ComponentData* components, sfVector2f position, float angle);

void create_fire(ComponentData* components, sfVector2f pos);

void create_tree(ComponentData* components, ColliderGrid* grid, sfVector2f position);

void create_rock(ComponentData* components, ColliderGrid* grid, sfVector2f position);

void create_object(ComponentData* components, int object, sfVector2f position, float angle);
