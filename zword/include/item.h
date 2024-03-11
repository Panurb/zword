#pragma once

#include "component.h"
#include "grid.h"
#include "game.h"


int create_flashlight(sfVector2f position);

int create_gas(sfVector2f position);

int create_bandage(sfVector2f position);

void create_item(sfVector2f position, int tier);

void pick_up_item(int entity);

void drop_item(int entity);

void use_item(int entity, float time_step);

void draw_items();
