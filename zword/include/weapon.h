#pragma once

#include "component.h"
#include "grid.h"


int get_akimbo(ComponentData* components, int entity);

void shoot(ComponentData* component, ColliderGrid* grid, int i);

void reload(ComponentData* component, int i);

void create_pistol(ComponentData* component, sfVector2f position);

void create_shotgun(ComponentData* components, sfVector2f position);

void create_assault_rifle(ComponentData* components, sfVector2f position);

void create_axe(ComponentData* component, sfVector2f position);

void create_lasersight(ComponentData* component, sfVector2f pos);

void update_weapons(ComponentData* components, float time_step);
