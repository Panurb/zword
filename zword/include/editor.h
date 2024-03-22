#pragma once

#include "game.h"


void create_editor_menu(GameData* data);

void update_editor(GameData data, sfRenderWindow* window, float time_step);

void input_editor(sfEvent event);

void draw_editor();
