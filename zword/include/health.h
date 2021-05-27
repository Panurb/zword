#pragma once

#include "component.h"
#include "grid.h"


void damage(ComponentData* components, ColliderGrid* grid, int entity, sfVector2f pos, sfVector2f dir, int dmg);

void blunt_damage(ComponentData* components, ColliderGrid* grid, int entity, sfVector2f vel);
