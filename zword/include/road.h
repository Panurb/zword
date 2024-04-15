#pragma once

#define _USE_MATH_DEFINES

#include <stdlib.h>
#include <math.h>

#include "component.h"
#include "perlin.h"
#include "camera.h"
#include "image.h"


void create_road(ComponentData* components, Vector2f start, Vector2f end);

void create_river(ComponentData* components, Vector2f start, Vector2f end);

void draw_road(int camera, int entity);

void resize_roads(ComponentData* components);
