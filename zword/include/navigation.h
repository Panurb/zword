#pragma once

#include <SFML/System/Vector2.h>
#include <SFML/Graphics.h>

#include "camera.h"
#include "grid.h"


#define MAX_NODES 1000
#define MAX_PATH_LENGTH 50


bool a_star(ComponentData* component, int start, int goal, List* path);

void init_waypoints(ComponentData* components, ColliderGrid* grid);

void update_waypoints(ComponentData* components, ColliderGrid* grid);

void draw_waypoints(ComponentData* components, sfRenderWindow* window, int camera);
