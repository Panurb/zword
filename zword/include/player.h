#pragma once

#include "grid.h"
#include "camera.h"
#include "component.h"


void input(ComponentData* component, sfRenderWindow* window, int camera);

void create_player(ComponentData* components, sfVector2f pos, int joystick);

int get_inventory_slot(ComponentData* components, int i);

int get_attachment(ComponentData* components, int i);

void update_players(ComponentData* components, ColliderGrid* grid, float time_step);

void draw_players(ComponentData* components, sfRenderWindow* window, int camera);
