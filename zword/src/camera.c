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


int create_camera(ComponentData* components, sfVideoMode mode) {
    int i = create_entity(components);
    CoordinateComponent_add(components, i, zeros(), 0.0);
    CameraComponent_add(components, i, (sfVector2i) { mode.width, mode.height });
    ParticleComponent* part = ParticleComponent_add(components, i, 0.0, 2 * M_PI, 0.1, 0.1, 1.0, 1.0, sfWhite, sfWhite);
    part->enabled = true;
    part->loop = true;
    return i;
}


sfVector2f world_to_screen(ComponentData* components, int camera, sfVector2f a) {
    CameraComponent* cam = CameraComponent_get(components, camera);
    sfVector2f pos = sum(get_position(components, camera), cam->shake.position);
    sfVector2f b;
    b.x = (a.x - pos.x) * cam->zoom + 0.5 * cam->resolution.x;
    b.y = (pos.y - a.y) * cam->zoom + 0.5 * cam->resolution.y;
    return b;
}


sfVector2f screen_to_world(ComponentData* components, int camera, sfVector2i a) {
    CameraComponent* cam = CameraComponent_get(components, camera);
    sfVector2f pos = sum(get_position(components, camera), cam->shake.position);
    sfVector2f b;
    b.x = (a.x - 0.5 * cam->resolution.x) / cam->zoom + pos.x;
    b.y = (0.5 * cam->resolution.y - a.y) / cam->zoom + pos.y;
    return b;
}


sfVector2f world_to_texture(ComponentData* components, int camera, sfVector2f a) {
    CameraComponent* cam = CameraComponent_get(components, camera);
    sfVector2f pos = sum(get_position(components, camera), cam->shake.position);
    sfVector2f b;
    b.x = (a.x - pos.x) * cam->zoom + 0.5 * cam->resolution.x;
    b.y = (a.y - pos.y) * cam->zoom + 0.5 * cam->resolution.y;
    return b;
}


void draw_line(sfRenderWindow* window, ComponentData* components, int camera, sfRectangleShape* line, sfVector2f start, sfVector2f end, float width, sfColor color) {
    CameraComponent* cam = CameraComponent_get(components, camera);
    
    bool created = false;
    if (!line) {
        line = sfRectangleShape_create();
        created = true;
    }

    sfVector2f r = diff(end, start);
    sfRectangleShape_setPosition(line, world_to_screen(components, camera, start));
    sfRectangleShape_setFillColor(line, color);
    sfRectangleShape_setSize(line, (sfVector2f) { dist(start, end) * cam->zoom, width * cam->zoom });
    sfRectangleShape_setRotation(line, to_degrees(-atan2(r.y, r.x)));
    sfRenderWindow_drawRectangleShape(window, line, NULL);

    if (created) {
        sfRectangleShape_destroy(line);
    }
}


void draw_circle(sfRenderWindow* window, ComponentData* components, int camera, sfCircleShape* shape, sfVector2f position, float radius, sfColor color) {
    CameraComponent* cam = CameraComponent_get(components, camera);

    bool created = false;
    if (!shape) {
        shape = sfCircleShape_create();
        created = true;
    }

    sfCircleShape_setOrigin(shape, (sfVector2f) { radius * cam->zoom, radius * cam->zoom });
    sfCircleShape_setPosition(shape, world_to_screen(components, camera, position));
    sfCircleShape_setRadius(shape, radius * cam->zoom);
    sfCircleShape_setFillColor(shape, color);
    sfRenderWindow_drawCircleShape(window, shape, NULL);

    if (created) {
        sfCircleShape_destroy(shape);
    }
}


void draw_ellipse(sfRenderWindow* window, ComponentData* components, int camera, sfCircleShape* shape, sfVector2f position, float major, float minor, float angle, sfColor color) {
    CameraComponent* cam = CameraComponent_get(components, camera);

    bool created = false;
    if (!shape) {
        shape = sfCircleShape_create();
        created = true;
    }

    sfCircleShape_setOrigin(shape, (sfVector2f) { major * cam->zoom, major * cam->zoom });
    sfCircleShape_setPosition(shape, world_to_screen(components, camera, position));
    sfCircleShape_setRadius(shape, major * cam->zoom);
    sfCircleShape_setScale(shape, (sfVector2f) { 1.0, minor / major });
    sfCircleShape_setFillColor(shape, color);
    sfCircleShape_setRotation(shape, -to_degrees(angle));
    sfRenderWindow_drawCircleShape(window, shape, NULL);

    if (created) {
        sfCircleShape_destroy(shape);
    }
}


void draw_rectangle(sfRenderWindow* window, ComponentData* components, int camera, sfRectangleShape* shape, sfVector2f position, float width, float height, float angle, sfColor color) {
    CameraComponent* cam = CameraComponent_get(components, camera);

    bool created = false;
    if (!shape) {
        shape = sfRectangleShape_create();
        created = true;
    }

    sfRectangleShape_setOrigin(shape, (sfVector2f) { 0.5 * width * cam->zoom, 0.5 * height * cam->zoom });
    sfRectangleShape_setPosition(shape, world_to_screen(components, camera, position));
    sfVector2f size = { width * cam->zoom, height * cam->zoom };
    sfRectangleShape_setSize(shape, size);
    sfRectangleShape_setRotation(shape, -to_degrees(angle));
    sfRectangleShape_setFillColor(shape, color);
    sfRenderWindow_drawRectangleShape(window, shape, NULL);

    if (created) {
        sfRectangleShape_destroy(shape);
    }
}


void draw_cone(sfRenderWindow* window, ComponentData* components, int camera, sfConvexShape* shape, int n, sfVector2f position, float range, float angle, float spread) {
    bool created = false;
    if (!shape) {
        shape = sfConvexShape_create();
        sfConvexShape_setPointCount(shape, n);
    }
    
    sfVector2f start = world_to_screen(components, camera, position);
    sfConvexShape_setPoint(shape, 0, start);
    sfConvexShape_setPoint(shape, n - 1, start);

    angle -= 0.5 * spread;

    for (int i = 1; i < n - 1; i++) {
        sfVector2f point = sum(position, polar_to_cartesian(range, angle));
        sfConvexShape_setPoint(shape, i, world_to_screen(components, camera, point));
        angle += spread / (n - 3);
    }

    sfRenderWindow_drawConvexShape(window, shape, NULL);

    if (created) {
        sfConvexShape_destroy(shape);
    }
}


void draw_slice(sfRenderWindow* window, ComponentData* components, int camera, sfVertexArray* verts, int verts_size, sfVector2f position, float min_range, float max_range, float angle, float spread, sfColor color) {
    bool created = false;
    if (!verts) {
        verts = sfVertexArray_create();
        sfVertexArray_setPrimitiveType(verts, sfTriangleStrip);
        sfVertexArray_resize(verts, verts_size);
    }

    sfVector2f start = polar_to_cartesian(min_range, angle - 0.5 * spread);
    sfVector2f end = polar_to_cartesian(max_range, angle - 0.5 * spread);

    Matrix2f rot = rotation_matrix(2.0 * spread / verts_size);

    for (int k = 0; k < verts_size / 2; k += 1) {
        sfVertex* v = sfVertexArray_getVertex(verts, 2 * k);
        v->position = world_to_screen(components, camera, sum(position, start));
        v->color = color;

        v = sfVertexArray_getVertex(verts, 2 * k + 1);
        v->position = world_to_screen(components, camera, sum(position, end));
        v->color = color;

        start = matrix_mult(rot, start);
        end = matrix_mult(rot, end);
    }

    sfRenderWindow_drawVertexArray(window, verts, NULL);

    if (created) {
        sfVertexArray_destroy(verts);
    }
}


void draw_arc(sfRenderWindow* window, ComponentData* components, int camera, sfRectangleShape* shape, sfVector2f position, float range, float angle, float spread) {
    int n = 5 * ceil(range * spread);

    Matrix2f rot = rotation_matrix(spread / n);

    sfVector2f start = polar_to_cartesian(range, angle - 0.5 * spread);
    sfVector2f end = start;

    for (int k = 0; k < n; k++) {
        end = matrix_mult(rot, end);
        draw_line(window, components, camera, shape, sum(position, start), sum(position, end), 0.05, sfWhite);
        start = end;
    }
}


void draw_slice_outline(sfRenderWindow* window, ComponentData* components, int camera, sfRectangleShape* shape, sfVector2f position, float min_range, float max_range, float angle, float spread) {
    sfVector2f start = polar_to_cartesian(max_range, angle - 0.5 * spread);
    sfVector2f end = mult(min_range / max_range, start);

    draw_line(window, components, camera, shape, sum(position, start), sum(position, end), 0.05, sfWhite);

    draw_arc(window, components, camera, shape, position, min_range, angle, spread);

    draw_arc(window, components, camera, shape, position, max_range, angle, spread);

    start = polar_to_cartesian(min_range, angle + 0.5 * spread);
    end = mult(max_range / min_range, start);    
    
    draw_line(window, components, camera, shape, sum(position, start), sum(position, end), 0.05, sfWhite);
}


void draw_sprite(sfRenderWindow* window, ComponentData* components, int camera, sfSprite* sprite, sfVector2f position, float angle, sfVector2f scale, int shader_index) {
    CameraComponent* cam = CameraComponent_get(components, camera);

    sfSprite_setPosition(sprite, world_to_screen(components, camera, position));
    sfSprite_setScale(sprite, mult(cam->zoom / PIXELS_PER_UNIT, scale));
    sfSprite_setRotation(sprite, -to_degrees(angle));

    sfFloatRect gb = sfSprite_getLocalBounds(sprite);
    sfVector2f origin = { 0.5 * gb.width, 0.5 * gb.height };
    sfSprite_setOrigin(sprite, origin);

    sfShader* shader = cam->shaders[shader_index];

    sfRenderStates state = { sfBlendAlpha, sfTransform_Identity, NULL, shader };
    sfRenderWindow_drawSprite(window, sprite, &state);
}


void draw_text(sfRenderWindow* window, ComponentData* components, int camera, sfText* text, sfVector2f position, char string[100], sfColor color) {
    bool created = false;
    if (!text) {
        text = sfText_create();
    }

    sfText_setFont(text, CameraComponent_get(components, camera)->fonts[0]);
    sfText_setCharacterSize(text, 20);
    sfText_setColor(text, color);

    sfText_setString(text, string);
    sfText_setPosition(text, world_to_screen(components, camera, position));

    sfFloatRect bounds = sfText_getLocalBounds(text);
    sfText_setOrigin(text, (sfVector2f) { bounds.left + 0.5f * bounds.width, bounds.top + 0.5f * bounds.height });

    sfRenderWindow_drawText(window, text, NULL);

    if (created) {
        sfText_destroy(text);
    }
}


void update_camera(ComponentData* components, int camera, float time_step) {
    CoordinateComponent* coord = CoordinateComponent_get(components, camera);
    CameraComponent* cam = CameraComponent_get(components, camera);

    sfVector2f pos = zeros();

    int n = 0;
    for (ListNode* node = components->player.order->head; node; node = node->next) {
        int i = node->value;
        PlayerComponent* player = PlayerComponent_get(components, i);
        if (player->state != PLAYER_DEAD) {
            n += 1;
            pos = sum(pos, get_position(components, i));
        }
    }
    if (n != 0) {
        pos = mult(1.0 / n, pos);
        coord->position = sum(coord->position, mult(10.0 * time_step, diff(pos, coord->position)));
    }

    cam->zoom += 10.0 * time_step * (cam->zoom_target * cam->resolution.y / 720.0 - cam->zoom);
    cam->shake.velocity = diff(cam->shake.velocity, lin_comb(5.0f, cam->shake.position, 0.1f, cam->shake.velocity));
    cam->shake.position = sum(cam->shake.position, mult(time_step, cam->shake.velocity));
}


bool on_screen(ComponentData* components, int camera, sfVector2f position, float width, float height) {
    sfVector2f pos = get_position(components, camera);
    CameraComponent* cam = CameraComponent_get(components, camera);

    if (fabsf(position.x - pos.x) < 0.5f * (width + cam->resolution.x / cam->zoom)) {
        if (fabsf(position.y - pos.y) < 0.5f * (height + cam->resolution.y / cam->zoom)) {
            return true;
        }
    }
    return false;
}


void shake_camera(ComponentData* components, float speed) {
    for (int i = 0; i < components->entities; i++) {
        CameraComponent* cam = CameraComponent_get(components, i);
        if (cam) {
            cam->shake.velocity = sum(cam->shake.velocity, mult(speed, rand_vector()));
        }
    }
}
