#pragma once

#include <SFML/Graphics.h>

#include "component.h"
#include "camera.h"
#include "grid.h"
#include "image.h"


void update_lights(ComponentData* component, float delta_time);

void draw_lights(ComponentData* component, ColliderGrid* grid, sfRenderWindow* window, sfRenderTexture* texture, Camera* camera, float ambient_light);
