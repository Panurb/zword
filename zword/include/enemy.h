#pragma once

#include "component.h"
#include "grid.h"
#include "camera.h"
#include "game.h"


void update_enemies(float time_step);

int create_zombie(Vector2f pos, float angle);

int create_farmer(Vector2f pos, float angle);

int create_priest(Vector2f pos, float angle);

int create_big_boy(Vector2f pos, float angle);

int create_boss(Vector2f pos, float angle);

void draw_enemies(int camera);

void alert_enemies(int player, float range);

void enemy_die(int entity);

void create_spawner(Vector2f position, float angle, float width, float height);

void draw_spawners();

int spawn_enemy(Vector2f position, float probs[4]);
