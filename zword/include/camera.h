#pragma once

#include <SFML/System/Vector2.h>

#include "component.h"


typedef enum {
    SHADER_NONE,
    SHADER_OUTLINE
} CameraShader;

int create_camera(ComponentData* components, sfVideoMode mode);

sfVector2f camera_size(ComponentData* components, int camera);

sfVector2f world_to_screen(ComponentData* components, int camera, sfVector2f a);

sfVector2f screen_to_world(ComponentData* components, int camera, sfVector2i a);

sfVector2f world_to_texture(ComponentData* components, int camera, sfVector2f a);

void draw_line(sfRenderWindow* window, ComponentData* components, int camera, sfRectangleShape* line, sfVector2f start, sfVector2f end, float width, sfColor color);

void draw_circle(sfRenderWindow* window, ComponentData* components, int camera, sfCircleShape* shape, sfVector2f position, float radius, sfColor color);

void draw_ellipse(sfRenderWindow* window, ComponentData* components, int camera, sfCircleShape* shape, sfVector2f position, float major, float minor, float angle, sfColor color);

void draw_rectangle(sfRenderWindow* window, ComponentData* components, int camera, sfRectangleShape* shape, sfVector2f position, float width, float height, float angle, sfColor color);

void draw_rectangle_outline(sfRenderWindow* window, ComponentData* components, int camera, sfRectangleShape* shape, 
    sfVector2f position, float width, float height, float angle, float line_width, sfColor color);

void draw_cone(sfRenderWindow* window, ComponentData* components, int camera, sfConvexShape* shape, int n, sfVector2f position, float range, float angle, float spread);

void draw_slice(sfRenderWindow* window, ComponentData* components, int camera, sfVertexArray* verts, int verts_size, sfVector2f position, float min_range, float max_range, float angle, float spread, sfColor color);

void draw_slice_outline(sfRenderWindow* window, ComponentData* components, int camera, sfRectangleShape* shape, sfVector2f position, float min_range, float max_range, float angle, float spread);

void draw_sprite(sfRenderWindow* window, ComponentData* components, int camera, sfSprite* sprite, sfVector2f position, float angle, sfVector2f scale, int shader_index);

void draw_text(sfRenderWindow* window, ComponentData* components, int camera, sfText* text, sfVector2f position, char string[100], sfColor color);

void update_camera(ComponentData* components, int camera, float time_step, bool follow_players);

bool on_screen(ComponentData* components, int camera, sfVector2f position, float width, float height);

void shake_camera(ComponentData* components, float speed);

void draw_overlay(sfRenderWindow* window, ComponentData* components, int camera);
