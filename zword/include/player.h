#pragma once

#include "component.h"


int create_player(Vector2f pos, float angle);

int get_slot(int i, int size);

int get_attachment(int i);

void update_player_movement(Entity entity);

void player_shoot(Entity entity, float time_step, bool apply_damage);

void update_players(float time_step);

void player_die(int entity);

void add_money(int entity, int amount);

void reset_player_spawn_ammo(Entity entity);

void respawn_player(Entity entity, Vector2f position);

Entity get_random_player();
