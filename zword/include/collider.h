#pragma once

#include <SFML/System/Vector2.h>

#include "component.h"
#include "camera.h"


typedef int CollisionGrid[64][64][10];

void update_grid(Component* component, CollisionGrid collision_grid, int i);

void clear_grid(Component* component, CollisionGrid collision_grid, int i);

void collide(Component* component, CollisionGrid collision_grid);

void debug_draw(Component* component, sfRenderWindow* window, Camera* camera);
