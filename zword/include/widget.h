#pragma once

#include "component.h"


float BUTTON_WIDTH;
float BUTTON_HEIGHT;

int create_window(ComponentData* components, sfVector2f position);

int create_button(ComponentData* components, ButtonText text, sfVector2f position, OnClick on_click);

int create_button_small(ComponentData* components, ButtonText text, sfVector2f position, OnClick on_click);

int create_container(ComponentData* components, sfVector2f position, int height);

int add_button_to_container(ComponentData* components, int container, ButtonText string, OnClick on_click);

int create_dropdown(ComponentData* components, sfVector2f position, ButtonText* strings, int max_value);
