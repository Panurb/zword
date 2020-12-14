#pragma once

#include <SFML/System/Vector2.h>

#include "component.h"
#include "camera.h"
#include "grid.h"


void get_corners(ComponentData* component, int i, sfVector2f* corners) ;

sfVector2f half_width(ComponentData* component, int i);

sfVector2f half_height(ComponentData* component, int i);

float axis_half_width(ComponentData* component, int i, sfVector2f axis);

void collide(ComponentData* component, ColliderGrid* collision_grid);

void draw_occupied_tiles(ComponentData* components, ColliderGrid* grid, sfRenderWindow* window, Camera* camera);

void debug_draw(ComponentData* component, sfRenderWindow* window, Camera* camera);
