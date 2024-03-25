#pragma once

#include "component.h"


#define BUTTON_WIDTH 8.0f
#define BUTTON_HEIGHT 2.0f
#define BORDER_WIDTH 0.2f
#define SCROLLBAR_WIDTH 1.0f
#define COLOR_CONTAINER get_color(0.15f, 0.15f, 0.15f, 1.0f)
#define COLOR_SHADOW get_color(0.2f, 0.2f, 0.2f, 1.0f)
#define COLOR_BUTTON get_color(0.3f, 0.3f, 0.3f, 1.0f)
#define COLOR_SELECTED get_color(0.4f, 0.4f, 0.4f, 1.0f)
#define COLOR_BORDER get_color(0.1f, 0.1f, 0.1f, 1.0f)
#define COLOR_TEXT sfWhite

void bring_to_top(int entity);

int create_window(sfVector2f position, ButtonText text, int width, OnClick on_close);

int create_label(ButtonText text, sfVector2f position);

int create_button(ButtonText text, sfVector2f position, OnClick on_click);

int create_button_small(ButtonText text, sfVector2f position, OnClick on_click);

int create_container(sfVector2f position, int width, int height);

void add_widget_to_container(int container, int entity);

int add_button_to_container(int container, ButtonText string, OnClick on_click);

void increment_value(int entity, int direction);

void add_row_to_container(int container, int left, int right);

void add_files_to_container(int container, Filename path, OnClick on_click);

int create_dropdown(sfVector2f position, ButtonText* strings, int size);

void set_slider(int entity, sfVector2f mouse_position);

int create_slider(sfVector2f position, int min_value, int max_value, int value, 
    OnChange on_change);

void set_scrollbar(int entity, sfVector2f mouse_position);

int create_scrollbar(sfVector2f position, int height, int max_value, OnChange on_change);

void add_scrollbar_to_container(int container);

int create_textbox(sfVector2f position, int width);

void update_widgets(int camera);

void draw_widgets(int camera);

bool input_widgets(int camera, sfEvent event);

void destroy_widgets();
