#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <stdbool.h>

#include <SFML/System/Vector2.h>
#include <SFML/Window.h>
#include <SFML/Graphics.h>

#include "camera.h"
#include "util.h"


Camera* Camera_create(sfVideoMode mode) {
    Camera* camera = malloc(sizeof(Camera));
    camera->position = (sfVector2f) { 0.0, 0.0 };
    camera->zoom = 25.0;
    camera->width = mode.width;
    camera->height = mode.height;
    for (int i = 0; i < 50; i++) {
        camera->grid[i] = sfRectangleShape_create();
        sfRectangleShape_setFillColor(camera->grid[i], sfWhite);
    }
    return camera;
}


sfVector2f world_to_screen(sfVector2f a, Camera* cam) {
    sfVector2f b;
    b.x = (a.x - cam->position.x) * cam->zoom + 0.5 * cam->width;
    b.y = (cam->position.y - a.y) * cam->zoom + 0.5 * cam->height;
    return b;
}


sfVector2f screen_to_world(sfVector2i a, Camera* cam) {
    sfVector2f b;
    b.x = (a.x - 0.5 * cam->width) / cam->zoom + cam->position.x;
    b.y = (0.5 * cam->height - a.y) / cam->zoom + cam->position.y;
    return b;
}


sfVector2f world_to_texture(sfVector2f a, Camera* cam) {
    sfVector2f b;
    b.x = (a.x - cam->position.x) * cam->zoom + 0.5 * cam->width;
    b.y = (a.y - cam->position.y) * cam->zoom + 0.5 * cam->height;
    return b;
}


void draw_line(sfRenderWindow* window, Camera* camera, sfRectangleShape* line, sfVector2f start, sfVector2f end, float width, sfColor color) {
    bool created = false;
    if (!line) {
        line = sfRectangleShape_create();
        created = true;
    }

    sfVector2f r = diff(end, start);

    sfRectangleShape_setPosition(line, world_to_screen(start, camera));

    sfRectangleShape_setFillColor(line, color);
    
    sfRectangleShape_setSize(line, (sfVector2f) { dist(start, end) * camera->zoom, width * camera->zoom });

    sfRectangleShape_setRotation(line, to_degrees(-atan2(r.y, r.x)));

    sfRenderWindow_drawRectangleShape(window, line, NULL);

    if (created) {
        sfRectangleShape_destroy(line);
    }
}


void draw_grid(sfRenderWindow* window, Camera* camera) {
    int nx = ceil(camera->width / camera->zoom);
    float x = floor(camera->position.x - 0.5 * camera->width / camera->zoom);
    for (int i = 0; i <= nx; i++) {
        sfVector2f start = { x + i, camera->position.y + 0.5 * camera->height / camera->zoom };
        sfVector2f end = { x + i, camera->position.y - 0.5 * camera->height / camera->zoom };

        draw_line(window, camera, camera->grid[i], start, end, 0.02, sfColor_fromRGB(50, 50, 50));
    }

    int ny = ceil(camera->height / camera->zoom);    
    float y = floor(camera->position.y - 0.5 * camera->height / camera->zoom);
    for (int i = nx; i <= nx + ny; i++) {
        sfVector2f start = { camera->position.x - 0.5 * camera->width / camera->zoom, y + (i - nx) };
        sfVector2f end = { camera->position.x + 0.5 * camera->width / camera->zoom, y + (i - nx) };

        draw_line(window, camera, camera->grid[i], start, end, 0.02, sfColor_fromRGB(50, 50, 50));
    }
}
