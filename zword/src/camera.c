#define _USE_MATH_DEFINES

#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <stdbool.h>

#include <SFML/System/Vector2.h>
#include <SFML/Window.h>
#include <SFML/Graphics.h>

#include "camera.h"
#include "component.h"
#include "util.h"
#include "image.h"


Camera* Camera_create(sfVideoMode mode) {
    Camera* camera = malloc(sizeof(Camera));
    camera->position = (sfVector2f) { 0.0, 0.0 };
    camera->resolution.x = mode.width;
    camera->resolution.y = mode.height;
    camera->zoom = 25.0 * camera->resolution.y / 720.0;

    camera->shaders[0] = NULL;
    camera->shaders[1] = sfShader_createFromFile(NULL, NULL, "blendColor.frag");

    return camera;
}


sfVector2f world_to_screen(sfVector2f a, Camera* cam) {
    sfVector2f b;
    b.x = (a.x - cam->position.x) * cam->zoom + 0.5 * cam->resolution.x;
    b.y = (cam->position.y - a.y) * cam->zoom + 0.5 * cam->resolution.y;
    return b;
}


sfVector2f screen_to_world(sfVector2i a, Camera* cam) {
    sfVector2f b;
    b.x = (a.x - 0.5 * cam->resolution.x) / cam->zoom + cam->position.x;
    b.y = (0.5 * cam->resolution.y - a.y) / cam->zoom + cam->position.y;
    return b;
}


sfVector2f world_to_texture(sfVector2f a, Camera* cam) {
    sfVector2f b;
    b.x = (a.x - cam->position.x) * cam->zoom + 0.5 * cam->resolution.x;
    b.y = (a.y - cam->position.y) * cam->zoom + 0.5 * cam->resolution.y;
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


void draw_circle(sfRenderWindow* window, Camera* camera, sfCircleShape* shape, sfVector2f position, float radius, sfColor color) {
    bool created = false;
    if (!shape) {
        shape = sfCircleShape_create();
        created = true;
    }

    sfCircleShape_setOrigin(shape, (sfVector2f) { radius * camera->zoom, radius * camera->zoom });
    sfCircleShape_setPosition(shape, world_to_screen(position, camera));
    sfCircleShape_setRadius(shape, radius * camera->zoom);
    sfCircleShape_setFillColor(shape, color);
    sfRenderWindow_drawCircleShape(window, shape, NULL);

    if (created) {
        sfCircleShape_destroy(shape);
    }
}


void draw_ellipse(sfRenderWindow* window, Camera* camera, sfCircleShape* shape, sfVector2f position, float major, float minor, float angle, sfColor color) {
    bool created = false;
    if (!shape) {
        shape = sfCircleShape_create();
        created = true;
    }

    sfCircleShape_setOrigin(shape, (sfVector2f) { major * camera->zoom, major * camera->zoom });
    sfCircleShape_setPosition(shape, world_to_screen(position, camera));
    sfCircleShape_setRadius(shape, major * camera->zoom);
    sfCircleShape_setScale(shape, (sfVector2f) { 1.0, minor / major });
    sfCircleShape_setFillColor(shape, color);
    sfCircleShape_setRotation(shape, -to_degrees(angle));
    sfRenderWindow_drawCircleShape(window, shape, NULL);

    if (created) {
        sfCircleShape_destroy(shape);
    }
}


void draw_rectangle(sfRenderWindow* window, Camera* camera, sfRectangleShape* shape, sfVector2f position, float width, float height, float angle, sfColor color) {
    bool created = false;
    if (!shape) {
        shape = sfRectangleShape_create();
        created = true;
    }

    sfRectangleShape_setOrigin(shape, (sfVector2f) { 0.5 * width * camera->zoom, 0.5 * height * camera->zoom });

    sfRectangleShape_setPosition(shape, world_to_screen(position, camera));

    sfVector2f size = { width * camera->zoom, height * camera->zoom };
    sfRectangleShape_setSize(shape, size);

    sfRectangleShape_setRotation(shape, -to_degrees(angle));

    sfRectangleShape_setFillColor(shape, color);

    sfRenderWindow_drawRectangleShape(window, shape, NULL);

    if (created) {
        sfRectangleShape_destroy(shape);
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

    sfVector2f start = polar_to_cartesian(min_range, ang);
    sfVector2f end = polar_to_cartesian(max_range, ang);

    sfConvexShape_setPoint(shape, 0, world_to_screen(sum(position, start), camera));
    sfConvexShape_setPoint(shape, 1, world_to_screen(sum(position, end), camera));

    int n = 5 * ceil(max_range * spread);

    Matrix2f rot = rotation_matrix(spread / n);

    for (int k = 0; k < n; k++) {
        start = matrix_mult(rot, start);
        end = matrix_mult(rot, end);

        sfConvexShape_setPoint(shape, 2, world_to_screen(sum(position, end), camera));
        sfConvexShape_setPoint(shape, 3, world_to_screen(sum(position, start), camera));

        sfRenderWindow_drawConvexShape(window, shape, NULL);

        sfConvexShape_setPoint(shape, 0, sfConvexShape_getPoint(shape, 3));
        sfConvexShape_setPoint(shape, 1, sfConvexShape_getPoint(shape, 2));
    }
}


void draw_arc(sfRenderWindow* window, Camera* camera, sfRectangleShape* shape, sfVector2f position, float range, float angle, float spread) {
    int n = 5 * ceil(range * spread);

    Matrix2f rot = rotation_matrix(spread / n);

    sfVector2f start = polar_to_cartesian(range, angle - 0.5 * spread);
    sfVector2f end = start;

    for (int k = 0; k < n; k++) {
        end = matrix_mult(rot, end);
        draw_line(window, camera, shape, sum(position, start), sum(position, end), 0.05, sfWhite);
        start = end;
    }
}


void draw_slice_outline(sfRenderWindow* window, Camera* camera, sfRectangleShape* shape, sfVector2f position, float min_range, float max_range, float angle, float spread) {
    sfVector2f start = polar_to_cartesian(max_range, angle - 0.5 * spread);
    sfVector2f end = mult(min_range / max_range, start);

    draw_line(window, camera, shape, sum(position, start), sum(position, end), 0.05, sfWhite);

    draw_arc(window, camera, shape, position, min_range, angle, spread);

    draw_arc(window, camera, shape, position, max_range, angle, spread);

    start = polar_to_cartesian(min_range, angle + 0.5 * spread);
    end = mult(max_range / min_range, start);    
    
    draw_line(window, camera, shape, sum(position, start), sum(position, end), 0.05, sfWhite);
}


void draw_sprite(sfRenderWindow* window, Camera* camera, sfSprite* sprite, sfVector2f position, float angle, sfVector2f scale, int shader_index) {
    sfSprite_setPosition(sprite, world_to_screen(position, camera));

    sfSprite_setScale(sprite, mult(camera->zoom / PIXELS_PER_UNIT, scale));

    sfSprite_setRotation(sprite, -to_degrees(angle));

    sfFloatRect gb = sfSprite_getLocalBounds(sprite);
    sfVector2f origin = { 0.5 * gb.width, 0.5 * gb.height };
    sfSprite_setOrigin(sprite, origin);

    sfShader* shader = camera->shaders[shader_index];
    if (shader_index == 1) {
        sfShader_setTextureUniform(shader, "texture", sfSprite_getTexture(sprite));
        sfShader_setFloatUniform(shader, "amount", 1.0);
    }

    sfRenderStates state = { sfBlendAlpha, sfTransform_Identity, NULL, shader };
    sfRenderWindow_drawSprite(window, sprite, &state);
}


void update_camera(ComponentData* components, Camera* camera, float time_step) {
    sfVector2f pos = zeros();

    int n = 0;
    for (int i = 0; i < components->entities; i++) {
        PlayerComponent* player = PlayerComponent_get(components, i);
        if (player) {
            n += 1;
            if (player->vehicle == -1) {
                pos = sum(pos, get_position(components, i));
            } else {
                pos = sum(pos, get_position(components, i));
            }
        }
    }
    pos = mult(1.0 / n, pos);

    camera->position = sum(camera->position, mult(10.0 * time_step, diff(pos, camera->position)));
    camera->zoom += 10.0 * time_step * (25.0 * camera->resolution.y / 720.0 - camera->zoom);
}


bool on_screen(sfVector2f position, float width, float height, Camera* camera) {
    if (fabs(position.x - camera->position.x) < width + 0.5 * camera->resolution.x / camera->zoom) {
        if (fabs(position.y - camera->position.y) < height + 0.5 * camera->resolution.y / camera->zoom) {
            return true;
        }
    }
    return false;
}
