#pragma once

#include "util.h"
#include "component.h"
#include "game.h"


void get_map_name(GameData* data, ButtonText buffer);

void create_menu(GameData data);

void destroy_menu(GameData data);

void create_pause_menu(GameData data);

void update_menu(GameData data, sfRenderWindow* window);

void input_menu(ComponentData* components, int camera, sfEvent event);

void draw_menu(GameData data, sfRenderWindow* window);

void update_widgets(ComponentData* components, sfRenderWindow* window, int camera);

void draw_widgets(ComponentData* components, sfRenderWindow* window, int camera);