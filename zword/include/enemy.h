#pragma once

#include <SFML/Window.h>
#include <SFML/System/Vector2.h>

#include "component.h"
#include "grid.h"
#include "camera.h"


void update_enemies(ComponentData* component, ColliderGrid* grid);

void create_enemy(ComponentData* component, sfVector2f pos);

void draw_enemies(ComponentData* component, sfRenderWindow* window, int camera);
