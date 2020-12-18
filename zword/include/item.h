#pragma once

#include "component.h"


void create_flashlight(ComponentData* components, sfVector2f position);

void pick_up_item(ComponentData* components, int entity);

void drop_item(ComponentData* components, int entity);
