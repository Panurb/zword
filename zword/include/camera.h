#pragma once

#include <SFML/System/Vector2.h>

#include "component.h"


typedef struct {
    sfVector2f position;
    float zoom;
    float width;
    float height;
    sfShader* shaders[10];
} Camera;

Camera* Camera_create(sfVideoMode mode);

sfVector2f world_to_screen(sfVector2f a, Camera* cam);

sfVector2f screen_to_world(sfVector2i a, Camera* cam);

sfVector2f world_to_texture(sfVector2f a, Camera* cam);

void draw_line(sfRenderWindow* window, Camera* camera, sfRectangleShape* line, sfVector2f start, sfVector2f end, float width, sfColor color);

void draw_circle(sfRenderWindow* window, Camera* camera, sfCircleShape* shape, sfVector2f position, float radius, sfColor color);

void draw_rectangle(sfRenderWindow* window, Camera* camera, sfRectangleShape* shape, sfVector2f position, float width, float height, float angle, sfColor color);

void draw_cone(sfRenderWindow* window, Camera* camera, sfConvexShape* shape, int n, sfVector2f position, float range, float angle, float spread);

void draw_slice(sfRenderWindow* window, Camera* camera, sfConvexShape* shape, sfVector2f position, float min_range, float max_range, float angle, float spread);

void draw_slice_outline(sfRenderWindow* window, Camera* camera, sfRectangleShape* shape, sfVector2f position, float min_range, float max_range, float angle, float spread);

void draw_sprite(sfRenderWindow* window, Camera* camera, sfSprite* sprite, sfVector2f position, float angle, sfVector2f scale, int shader_index);

void update_camera(ComponentData* component, Camera* camera, float time_step);
