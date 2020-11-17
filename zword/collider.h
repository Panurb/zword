#pragma once

#include <SFML/System/Vector2.h>

#include "component.h"
#include "camera.h"


void collide(Component* component);

void debug_draw(Component* component, sfRenderWindow* window, Camera* camera);
