#pragma once

#include "component.h"
#include "grid.h"


int get_akimbo(ComponentData* components, int entity);

void update_energy(ComponentData* components, ColliderGrid* grid);

void attack(ComponentData* component, ColliderGrid* grid, int i);

void reload(ComponentData* component, int i);

void update_weapons(ComponentData* components, float time_step);

int create_pistol(ComponentData* component, sfVector2f position);

int create_shotgun(ComponentData* components, sfVector2f position);

int create_sawed_off(ComponentData* components, sfVector2f position);

int create_rifle(ComponentData* components, sfVector2f position);

int create_assault_rifle(ComponentData* components, sfVector2f position);

int create_smg(ComponentData* components, sfVector2f position);

int create_axe(ComponentData* component, sfVector2f position);

int create_sword(ComponentData* component, sfVector2f position);

int create_rope_gun(ComponentData* components, sfVector2f position);

int create_lasersight(ComponentData* component, sfVector2f pos);

int create_ammo(ComponentData* components, sfVector2f position, AmmoType type);
