#pragma once

#include "component.h"
#include "grid.h"


int create_car(ComponentData* component, sfVector2f pos, float angle);

bool enter_vehicle(ComponentData* component, ColliderGrid* grid, int i);

void exit_vehicle(ComponentData* component, int i);

void drive_vehicle(ComponentData* component, int i, float gas, float steering);
