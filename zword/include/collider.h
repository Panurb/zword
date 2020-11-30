#pragma once

#include <SFML/System/Vector2.h>

#include "component.h"
#include "camera.h"
#include "grid.h"


sfVector2f half_width(Component* component, int i);

sfVector2f half_height(Component* component, int i);

float axis_half_width(Component* component, int i, sfVector2f axis);

void collide(Component* component, ColliderGrid* collision_grid);

void debug_draw(Component* component, sfRenderWindow* window, Camera* camera);
