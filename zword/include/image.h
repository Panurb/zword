#pragma once

#include <SFML/Graphics.h>

#include "component.h"
#include "camera.h"


sfSprite* load_sprite(char filename[20]);

void draw(Component* component, sfRenderWindow* window,  Camera* camera);