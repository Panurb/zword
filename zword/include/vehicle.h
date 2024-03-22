#pragma once

#include "component.h"
#include "grid.h"


int create_car(sfVector2f pos, float angle);

bool enter_vehicle(int i);

void exit_vehicle(int i);

void drive_vehicle(int i, float gas, float steering);
