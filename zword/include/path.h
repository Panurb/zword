#pragma once

#include "component.h"


void create_path(Vector2f start, Vector2f end, String filename, String end_filename);

void create_road(Vector2f start, Vector2f end);

void create_river(Vector2f start, Vector2f end);

void create_footpath(Vector2f start, Vector2f end);

void draw_path(int camera, int entity);

void resize_roads(ComponentData* components);

void debug_draw_paths(Entity camera);
