#pragma once

#include "component.h"
#include "grid.h"


void damage(int entity, Vector2f pos, Vector2f dir, int dmg, int dealer, DamageType type);

void blunt_damage(int entity, Vector2f vel);

void burn(int entity);

void freeze(Entity entity);

void update_health(float time_step);
