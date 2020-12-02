#pragma once

#include <SFML/Window.h>
#include <SFML/System/Vector2.h>

#include "component.h"
#include "grid.h"
#include "camera.h"


void update_enemy(Component* component, ColliderGrid* grid);

void create_enemy(Component* component, sfVector2f pos);

void draw_enemies(Component* component, sfRenderWindow* window, Camera* camera);
