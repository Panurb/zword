#pragma once

#include "grid.h"
#include "camera.h"
#include "component.h"


void input(Component* component);

void create_player(Component* component, sfVector2f pos);

void update_players(Component* component, ColliderGrid* grid, sfRenderWindow* window, Camera* camera, float time_step);

void draw_players(Component* component, sfRenderWindow* window, Camera* camera);
