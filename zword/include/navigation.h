#pragma once

#include "camera.h"
#include "grid.h"


#define MAX_NODES 1000
#define MAX_PATH_LENGTH 50


int create_waypoint(Vector2f pos);

bool a_star(int start, int goal, List* path);

void update_waypoints(int camera, float range);

void draw_waypoints(int camera, bool draw_neighbors);
