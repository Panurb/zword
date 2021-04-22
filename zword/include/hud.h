#pragma once

#include "component.h"

void draw_menu_slot(ComponentData* components, sfRenderWindow* window, int camera, int entity, int slot, float offset, float alpha);

void draw_menu_attachment(ComponentData* components, sfRenderWindow* window, int camera, int entity, int slot, int atch, float offset, float alpha);

void draw_hud(ComponentData* components, sfRenderWindow* window, int camera);
