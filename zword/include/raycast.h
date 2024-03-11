#pragma once

#include "component.h"
#include "grid.h"


typedef struct {
    sfVector2f position;
    int entity;
    sfVector2f normal;
} HitInfo;

HitInfo raycast(sfVector2f start, sfVector2f velocity, float range, ColliderGroup group);
