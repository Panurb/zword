#pragma once

#include "component.h"
#include "collider.h"


void input(Component* component, sfRenderWindow* window, ColliderGrid* grid, Camera* camera, float delta_time);

void create_player(Component* component, sfVector2f pos);
