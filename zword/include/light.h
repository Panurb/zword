#pragma once

#include <SFML/Graphics.h>

#include "component.h"
#include "camera.h"
#include "grid.h"


void update_lights(Component* component, float delta_time);

void draw_lights(Component* component, ColliderGrid* grid, sfRenderWindow* window, sfRenderTexture* texture, Camera* camera, float ambient_light);
