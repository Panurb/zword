#pragma once

#include "game.h"


void update_editor(GameData data, sfRenderWindow* window, float time_step);

void input_editor(GameData data, sfRenderWindow* window, sfEvent event);

void draw_editor(GameData data, sfRenderWindow* window);
