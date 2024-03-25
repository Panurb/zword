#pragma once

#include "grid.h"
#include "camera.h"
#include "component.h"


int create_player(sfVector2f pos, float angle);

int get_slot(int i, int size);

int get_attachment(int i);

void update_players(float time_step);

void add_money(int entity, int amount);
