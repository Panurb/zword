#pragma once

#include "component.h"


float BUTTON_WIDTH;
float BUTTON_HEIGHT;

void bring_to_top(ComponentData* components, int entity);

int create_window(ComponentData* components, sfVector2f position, ButtonText text, OnClick on_close);

int create_label(ComponentData* components, ButtonText text, sfVector2f position);

int create_button(ComponentData* components, ButtonText text, sfVector2f position, OnClick on_click);

int create_button_small(ComponentData* components, ButtonText text, sfVector2f position, OnClick on_click);

int create_container(ComponentData* components, sfVector2f position, int width, int height);

void add_widget_to_container(ComponentData* components, int container, int entity);

int add_button_to_container(ComponentData* components, int container, ButtonText string, OnClick on_click);

void increment_value(ComponentData* components, int entity, int direction);

void add_row_to_container(ComponentData* components, int container, int left, int right);

int create_dropdown(ComponentData* components, sfVector2f position, ButtonText* strings, int size);

void set_slider(ComponentData* components, int entity, sfVector2f mouse_position);

int create_slider(ComponentData* components, sfVector2f position, int min_value, int max_value, int value);
