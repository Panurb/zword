#pragma once

#include <SFML/Window.h>
#include <SFML/System/Vector2.h>

#include "component.h"
#include "grid.h"
#include "camera.h"
#include "game.h"


void update_enemies(ComponentData* component, ColliderGrid* grid, float time_step);

void create_zombie(ComponentData* component, sfVector2f pos, float angle);

void create_farmer(ComponentData* component, sfVector2f pos, float angle);

void create_priest(ComponentData* components, sfVector2f pos, float angle);

void create_big_boy(ComponentData* components, sfVector2f pos, float angle);

void create_boss(ComponentData* components, sfVector2f pos, float angle);

void draw_enemies(ComponentData* component, sfRenderWindow* window, int camera);

void alert_enemies(ComponentData* components, ColliderGrid* grid, int player, float range);

void create_spawner(ComponentData* components, sfVector2f position, float angle, float width, float height);

int spawn_enemies(ComponentData* components, ColliderGrid* grid, int camera, float time_step, int max_enemies);

void draw_spawners(sfRenderWindow* window, GameData data);
