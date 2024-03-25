#pragma once

#include <SFML/Graphics.h>

#include "component.h"
#include "camera.h"
#include "grid.h"
#include "image.h"


void update_lights(float delta_time);

void draw_shadows(sfRenderTexture* texture, int camera);

void draw_lights(sfRenderTexture* texture, int camera, float ambient_light);
