#pragma once

#include <SFML/System/Vector2.h>
#include <SFML/Graphics.h>

#include "camera.h"
#include "grid.h"


#define MAX_NODES 1000
#define MAX_PATH_LENGTH 50


bool a_star(Component* component, int start, int goal, int* path);

void init_waypoints(Component* component, ColliderGrid* grid);

void update_waypoints(Component* component, ColliderGrid* grid);

void draw_waypoints(Component* component, sfRenderWindow* window, Camera* camera);
