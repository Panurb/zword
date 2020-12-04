#pragma once

#include "component.h"
#include "grid.h"


void shoot(Component* component, ColliderGrid* grid, int i);

void reload(Component* component, int i);

void create_weapon(Component* component, float x, float y);
