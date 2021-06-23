#pragma once

#include <SFML/Graphics.h>

#include "component.h"
#include "camera.h"
#include "grid.h"
#include "image.h"


void update_lights(ComponentData* component, float delta_time);

void draw_shadows(ComponentData* components, sfRenderTexture* texture, int camera);

void draw_lights(ComponentData* component, ColliderGrid* grid, sfRenderTexture* texture, int camera, float ambient_light);
