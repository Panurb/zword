#pragma once

#include "component.h"
#include "grid.h"


#define CHUNK_WIDTH 32
#define CHUNK_HEIGHT 32
#define LEVEL_WIDTH 9
#define LEVEL_HEIGHT 9


void create_decal(ComponentData* components, sfVector2f pos, float width, float height, Filename filename);

void create_level(ComponentData* component, ColliderGrid* grid, int seed);

void test(ComponentData* components, ColliderGrid* grid);
