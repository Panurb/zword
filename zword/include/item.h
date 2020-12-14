#pragma once

#include "component.h"


void create_flashlight(ComponentData* components, float x, float y);

void pick_up_item(ComponentData* components, int entity);

void drop_item(ComponentData* components, int entity);
