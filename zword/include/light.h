#pragma once

#include <SFML/Graphics.h>

#include "component.h"
#include "camera.h"
#include "grid.h"


typedef struct {
    sfVector2f position;
    int object;
    sfVector2f normal; // TODO
} HitInfo;

HitInfo raycast(Component* component, ColliderGrid* grid, sfVector2f start, sfVector2f velocity, float range);

void draw_light(Component* component, ColliderGrid* grid, sfRenderWindow* window, Camera* camera);
