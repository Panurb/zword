#pragma once

#include <SFML/System/Vector2.h>
#include <SFML/Graphics.h>

#include "camera.h"
#include "grid.h"


#define MAX_NODES 1000
#define MAX_PATH_LENGTH 50


int create_waypoint(sfVector2f pos);

bool a_star(int start, int goal, List* path);

void update_waypoints(ComponentData* components, ColliderGrid* grid, int camera);

void draw_waypoints(ComponentData* components, sfRenderWindow* window, int camera, bool draw_neighbors);
