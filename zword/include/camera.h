#pragma once

#include "component.h"


typedef enum {
    SHADER_NONE,
    SHADER_OUTLINE
} CameraShader;

int create_camera();

int create_menu_camera();

Vector2f camera_size(int camera);

sfVector2f world_to_screen(int camera, Vector2f a);

Vector2f sdl_world_to_screen(int camera, Vector2f a);

Vector2f screen_to_world(int camera, sfVector2i a);

Vector2f sdl_screen_to_world(int camera, Vector2f a);

sfVector2f world_to_texture(int camera, Vector2f a);

void draw_line(int camera, Vector2f start, Vector2f end, float width, sfColor color);

void draw_circle(int camera, Vector2f position, float radius, sfColor color);

void draw_circle_outline(int camera, Vector2f position, float radius, float line_width, sfColor color);

void draw_ellipse(int camera, Vector2f position, float major, float minor, float angle, sfColor color);

void draw_rectangle(int camera, Vector2f position, float width, float height, float angle, sfColor color);

void draw_rectangle_outline(int camera, Vector2f position, float width, float height, 
    float angle, float line_width, sfColor color);

void draw_slice(int camera, Vector2f position, float min_range, float max_range, 
    float angle, float spread, sfColor color);

void draw_slice_outline(int camera, Vector2f position, float min_range, float max_range, 
    float angle, float spread);

void draw_sprite(int camera, Filename filename, float width, float height, int offset, Vector2f position, float angle, 
    Vector2f scale, float alpha, int shader_index);

void draw_text(int camera, Vector2f position, char string[100], int size, sfColor color);

void update_camera(int camera, float time_step, bool follow_players);

bool on_screen(int camera, Vector2f position, float width, float height);

void shake_camera(float speed);

void draw_overlay(int camera, float alpha);
