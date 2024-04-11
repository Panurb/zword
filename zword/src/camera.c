#define _USE_MATH_DEFINES

#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

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


Vector2f camera_size(int camera) {
    CameraComponent* cam = CameraComponent_get(camera);
    sfVector2i res = cam->resolution;
    float zoom = cam->zoom;
    return (Vector2f) { res.x / zoom, res.y / zoom };
}


sfVector2f world_to_screen(int camera, Vector2f a) {
    CameraComponent* cam = CameraComponent_get(camera);
    Vector2f pos = sum(get_position(camera), cam->shake.position);
    sfVector2f b;
    b.x = (a.x - pos.x) * cam->zoom + 0.5 * cam->resolution.x;
    b.y = (pos.y - a.y) * cam->zoom + 0.5 * cam->resolution.y;
    return b;
}


Vector2f sdl_world_to_screen(int camera, Vector2f a) {
    CameraComponent* cam = CameraComponent_get(camera);
    Vector2f pos = sum(get_position(camera), cam->shake.position);
    Vector2f b;
    b.x = (a.x - pos.x) * cam->zoom + 0.5 * cam->resolution.x;
    b.y = (pos.y - a.y) * cam->zoom + 0.5 * cam->resolution.y;
    return b;
}


Vector2f screen_to_world(int camera, sfVector2i a) {
    CameraComponent* cam = CameraComponent_get(camera);
    Vector2f pos = sum(get_position(camera), cam->shake.position);
    Vector2f b;
    b.x = (a.x - 0.5 * cam->resolution.x) / cam->zoom + pos.x;
    b.y = (0.5 * cam->resolution.y - a.y) / cam->zoom + pos.y;
    return b;
}


sfVector2f world_to_texture(int camera, Vector2f a) {
    CameraComponent* cam = CameraComponent_get(camera);
    Vector2f pos = sum(get_position(camera), cam->shake.position);
    sfVector2f b;
    b.x = (a.x - pos.x) * cam->zoom + 0.5 * cam->resolution.x;
    b.y = (a.y - pos.y) * cam->zoom + 0.5 * cam->resolution.y;
    return b;
}


void draw_triangle_fan(int camera, SDL_Vertex* vertices, int verts_size) {
    int* indices = malloc(3 * verts_size * sizeof(int));
    for (int i = 0; i < verts_size; i++) {
        indices[3 * i] = 0;
        indices[3 * i + 1] = i;
        indices[3 * i + 2] = (i + 1) % verts_size;
    }
    indices[verts_size * 3 - 1] = 1;
    SDL_RenderGeometry(app.renderer, NULL, vertices, verts_size, indices, 3 * verts_size);
    free(indices);
}


void draw_triangle_strip(int camera, SDL_Vertex* vertices, int verts_size) {
    int* indices = malloc(3 * (verts_size - 2) * sizeof(int));
    for (int i = 0; i < verts_size - 2; i++) {
        indices[3 * i] = i;
        indices[3 * i + 1] = i + 1;
        indices[3 * i + 2] = i + 2;
    }
    SDL_RenderGeometry(app.renderer, NULL, vertices, verts_size, indices, 3 * (verts_size - 2));
    free(indices);
}


void draw_line(int camera, sfRectangleShape* line, Vector2f start, Vector2f end, float width, sfColor color) {
    CameraComponent* cam = CameraComponent_get(camera);
    
    bool created = false;
    if (!line) {
        line = sfRectangleShape_create();
        created = true;
    }

    Vector2f r = diff(end, start);
    Vector2f pos = sum(start, mult(0.5f * width / norm(r), perp(r)));
    sfRectangleShape_setPosition(line, world_to_screen(camera, pos));
    sfRectangleShape_setFillColor(line, color);
    sfRectangleShape_setSize(line, (sfVector2f) { dist(start, end) * cam->zoom, width * cam->zoom });
    sfRectangleShape_setRotation(line, to_degrees(-atan2(r.y, r.x)));
    sfRenderWindow_drawRectangleShape(game_window, line, NULL);

    if (created) {
        sfRectangleShape_destroy(line);
    }

    Vector2f corners[4];
    get_rect_corners(sum(start, mult(0.5f, r)), atan2(r.y, r.x), dist(start, end), width, corners);

    SDL_Vertex vertices[4];
    for (int i = 0; i < 4; i++) {
        Vector2f v = sdl_world_to_screen(camera, corners[i]);
        vertices[i].position = (SDL_FPoint) { v.x, v.y };
        vertices[i].color = (SDL_Color) { color.r, color.g, color.b, color.a };
    }

    int indices[6] = { 0, 1, 2, 2, 3, 0 };
    SDL_SetRenderDrawBlendMode(app.renderer, SDL_BLENDMODE_BLEND);
    SDL_RenderGeometry(app.renderer, NULL, vertices, 4, indices, 6);
}


void draw_circle(int camera, sfCircleShape* shape, Vector2f position, float radius, sfColor color) {
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

    Vector2f points[20];
    get_circle_points(position, radius, 20, points);

    SDL_Vertex vertices[20];
    for (int i = 0; i < 20; i++) {
        Vector2f v = sdl_world_to_screen(camera, points[i]);
        vertices[i].position = (SDL_FPoint) { v.x, v.y };
        vertices[i].color = (SDL_Color) { color.r, color.g, color.b, color.a };
    }

    draw_triangle_fan(camera, vertices, 20);
}


void draw_ellipse(int camera, sfCircleShape* shape, Vector2f position, float major, float minor, float angle, sfColor color) {
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

    Vector2f points[20];
    get_ellipse_points(position, major, minor, angle, 20, points);

    SDL_Vertex vertices[20];
    for (int i = 0; i < 20; i++) {
        Vector2f v = sdl_world_to_screen(camera, points[i]);
        vertices[i].position = (SDL_FPoint) { v.x, v.y };
        vertices[i].color = (SDL_Color) { color.r, color.g, color.b, color.a };
    }

    draw_triangle_fan(camera, vertices, 20);
}


void draw_rectangle(int camera, sfRectangleShape* shape, Vector2f position, float width, float height, float angle, sfColor color) {
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

    Vector2f corners[4];
    get_rect_corners(position, angle, width, height, corners);

    SDL_Vertex vertices[4];
    for (int i = 0; i < 4; i++) {
        Vector2f v = sdl_world_to_screen(camera, corners[i]);
        vertices[i].position = (SDL_FPoint) { v.x, v.y };
        vertices[i].color = (SDL_Color) { color.r, color.g, color.b, color.a };
    }

    int indices[6] = { 0, 1, 2, 2, 3, 0 };
    SDL_RenderGeometry(app.renderer, NULL, vertices, 4, indices, 6);
}



void draw_rectangle_outline(int camera, sfRectangleShape* shape, Vector2f position, float width, float height, 
        float angle, float line_width, sfColor color) {
    bool created = false;
    if (!shape) {
        shape = sfRectangleShape_create();
        created = true;
    }

    Vector2f hw = polar_to_cartesian(0.5 * width, angle);
    Vector2f hh = polar_to_cartesian(0.5 * height, angle + 0.5 * M_PI);

    Vector2f corners[4];
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


void draw_cone(int camera, sfConvexShape* shape, int n, Vector2f position, float range, float angle, float spread) {
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
        Vector2f point = sum(position, polar_to_cartesian(range, angle));
        sfConvexShape_setPoint(shape, i, world_to_screen(camera, point));
        angle += spread / (n - 3);
    }

    sfRenderWindow_drawConvexShape(game_window, shape, NULL);

    if (created) {
        sfConvexShape_destroy(shape);
    }
}


void draw_slice(int camera, sfVertexArray* verts, int verts_size, Vector2f position, float min_range, float max_range, 
        float angle, float spread, sfColor color) {
    bool created = false;
    if (!verts) {
        verts = sfVertexArray_create();
        sfVertexArray_setPrimitiveType(verts, sfTriangleStrip);
        sfVertexArray_resize(verts, verts_size);
        created = true;
    }

    SDL_Vertex* vertices = malloc(verts_size * sizeof(SDL_Vertex));

    Vector2f start = polar_to_cartesian(min_range, angle - 0.5 * spread);
    Vector2f end = polar_to_cartesian(max_range, angle - 0.5 * spread);

    Matrix2f rot = rotation_matrix(spread / (verts_size / 2.0f - 1));

    for (int k = 0; k < verts_size / 2; k += 1) {
        sfVertex* v = sfVertexArray_getVertex(verts, 2 * k);
        v->position = world_to_screen(camera, sum(position, start));
        v->color = color;

        v = sfVertexArray_getVertex(verts, 2 * k + 1);
        v->position = world_to_screen(camera, sum(position, end));
        v->color = color;

        Vector2f pos = sdl_world_to_screen(camera, sum(position, start));
        vertices[2 * k].position = (SDL_FPoint) { pos.x, pos.y };
        vertices[2 * k].color = (SDL_Color) { color.r, color.g, color.b, color.a };
        
        pos = sdl_world_to_screen(camera, sum(position, end));
        vertices[2 * k + 1].position = (SDL_FPoint) { pos.x, pos.y };
        vertices[2 * k + 1].color = (SDL_Color) { color.r, color.g, color.b, color.a };

        start = matrix_mult(rot, start);
        end = matrix_mult(rot, end);
    }

    sfRenderWindow_drawVertexArray(game_window, verts, NULL);

    draw_triangle_strip(camera, vertices, verts_size);
    free(vertices);

    if (created) {
        sfVertexArray_destroy(verts);
    }
}


void draw_arc(int camera, sfRectangleShape* shape, Vector2f position, float range, float angle, float spread) {
    int n = 5 * ceil(range * spread);

    Matrix2f rot = rotation_matrix(spread / n);

    Vector2f start = polar_to_cartesian(range, angle - 0.5 * spread);
    Vector2f end = start;

    for (int k = 0; k < n; k++) {
        end = matrix_mult(rot, end);
        draw_line(camera, shape, sum(position, start), sum(position, end), 0.05, sfWhite);
        start = end;
    }
}


void draw_slice_outline(int camera, sfRectangleShape* shape, Vector2f position, float min_range, float max_range, float angle, float spread) {
    Vector2f start = polar_to_cartesian(max_range, angle - 0.5 * spread);
    Vector2f end = mult(min_range / max_range, start);

    draw_line(camera, shape, sum(position, start), sum(position, end), 0.05, sfWhite);

    draw_arc(camera, shape, position, min_range, angle, spread);

    draw_arc(camera, shape, position, max_range, angle, spread);

    start = polar_to_cartesian(min_range, angle + 0.5 * spread);
    end = mult(max_range / min_range, start);    
    
    draw_line(camera, shape, sum(position, start), sum(position, end), 0.05, sfWhite);
}


void draw_sprite(int camera, Filename filename, float width, float height, int offset, Vector2f position, float angle, Vector2f scale, float alpha, int shader_index) {
    CameraComponent* cam = CameraComponent_get(camera);

    sfSprite* sprite = sfSprite_create();
    // TODO: don't get texture index every frame
    int i = get_texture_index(filename);
    if (i == -1) {
        printf("Texture not found: %s\n", filename);
        return;
    }
    sfSprite_setTexture(sprite, game_data->textures[i], sfTrue);

    if (width != 0.0f && height != 0.0f) {
        sfIntRect rect = { 0, 0, width * PIXELS_PER_UNIT, height * PIXELS_PER_UNIT };
        sfSprite_setTextureRect(sprite, rect);
    }

    if (offset != 0) {
        sfIntRect rect = { offset * width * PIXELS_PER_UNIT, 0, width * PIXELS_PER_UNIT, height * PIXELS_PER_UNIT };
        sfSprite_setTextureRect(sprite, rect);
    }

    sfSprite_setPosition(sprite, world_to_screen(camera, position));
    Vector2f r = mult(cam->zoom / PIXELS_PER_UNIT, scale);
    sfSprite_setScale(sprite, (sfVector2f) { r.x, r.y });
    sfSprite_setRotation(sprite, -to_degrees(angle));

    sfFloatRect gb = sfSprite_getLocalBounds(sprite);
    sfVector2f origin = { 0.5 * gb.width, 0.5 * gb.height };
    sfSprite_setOrigin(sprite, origin);

    sfSprite_setColor(sprite, get_color(1.0f, 1.0f, 1.0f, alpha));

    sfShader* shader = cam->shaders[shader_index];

    sfRenderStates state = { sfBlendAlpha, sfTransform_Identity, NULL, shader };
    sfRenderWindow_drawSprite(game_window, sprite, &state);

    sfSprite_destroy(sprite);

    SDL_Texture* texture = resources.textures[i];
    SDL_SetTextureAlphaMod(texture, 255 * alpha);

    SDL_Rect src = { 0, 0, width * PIXELS_PER_UNIT, height * PIXELS_PER_UNIT };
    if (offset != 0) {
        src.x = offset * width * PIXELS_PER_UNIT;
    }

    if (strstr(filename, "tile")) {
        SDL_QueryTexture(texture, NULL, NULL, &src.w, &src.h);
        float tile_width = (float)src.w / PIXELS_PER_UNIT;
        float tile_height = (float)src.h / PIXELS_PER_UNIT;

        SDL_FRect dest;
        dest.w = tile_width * r.x * PIXELS_PER_UNIT;
        dest.h = tile_height * r.y * PIXELS_PER_UNIT;

        Vector2f x = polar_to_cartesian(1.0f, angle);
        Vector2f y = perp(x);
        
        int nx = ceil(width / tile_width);
        int ny = ceil(height / tile_height);
        float current_tile_width = tile_width;
        for (int i = 0; i < nx; i++) {
            if (i == nx - 1) {
                current_tile_width = (width - tile_width * (nx - 1));
                src.w = current_tile_width * PIXELS_PER_UNIT;
                dest.w = src.w * r.x;
            }

            float current_tile_height = tile_height;
            src.h = tile_height * PIXELS_PER_UNIT;
            dest.h = src.h * r.y;
            for (int j = 0; j < ny; j++) {
                if (j == ny - 1) {
                    current_tile_height = (height - tile_height * (ny - 1));
                    src.h = current_tile_height * PIXELS_PER_UNIT;
                    dest.h = src.h * r.y;
                }

                Vector2f p = sum(position, lin_comb(i * tile_width - 0.5f * (width - current_tile_width), x, 
                                                    j * tile_height - 0.5f * (height - current_tile_height), y));
                Vector2f pos = sdl_world_to_screen(camera, p);
                dest.x = pos.x - 0.5f * dest.w;
                dest.y = pos.y - 0.5f * dest.h;
                SDL_RenderCopyExF(app.renderer, texture, &src, &dest, -to_degrees(angle), NULL, SDL_FLIP_NONE);
            }
        }
    } else {
        if (width == 0.0f || height == 0.0f) {
            SDL_QueryTexture(texture, NULL, NULL, &src.w, &src.h);
            width = (float)src.w / PIXELS_PER_UNIT;
            height = (float)src.h / PIXELS_PER_UNIT;
        }

        Vector2f pos = sdl_world_to_screen(camera, position);

        SDL_FRect dest;
        dest.w = width * r.x * PIXELS_PER_UNIT;
        dest.h = height * r.y * PIXELS_PER_UNIT;
        dest.x = pos.x - 0.5f * dest.w;
        dest.y = pos.y - 0.5f * dest.h;

        SDL_Rect* psrc = &src;
        if (width == 0.0f || height == 0.0f) {
            SDL_Rect* psrc = NULL;
        }
        SDL_RenderCopyExF(app.renderer, texture, psrc, &dest, -to_degrees(angle), NULL, SDL_FLIP_NONE);
    }
}


void draw_text(int camera, sfText* text, Vector2f position, char string[100], int size, sfColor color) {
    bool created = false;
    if (!text) {
        text = sfText_create();
        created = true;
    }

    if (color.a == 0) {
        return;
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

    if (string[0] == '\0') {
        return;
    }

    int font_size = size * cam->zoom / 40.0f;
    TTF_Font* font = resources.fonts[font_size];
    if (!font) {
        return;
    }

    Vector2f pos = sdl_world_to_screen(camera, position);
    SDL_Color c = { color.r, color.g, color.b, color.a };
    SDL_Surface* surface = TTF_RenderText_Blended(font, string, c);
    SDL_Texture* texture = SDL_CreateTextureFromSurface(app.renderer, surface);
    SDL_FRect dest = { pos.x - 0.5f * surface->w, pos.y - 0.5f * surface->h, surface->w, surface->h };
    SDL_RenderCopyF(app.renderer, texture, NULL, &dest);
    SDL_FreeSurface(surface);
    SDL_DestroyTexture(texture);
}


void update_camera(int camera, float time_step, bool follow_players) {
    CoordinateComponent* coord = CoordinateComponent_get(camera);
    CameraComponent* cam = CameraComponent_get(camera);

    Vector2f pos = zeros();

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


bool on_screen(int camera, Vector2f position, float width, float height) {
    Vector2f pos = get_position(camera);
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
    Vector2f pos = get_position(camera);
    sfColor color = get_color(0.0f, 0.0f, 0.0f, alpha);
    draw_rectangle(camera, NULL, pos, width, height, 0.0f, color);
}
