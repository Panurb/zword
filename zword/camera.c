#include <SFML/System/Vector2.h>

#include "camera.h"

sfVector2f world_to_screen(sfVector2f a, Camera* cam) {
    sfVector2f b;
    b.x = (a.x - cam->position.x) * cam->zoom + 0.5 * cam->width;
    b.y = (cam->position.y - a.y) * cam->zoom + 0.5 * cam->height;
    return b;
}


sfVector2f screen_to_world(sfVector2i a, Camera* cam) {
    sfVector2f b;
    b.x = (a.x - 0.5 * cam->width) / cam->zoom + cam->position.x;
    b.y = (0.5 * cam->height - a.y) / cam->zoom - cam->position.y;
    return b;
}
