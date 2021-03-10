#pragma once

#include "component.h"
#include "grid.h"


#define CHUNK_WIDTH 32
#define CHUNK_HEIGHT 32
#define LEVEL_WIDTH 9
#define LEVEL_HEIGHT 9


void create_level(ComponentData* component, ColliderGrid* grid, int seed);
