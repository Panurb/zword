#pragma once

#define _USE_MATH_DEFINES

#include <stdlib.h>
#include <math.h>

#include "component.h"
#include "perlin.h"
#include "camera.h"
#include "image.h"


void create_road(Vector2f start, Vector2f end);

void create_river(Vector2f start, Vector2f end);

void create_footpath(Vector2f start, Vector2f end);

void draw_path(int camera, int entity);

void resize_roads(ComponentData* components);

void debug_draw_paths(Entity camera);
