#pragma once

#include "component.h"


#define CHUNK_WIDTH 32
#define CHUNK_HEIGHT 32
#define LEVEL_WIDTH 9
#define LEVEL_HEIGHT 9


void create_level(ComponentData* component, int seed);
