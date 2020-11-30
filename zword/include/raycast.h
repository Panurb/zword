#pragma once

#include "component.h"
#include "grid.h"


typedef struct {
    sfVector2f position;
    int object;
    sfVector2f normal;
} HitInfo;

HitInfo raycast(Component* component, ColliderGrid* grid, sfVector2f start, sfVector2f velocity, float range);
