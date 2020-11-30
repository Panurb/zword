#pragma once

#include "component.h"
#include "grid.h"


void update_enemy(Component* component, ColliderGrid* grid);

void create_enemy(Component* component, sfVector2f pos);
