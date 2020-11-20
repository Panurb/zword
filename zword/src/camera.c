#include <stdio.h>
#include <math.h>

#include <SFML/System/Vector2.h>
#include <SFML/Window.h>
#include <SFML/Graphics.h>

#include "camera.h"
#include "util.h"


Camera* Camera_create(sfVideoMode mode) {
    Camera* camera = malloc(sizeof(Camera));
    camera->position = (sfVector2f) { 0.0, 0.0 };
    camera->zoom = 50.0;
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


void draw_line(sfRenderWindow* window, sfRectangleShape* line, sfVector2f start, sfVector2f end) {
    sfVector2f r = diff(end, start);

    sfRectangleShape_setPosition(line, start);
    
    sfRectangleShape_setSize(line, (sfVector2f) { dist(start, end), 1.0 });

    sfRectangleShape_setRotation(line, to_degrees(atan2(r.y, r.x)));

    sfRenderWindow_drawRectangleShape(window, line, NULL);
}


void draw_grid(sfRenderWindow* window, Camera* camera) {
    int nx = ceil(camera->width / camera->zoom);
    for (int i = 0; i < nx; i++) {
        float offset = -camera->zoom * (fmod(camera->position.x, 1.0) - 1.0);
        sfVector2f start = { i * camera->zoom + offset, 0.0 };
        sfVector2f end = { i * camera->zoom + offset, camera->height };

        draw_line(window, camera->grid[i], start, end);
    }

    int ny = ceil(camera->height / camera->zoom) + 1;
    for (int i = nx; i < nx + ny; i++) {
        float offset = camera->zoom * (fmod(camera->position.y, 1.0));
        sfVector2f start = { 0.0, (i - nx) * camera->zoom + offset };
        sfVector2f end = { camera->width, (i - nx) * camera->zoom + offset };

        draw_line(window, camera->grid[i], start, end);
    }
}
