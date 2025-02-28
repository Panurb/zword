#pragma once

#include "component.h"
#include "grid.h"


typedef enum {
    DAMAGE_BLUNT,
    DAMAGE_BULLET,
    DAMAGE_BURN
} DamageType;


void damage(int entity, Vector2f pos, Vector2f dir, int dmg, int dealer, DamageType type);

void blunt_damage(int entity, Vector2f vel);

void burn(int entity);

void update_health(float time_step);
