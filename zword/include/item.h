#pragma once

#include "component.h"
#include "grid.h"


void create_flashlight(ComponentData* components, sfVector2f position);

void create_gas(ComponentData* components, sfVector2f position);

void pick_up_item(ComponentData* components, ColliderGrid* grid, int entity);

void drop_item(ComponentData* components, int entity);
