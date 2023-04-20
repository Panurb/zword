#pragma once

#define _USE_MATH_DEFINES

#include <math.h>

#include "component.h"


int create_wall(ComponentData* components, sfVector2f pos, float angle, float width, float height, Filename filename);

void create_object(ComponentData* components, int object, sfVector2f position, float angle);

void create_church(ComponentData* components, sfVector2f pos);

void create_barn(ComponentData* components, sfVector2f pos);

void create_house(ComponentData* components, sfVector2f pos);

void create_outhouse(ComponentData* components, sfVector2f pos);

void create_garage(ComponentData* components, sfVector2f pos);

void create_mansion(ComponentData* components, sfVector2f pos);

void create_school(ComponentData* components, sfVector2f pos);
