#pragma once

#include "component.h"


void die(ComponentData* components, int entity);

void damage(ComponentData* components, int entity, sfVector2f pos, sfVector2f dir, int dmg);
