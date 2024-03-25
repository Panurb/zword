#pragma once

#include <SFML/Window.h>
#include <SFML/System/Vector2.h>

#include "component.h"
#include "grid.h"
#include "camera.h"
#include "game.h"


void update_enemies(float time_step);

int create_zombie(sfVector2f pos, float angle);

int create_farmer(sfVector2f pos, float angle);

int create_priest(sfVector2f pos, float angle);

int create_big_boy(sfVector2f pos, float angle);

int create_boss(sfVector2f pos, float angle);

void draw_enemies(int camera);

void alert_enemies(int player, float range);

void create_spawner(sfVector2f position, float angle, float width, float height);

void draw_spawners(sfRenderWindow* window, GameData data);
