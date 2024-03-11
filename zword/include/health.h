#pragma once

#include "component.h"
#include "grid.h"


void damage(int entity, sfVector2f pos, sfVector2f dir, int dmg, int dealer);

void blunt_damage(int entity, sfVector2f vel);
