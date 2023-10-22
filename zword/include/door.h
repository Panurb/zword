#pragma once

#include "component.h"
#include "grid.h"

int create_door(ComponentData* components, sfVector2f pos, float angle);

void update_doors(ComponentData* components);
