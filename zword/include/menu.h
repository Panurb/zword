#include "util.h"


void create_menu(GameData data);

void update_menu(GameData data, sfRenderWindow* window, ButtonMenu menu);

void input_menu(ComponentData* components, sfEvent event);

void draw_menu(GameData data, sfRenderWindow* window, ButtonMenu menu);

void update_buttons(ComponentData* components, sfRenderWindow* window, int camera, ButtonMenu menu);

void draw_buttons(ComponentData* components, sfRenderWindow* window, int camera, ButtonMenu menu);
