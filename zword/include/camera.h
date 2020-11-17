#pragma once

#include <SFML/System/Vector2.h>


typedef struct {
    sfVector2f position;
    float zoom;
    float width;
    float height;
} Camera;

sfVector2f world_to_screen(sfVector2f a, Camera* cam);

sfVector2f screen_to_world(sfVector2i a, Camera* cam);
