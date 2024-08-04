#pragma once

#include "component.h"
#include "grid.h"


typedef struct {
    float magazine;
    float spread;
    float recoil;
    float damage;
    float fire_rate;
} WeaponStats;


int get_akimbo(int entity);

void reload(int i);

void update_energy();

void attack(int i);

void update_weapons(float time_step);

int create_pistol( Vector2f position);

int create_shotgun(Vector2f position);

int create_sawed_off(Vector2f position);

int create_rifle(Vector2f position);

int create_assault_rifle(Vector2f position);

int create_smg(Vector2f position);

int create_axe(Vector2f position);

int create_sword(Vector2f position);

int create_rope_gun(Vector2f position);

int create_lasersight(Vector2f pos);

int create_ammo(Vector2f position, AmmoType type);
