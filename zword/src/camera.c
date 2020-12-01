#define _USE_MATH_DEFINES

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
    for (int i = 0; i < 100; i++) {
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
    for (int i = 0; i < nx; i++) {
        sfVector2f start = { x + i, camera->position.y + 0.5 * camera->height / camera->zoom };
        sfVector2f end = { x + i, camera->position.y - 0.5 * camera->height / camera->zoom };

        draw_line(window, camera, camera->grid[i], start, end, 0.02, sfColor_fromRGB(150, 150, 150));
    }

    int ny = ceil(camera->height / camera->zoom);    
    float y = floor(camera->position.y - 0.5 * camera->height / camera->zoom);
    for (int i = nx; i <= nx + ny; i++) {
        sfVector2f start = { camera->position.x - 0.5 * camera->width / camera->zoom, y + (i - nx) };
        sfVector2f end = { camera->position.x + 0.5 * camera->width / camera->zoom, y + (i - nx) };

        draw_line(window, camera, camera->grid[i], start, end, 0.02, sfColor_fromRGB(150, 150, 150));
    }
}


void draw_cone(sfRenderWindow* window, Camera* camera, sfConvexShape* shape, int n, sfVector2f position, float range, float angle, float spread) {
    sfConvexShape_setPoint(shape, 0, world_to_screen(position, camera));
    sfConvexShape_setPoint(shape, n - 1, world_to_screen(position, camera));

    angle -= 0.5 * spread;

    for (int k = 1; k < n - 1; k++) {
        sfVector2f point = sum(position, polar_to_cartesian(range, angle));

        sfConvexShape_setPoint(shape, k, world_to_screen(point, camera));

        angle += spread / (n - 1);
    }

    sfRenderWindow_drawConvexShape(window, shape, NULL);
}


void draw_slice(sfRenderWindow* window, Camera* camera, sfConvexShape* shape, sfVector2f position, float min_range, float max_range, float angle, float spread) {
    float ang = angle - 0.5 * spread;

    sfVector2f start = sum(position, polar_to_cartesian(min_range, ang));
    sfVector2f end = sum(position, polar_to_cartesian(max_range, ang));

    sfConvexShape_setPoint(shape, 0, world_to_screen(start, camera));
    sfConvexShape_setPoint(shape, 1, world_to_screen(end, camera));

    int n = 20;
    for (int k = 0; k < n; k++) {
        ang += spread / n;

        start = sum(position, polar_to_cartesian(min_range, ang));
        end = sum(position, polar_to_cartesian(max_range, ang));

        sfConvexShape_setPoint(shape, 2, world_to_screen(end, camera));
        sfConvexShape_setPoint(shape, 3, world_to_screen(start, camera));

        sfRenderWindow_drawConvexShape(window, shape, NULL);

        sfConvexShape_setPoint(shape, 0, world_to_screen(start, camera));
        sfConvexShape_setPoint(shape, 1, world_to_screen(end, camera));
    }
}
