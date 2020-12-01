#pragma once

#include "component.h"
#include "grid.h"


void shoot(Component* component, ColliderGrid* grid, int i, float delta_time);

void create_weapon(Component* component, float x, float y);
