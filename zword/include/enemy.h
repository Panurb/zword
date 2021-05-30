#pragma once

#include <SFML/Window.h>
#include <SFML/System/Vector2.h>

#include "component.h"
#include "grid.h"
#include "camera.h"


void update_enemies(ComponentData* component, ColliderGrid* grid);

void create_zombie(ComponentData* component, sfVector2f pos);

void create_farmer(ComponentData* component, sfVector2f pos);

void create_big_boy(ComponentData* components, sfVector2f pos);

void draw_enemies(ComponentData* component, sfRenderWindow* window, int camera);

void alert_enemies(ComponentData* components, ColliderGrid* grid, int player, float range);
