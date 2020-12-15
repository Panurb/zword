#pragma once

#include "component.h"


void apply_force(ComponentData* components, int entity, sfVector2f force);

void update(ComponentData* component, float deltaTime, ColliderGrid* collision_grid);
