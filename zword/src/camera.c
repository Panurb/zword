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
#include "game.h"


int create_camera(sfVideoMode mode) {
    int i = create_entity();
    CoordinateComponent_add(i, zeros(), 0.0);
    CameraComponent_add(i, (sfVector2i) { mode.width, mode.height }, 40.0f);
    // ParticleComponent* part = ParticleComponent_add(i, 0.0, 2 * M_PI, 0.1, 0.1, 1.0, 1.0, sfWhite, sfWhite);
    // part->enabled = true;
    // part->loop = true;
    return i;
}


int create_menu_camera(sfVideoMode mode) {
    int i = create_entity();
    CoordinateComponent_add(i, zeros(), 0.0);
    CameraComponent_add(i, (sfVector2i) { mode.width, mode.height }, 25.0f);
    return i;
}


sfVector2f camera_size(int camera) {
    CameraComponent* cam = CameraComponent_get(camera);
    sfVector2i res = cam->resolution;
    float zoom = cam->zoom;
    return (sfVector2f) { res.x / zoom, res.y / zoom };
}


sfVector2f world_to_screen(int camera, sfVector2f a) {
    CameraComponent* cam = CameraComponent_get(camera);
    sfVector2f pos = sum(get_position(camera), cam->shake.position);
    sfVector2f b;
    b.x = (a.x - pos.x) * cam->zoom + 0.5 * cam->resolution.x;
    b.y = (pos.y - a.y) * cam->zoom + 0.5 * cam->resolution.y;
    return b;
}


sfVector2f screen_to_world(int camera, sfVector2i a) {
    CameraComponent* cam = CameraComponent_get(camera);
    sfVector2f pos = sum(get_position(camera), cam->shake.position);
    sfVector2f b;
    b.x = (a.x - 0.5 * cam->resolution.x) / cam->zoom + pos.x;
    b.y = (0.5 * cam->resolution.y - a.y) / cam->zoom + pos.y;
    return b;
}


sfVector2f world_to_texture(int camera, sfVector2f a) {
    CameraComponent* cam = CameraComponent_get(camera);
    sfVector2f pos = sum(get_position(camera), cam->shake.position);
    sfVector2f b;
    b.x = (a.x - pos.x) * cam->zoom + 0.5 * cam->resolution.x;
    b.y = (a.y - pos.y) * cam->zoom + 0.5 * cam->resolution.y;
    return b;
}


void draw_line(int camera, sfRectangleShape* line, sfVector2f start, sfVector2f end, float width, sfColor color) {
    CameraComponent* cam = CameraComponent_get(camera);
    
    bool created = false;
    if (!line) {
        line = sfRectangleShape_create();
        created = true;
    }

    sfVector2f r = diff(end, start);
    sfVector2f pos = sum(start, mult(0.5f * width / norm(r), perp(r)));
    sfRectangleShape_setPosition(line, world_to_screen(camera, pos));
    sfRectangleShape_setFillColor(line, color);
    sfRectangleShape_setSize(line, (sfVector2f) { dist(start, end) * cam->zoom, width * cam->zoom });
    sfRectangleShape_setRotation(line, to_degrees(-atan2(r.y, r.x)));
    sfRenderWindow_drawRectangleShape(game_window, line, NULL);

    if (created) {
        sfRectangleShape_destroy(line);
    }
}


void draw_circle(int camera, sfCircleShape* shape, sfVector2f position, float radius, sfColor color) {
    CameraComponent* cam = CameraComponent_get(camera);

    bool created = false;
    if (!shape) {
        shape = sfCircleShape_create();
        created = true;
    }

    sfCircleShape_setOrigin(shape, (sfVector2f) { radius * cam->zoom, radius * cam->zoom });
    sfCircleShape_setPosition(shape, world_to_screen(camera, position));
    sfCircleShape_setRadius(shape, radius * cam->zoom);
    sfCircleShape_setFillColor(shape, color);
    sfRenderWindow_drawCircleShape(game_window, shape, NULL);

    if (created) {
        sfCircleShape_destroy(shape);
    }
}


void draw_ellipse(int camera, sfCircleShape* shape, sfVector2f position, float major, float minor, float angle, sfColor color) {
    CameraComponent* cam = CameraComponent_get(camera);

    bool created = false;
    if (!shape) {
        shape = sfCircleShape_create();
        created = true;
    }

    sfCircleShape_setOrigin(shape, (sfVector2f) { major * cam->zoom, major * cam->zoom });
    sfCircleShape_setPosition(shape, world_to_screen(camera, position));
    sfCircleShape_setRadius(shape, major * cam->zoom);
    sfCircleShape_setScale(shape, (sfVector2f) { 1.0, minor / major });
    sfCircleShape_setFillColor(shape, color);
    sfCircleShape_setRotation(shape, -to_degrees(angle));
    sfRenderWindow_drawCircleShape(game_window, shape, NULL);

    if (created) {
        sfCircleShape_destroy(shape);
    }
}


void draw_rectangle(int camera, sfRectangleShape* shape, sfVector2f position, float width, float height, float angle, sfColor color) {
    CameraComponent* cam = CameraComponent_get(camera);

    bool created = false;
    if (!shape) {
        shape = sfRectangleShape_create();
        created = true;
    }

    sfRectangleShape_setOrigin(shape, (sfVector2f) { 0.5 * width * cam->zoom, 0.5 * height * cam->zoom });
    sfRectangleShape_setPosition(shape, world_to_screen(camera, position));
    sfVector2f size = { width * cam->zoom, height * cam->zoom };
    sfRectangleShape_setSize(shape, size);
    sfRectangleShape_setRotation(shape, -to_degrees(angle));
    sfRectangleShape_setFillColor(shape, color);
    sfRenderWindow_drawRectangleShape(game_window, shape, NULL);

    if (created) {
        sfRectangleShape_destroy(shape);
    }
}



void draw_rectangle_outline(int camera, sfRectangleShape* shape, sfVector2f position, float width, float height, 
        float angle, float line_width, sfColor color) {
    bool created = false;
    if (!shape) {
        shape = sfRectangleShape_create();
        created = true;
    }

    sfVector2f hw = polar_to_cartesian(0.5 * width, angle);
    sfVector2f hh = polar_to_cartesian(0.5 * height, angle + 0.5 * M_PI);

    sfVector2f corners[4];
    corners[0] = sum(position, sum(hw, hh));
    corners[1] = diff(corners[0], mult(2, hh));
    corners[2] = diff(corners[1], mult(2, hw));
    corners[3] = sum(corners[2], mult(2, hh));

    for (int i = 0; i < 4; i++) {
        draw_line(camera, shape, corners[i], corners[(i + 1) % 4], line_width, color);
    }

    if (created) {
        sfRectangleShape_destroy(shape);
    }
}


void draw_cone(int camera, sfConvexShape* shape, int n, sfVector2f position, float range, float angle, float spread) {
    bool created = false;
    if (!shape) {
        shape = sfConvexShape_create();
        sfConvexShape_setPointCount(shape, n);
    }
    
    sfVector2f start = world_to_screen(camera, position);
    sfConvexShape_setPoint(shape, 0, start);
    sfConvexShape_setPoint(shape, n - 1, start);

    angle -= 0.5 * spread;

    for (int i = 1; i < n - 1; i++) {
        sfVector2f point = sum(position, polar_to_cartesian(range, angle));
        sfConvexShape_setPoint(shape, i, world_to_screen(camera, point));
        angle += spread / (n - 3);
    }

    sfRenderWindow_drawConvexShape(game_window, shape, NULL);

    if (created) {
        sfConvexShape_destroy(shape);
    }
}


void draw_slice(int camera, sfVertexArray* verts, int verts_size, sfVector2f position, float min_range, float max_range, 
        float angle, float spread, sfColor color) {
    bool created = false;
    if (!verts) {
        verts = sfVertexArray_create();
        sfVertexArray_setPrimitiveType(verts, sfTriangleStrip);
        sfVertexArray_resize(verts, verts_size);
        created = true;
    }

    sfVector2f start = polar_to_cartesian(min_range, angle - 0.5 * spread);
    sfVector2f end = polar_to_cartesian(max_range, angle - 0.5 * spread);

    Matrix2f rot = rotation_matrix(spread / (verts_size / 2.0f - 1));

    for (int k = 0; k < verts_size / 2; k += 1) {
        sfVertex* v = sfVertexArray_getVertex(verts, 2 * k);
        v->position = world_to_screen(camera, sum(position, start));
        v->color = color;

        v = sfVertexArray_getVertex(verts, 2 * k + 1);
        v->position = world_to_screen(camera, sum(position, end));
        v->color = color;

        start = matrix_mult(rot, start);
        end = matrix_mult(rot, end);
    }

    sfRenderWindow_drawVertexArray(game_window, verts, NULL);

    if (created) {
        sfVertexArray_destroy(verts);
    }
}


void draw_arc(int camera, sfRectangleShape* shape, sfVector2f position, float range, float angle, float spread) {
    int n = 5 * ceil(range * spread);

    Matrix2f rot = rotation_matrix(spread / n);

    sfVector2f start = polar_to_cartesian(range, angle - 0.5 * spread);
    sfVector2f end = start;

    for (int k = 0; k < n; k++) {
        end = matrix_mult(rot, end);
        draw_line(camera, shape, sum(position, start), sum(position, end), 0.05, sfWhite);
        start = end;
    }
}


void draw_slice_outline(int camera, sfRectangleShape* shape, sfVector2f position, float min_range, float max_range, float angle, float spread) {
    sfVector2f start = polar_to_cartesian(max_range, angle - 0.5 * spread);
    sfVector2f end = mult(min_range / max_range, start);

    draw_line(camera, shape, sum(position, start), sum(position, end), 0.05, sfWhite);

    draw_arc(camera, shape, position, min_range, angle, spread);

    draw_arc(camera, shape, position, max_range, angle, spread);

    start = polar_to_cartesian(min_range, angle + 0.5 * spread);
    end = mult(max_range / min_range, start);    
    
    draw_line(camera, shape, sum(position, start), sum(position, end), 0.05, sfWhite);
}


void draw_sprite(int camera, sfSprite* sprite, sfVector2f position, float angle, sfVector2f scale, int shader_index) {
    CameraComponent* cam = CameraComponent_get(camera);

    sfSprite_setPosition(sprite, world_to_screen(camera, position));
    sfSprite_setScale(sprite, mult(cam->zoom / PIXELS_PER_UNIT, scale));
    sfSprite_setRotation(sprite, -to_degrees(angle));

    sfFloatRect gb = sfSprite_getLocalBounds(sprite);
    sfVector2f origin = { 0.5 * gb.width, 0.5 * gb.height };
    sfSprite_setOrigin(sprite, origin);

    sfShader* shader = cam->shaders[shader_index];

    sfRenderStates state = { sfBlendAlpha, sfTransform_Identity, NULL, shader };
    sfRenderWindow_drawSprite(game_window, sprite, &state);
}


void draw_text(int camera, sfText* text, sfVector2f position, char string[100], int size, sfColor color) {
    bool created = false;
    if (!text) {
        text = sfText_create();
        created = true;
    }

    CameraComponent* cam = CameraComponent_get(camera);
    sfText_setFont(text, cam->fonts[0]);
    sfText_setCharacterSize(text, size * cam->zoom / 40.0f);
    sfText_setColor(text, color);

    sfText_setString(text, string);
    sfText_setPosition(text, world_to_screen(camera, position));

    sfFloatRect bounds = sfText_getLocalBounds(text);
    sfText_setOrigin(text, (sfVector2f) { bounds.left + 0.5f * bounds.width, bounds.top + 0.5f * bounds.height });

    sfRenderWindow_drawText(game_window, text, NULL);

    if (created) {
        sfText_destroy(text);
    }
}


void update_camera(int camera, float time_step, bool follow_players) {
    CoordinateComponent* coord = CoordinateComponent_get(camera);
    CameraComponent* cam = CameraComponent_get(camera);

    sfVector2f pos = zeros();

    if (follow_players) {
        int n = 0;
        ListNode* node;
        FOREACH (node, game_data->components->player.order) {
            int i = node->value;
            PlayerComponent* player = PlayerComponent_get(i);
            if (player->state != PLAYER_DEAD) {
                n += 1;
                pos = sum(pos, get_position(i));
            }
        }
        if (n != 0) {
            pos = mult(1.0 / n, pos);
            coord->position = sum(coord->position, mult(10.0 * time_step, diff(pos, coord->position)));
        }
    }

    cam->zoom += 10.0 * time_step * (cam->zoom_target * cam->resolution.y / 720.0 - cam->zoom);
    cam->shake.velocity = diff(cam->shake.velocity, lin_comb(5.0f, cam->shake.position, 0.1f, cam->shake.velocity));
    cam->shake.position = sum(cam->shake.position, mult(time_step, cam->shake.velocity));
}


bool on_screen(int camera, sfVector2f position, float width, float height) {
    sfVector2f pos = get_position(camera);
    CameraComponent* cam = CameraComponent_get(camera);

    if (fabsf(position.x - pos.x) < 0.5f * (width + cam->resolution.x / cam->zoom)) {
        if (fabsf(position.y - pos.y) < 0.5f * (height + cam->resolution.y / cam->zoom)) {
            return true;
        }
    }
    return false;
}


void shake_camera(float speed) {
    for (int i = 0; i < game_data->components->entities; i++) {
        CameraComponent* cam = CameraComponent_get(i);
        if (cam) {
            cam->shake.velocity = sum(cam->shake.velocity, mult(speed, rand_vector()));
        }
    }
}


void draw_overlay(int camera, float alpha) {
    CameraComponent* cam = CameraComponent_get(camera);
    float width = cam->resolution.x / cam->zoom;
    float height = cam->resolution.y / cam->zoom;
    sfVector2f pos = get_position(camera);
    sfColor color = get_color(0.0f, 0.0f, 0.0f, alpha);
    draw_rectangle(camera, NULL, pos, width, height, 0.0f, color);
}
