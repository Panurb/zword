#pragma once

#include "component.h"
#include "grid.h"

void create_door(ComponentData* components, sfVector2f pos, float angle);

void update_doors(ComponentData* components, ColliderGrid* grid);
