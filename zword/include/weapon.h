#pragma once

#include "component.h"
#include "grid.h"


void shoot(ComponentData* component, ColliderGrid* grid, int i);

void reload(ComponentData* component, int i);

void create_weapon(ComponentData* component, float x, float y);

void create_lasersight(ComponentData* component, float x, float y);
