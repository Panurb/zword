#pragma once

#include <SFML/System/Vector2.h>


typedef struct {
    sfVector2f position;
    float zoom;
    float width;
    float height;
    sfRectangleShape* grid[50];
} Camera;

Camera* Camera_create(sfVideoMode mode);

sfVector2f world_to_screen(sfVector2f a, Camera* cam);

sfVector2f screen_to_world(sfVector2i a, Camera* cam);

void draw_grid(sfRenderWindow* window, Camera* camera);

void draw_line(sfRenderWindow* window, Camera* camera, sfRectangleShape* line, sfVector2f start, sfVector2f end, float width, sfColor color);
