#define _USE_MATH_DEFINES

#include <stdlib.h>
#include <math.h>

#include "component.h"
#include "perlin.h"
#include "camera.h"
#include "image.h"


void create_road(ComponentData* components, sfVector2f start, sfVector2f end);

void create_river(ComponentData* components, sfVector2f start, sfVector2f end);

void draw_road(ComponentData* components, sfRenderWindow* window, int camera, TextureArray textures, int entity);

void resize_roads(ComponentData* components);
