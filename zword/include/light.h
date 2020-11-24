#pragma once

#include <SFML/Graphics.h>

#include "component.h"
#include "camera.h"


void draw_light(Component* component, ColliderGrid* grid, sfRenderWindow* window, Camera* camera);
