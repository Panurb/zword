#pragma once

#include "grid.h"
#include "camera.h"
#include "component.h"


void input(ComponentData* component);

void create_player(ComponentData* components, sfVector2f pos);

void update_players(ComponentData* components, ColliderGrid* grid, sfRenderWindow* window, int camera, float time_step);

void draw_players(ComponentData* components, sfRenderWindow* window, int camera);
