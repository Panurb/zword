#pragma once

#include "component.h"
#include "grid.h"


int get_akimbo(ComponentData* components, int entity);

void shoot(ComponentData* component, ColliderGrid* grid, int i);

void reload(ComponentData* component, int i);

void create_weapon(ComponentData* component, float x, float y);

void create_lasersight(ComponentData* component, float x, float y);

void update_weapons(ComponentData* components, float time_step);
