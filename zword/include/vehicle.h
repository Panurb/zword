#pragma once

#include "component.h"


void create_car(ComponentData* component, sfVector2f pos);

bool enter_vehicle(ComponentData* component, int i);

void exit_vehicle(ComponentData* component, int i);

void drive_vehicle(ComponentData* component, int i, sfVector2f v, float delta_time);
