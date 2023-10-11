#pragma once

#include "component.h"

typedef enum {
    OBJECT_BED,
    OBJECT_BENCH,
    OBJECT_BIG_BOY,
    OBJECT_BOSS,
    OBJECT_CANDLE,
    OBJECT_CAR,
    OBJECT_DESK,
    OBJECT_DOOR,
    OBJECT_FARMER,
    OBJECT_FLASHLIGHT,
    OBJECT_GAS,
    OBJECT_HAY_BALE,
    OBJECT_LAMP,
    OBJECT_PLAYER,
    OBJECT_PRIEST,
    OBJECT_ROCK,
    OBJECT_TABLE,
    OBJECT_TREE,
    OBJECT_WAYPOINT,
    OBJECT_ZOMBIE
} Object;

void create_bench(ComponentData* components, sfVector2f position, float angle);

void create_table(ComponentData* components, sfVector2f position);

void create_hay_bale(ComponentData* components, sfVector2f position, float angle);

void create_stove(ComponentData* components, sfVector2f position, float angle);

void create_sink(ComponentData* components, sfVector2f position, float angle);

void create_toilet(ComponentData* components, sfVector2f position, float angle);

int create_bed(ComponentData* components, sfVector2f position, float angle);

void create_candle(ComponentData* components, sfVector2f pos);

void create_lamp(ComponentData* components, sfVector2f position);

void create_desk(ComponentData* components, sfVector2f position, float angle);

void create_fire(ComponentData* components, sfVector2f pos);

void create_tree(ComponentData* components, sfVector2f position);

void create_rock(ComponentData* components, sfVector2f position);

int create_object(ComponentData* components, ButtonText object_name, sfVector2f position, float angle);
