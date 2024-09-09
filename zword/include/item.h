#pragma once

#include "component.h"
#include "grid.h"
#include "game.h"


bool is_attachment(int entity);

int create_flashlight(Vector2f position);

int create_gas(Vector2f position);

int create_bandage(Vector2f position);

void create_item(Vector2f position, int tier);

void pick_up_item(int entity);

void drop_item(int entity);

bool use_item(int entity, float time_step);

void draw_player_targets();
