#pragma once

#include "game.h"


void update_editor(GameData data, sfRenderWindow* window, float time_step);

void input_editor(ComponentData* components, int camera, sfEvent event);

void draw_editor(GameData data, sfRenderWindow* window);
