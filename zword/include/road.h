#define _USE_MATH_DEFINES

#include <stdlib.h>
#include <math.h>

#include "component.h"
#include "perlin.h"
#include "camera.h"
#include "image.h"


void create_road(ComponentData* components, sfVector2f start, sfVector2f end, Permutation perm);

void draw_road(ComponentData* components, sfRenderWindow* window, int camera, TextureArray textures, int entity);