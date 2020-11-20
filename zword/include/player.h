#pragma once

#include "component.h"


void input(Component* component, sfRenderWindow* window, Camera* camera, float delta_time);

void create_player(Component* component, sfVector2f pos);
