#pragma once

#include <SFML/Graphics.h>

#include "component.h"
#include "camera.h"
#include "grid.h"


typedef struct {
    sfVector2f position;
    int object;
    sfVector2f normal;
} HitInfo;

HitInfo raycast(Component* component, ColliderGrid* grid, sfVector2f start, sfVector2f velocity, float range);

void update_lights(Component* component, float delta_time);

void draw_lights(Component* component, ColliderGrid* grid, sfRenderWindow* window, sfRenderTexture* texture, Camera* camera, float ambient_light);