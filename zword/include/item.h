#pragma once

#include "component.h"
#include "grid.h"


void create_flashlight(ComponentData* components, sfVector2f position, float angle);

void create_gas(ComponentData* components, sfVector2f position, float angle);

void create_item(ComponentData* components, sfVector2f position, int tier);

void pick_up_item(ComponentData* components, ColliderGrid* grid, int entity);

void drop_item(ComponentData* components, int entity);
