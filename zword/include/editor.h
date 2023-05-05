#pragma once

#include "game.h"


void set_map_name(ButtonText name);

void create_editor_menu(GameData* data);

void update_editor(GameData data, sfRenderWindow* window, float time_step);

void input_editor(GameData* data, sfRenderWindow* window, sfEvent event);

void draw_editor(GameData data, sfRenderWindow* window);
