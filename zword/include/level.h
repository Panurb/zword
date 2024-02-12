#pragma once

#include "component.h"
#include "grid.h"


#define CHUNK_WIDTH 32
#define CHUNK_HEIGHT 32
#define LEVEL_WIDTH 13
#define LEVEL_HEIGHT 13


void create_level(ComponentData* component, ColliderGrid* grid, int seed);

void test(ComponentData* components);
