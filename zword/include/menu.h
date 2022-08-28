#include "util.h"


void create_menu(GameData data);

void update_menu(GameData data, sfRenderWindow* window);

void input_menu(ComponentData* components, int camera, sfEvent event);

void draw_menu(GameData data, sfRenderWindow* window);

void update_buttons(ComponentData* components, sfRenderWindow* window, int camera);

void draw_buttons(ComponentData* components, sfRenderWindow* window, int camera);
