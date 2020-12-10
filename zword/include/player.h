#pragma once

#include "grid.h"
#include "camera.h"
#include "component.h"


void input(ComponentData* component);

void create_player(ComponentData* component, sfVector2f pos);

void update_players(ComponentData* component, ColliderGrid* grid, sfRenderWindow* window, Camera* camera, float time_step);

void draw_players(ComponentData* component, sfRenderWindow* window, Camera* camera);
