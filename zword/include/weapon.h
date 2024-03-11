#pragma once

#include "component.h"
#include "grid.h"


int get_akimbo(int entity);

void reload(int i);

void update_energy();

void attack(int i);

void update_weapons(float time_step);

int create_pistol( sfVector2f position);

int create_shotgun(sfVector2f position);

int create_sawed_off(sfVector2f position);

int create_rifle(sfVector2f position);

int create_assault_rifle(sfVector2f position);

int create_smg(sfVector2f position);

int create_axe(sfVector2f position);

int create_sword(sfVector2f position);

int create_rope_gun(sfVector2f position);

int create_lasersight(sfVector2f pos);

int create_ammo(sfVector2f position, AmmoType type);
