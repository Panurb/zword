#pragma once

#include <SFML/Window.h>
#include <SFML/System/Vector2.h>

#include "component.h"
#include "grid.h"
#include "camera.h"


void update_enemies(ComponentData* component, ColliderGrid* grid, float time_step);

int create_zombie(ComponentData* component, ColliderGrid* grid, sfVector2f pos);

void create_farmer(ComponentData* component, ColliderGrid* grid, sfVector2f pos);

void create_priest(ComponentData* components, ColliderGrid* grid, sfVector2f pos);

void create_big_boy(ComponentData* components, ColliderGrid* grid, sfVector2f pos);

void create_boss(ComponentData* components, sfVector2f pos, float angle);

void draw_enemies(ComponentData* component, sfRenderWindow* window, int camera);

void alert_enemies(ComponentData* components, ColliderGrid* grid, int player, float range);

void spawn_enemies(ComponentData* components, ColliderGrid* grid, int camera);
