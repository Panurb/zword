#pragma once

#include <SFML/System/Vector2.h>
#include <SFML/Graphics.h>

#include "camera.h"
#include "grid.h"


#define MAX_NODES 1000
#define MAX_PATH_LENGTH 50


bool a_star(ComponentData* component, int start, int goal, int* path);

void init_waypoints(ComponentData* component, ColliderGrid* grid);

void update_waypoints(ComponentData* component, ColliderGrid* grid);

void draw_waypoints(ComponentData* component, sfRenderWindow* window, Camera* camera);
