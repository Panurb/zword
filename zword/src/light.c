#define _USE_MATH_DEFINES

#include <stdlib.h>
#include <math.h>

#include "app.h"
#include "component.h"
#include "camera.h"
#include "collider.h"
#include "util.h"
#include "light.h"
#include "grid.h"
#include "raycast.h"
#include "image.h"
#include "game.h"


void update_lights(float delta_time) {
    for (int i = 0; i < game_data->components->entities; i++) {
        LightComponent* light = LightComponent_get(i);

        if (!light) continue;

        CoordinateComponent* coord = CoordinateComponent_get(i);
        if (coord->parent != -1) {
            VehicleComponent* vehicle = VehicleComponent_get(coord->parent);
            if (vehicle) {
                light->enabled = (vehicle->riders[0] != -1);
            }
        }

        if (light->enabled) {
            light->brightness = fminf(light->max_brightness, light->brightness + light->speed * delta_time);
        } else {
            light->brightness = fmaxf(0.0, light->brightness - light->speed * delta_time);
        }

        light->time = fmodf(light->time + delta_time, 400.0f * M_PI);
    }
}


void draw_shine(int camera, int entity, HitInfo info, Vector2f velocity) {
    ImageComponent* image = ImageComponent_get(info.entity);
    if (!image) return;

    LightComponent* light = LightComponent_get(entity);

    if (image->shine > 0.0) {
        float vn = dot(velocity, info.normal);
        if (vn < -0.98) {
            Color color = COLOR_WHITE;
            color.a = (1 - 50 * (1 + vn)) * 128;
            float radius = 0.05 * (1 - 5 * (1 + vn)) * CameraComponent_get(camera)->zoom;
            draw_circle(camera, info.position, radius, color);
        }
    }
}


void draw_shadow_circle(int camera, Vector2f position , float radius) {
    Vector2f points[20];
    get_circle_points(position, radius, 20, points);

    SDL_Vertex vertices[20];
    for (int i = 0; i < 20; i++) {
        Vector2f v = world_to_screen(camera, points[i]);
        vertices[i].position = (SDL_FPoint) { v.x, v.y };
        vertices[i].color = (SDL_Color) { 0, 0, 0, 0 };
    }
    vertices[0].color.a = 128;

    int indices[60];
    for (int i = 0; i < 20; i++) {
        indices[3 * i] = 0;
        indices[3 * i + 1] = i;
        indices[3 * i + 2] = (i + 1) % 20;
    }
    indices[59] = 1;
    SDL_RenderGeometry(app.renderer, NULL, vertices, 20, indices, 60);
}


void draw_shadow_rectangle(int camera, Vector2f position, float width, float height, float angle) {
    // 7 ----------- 4
    // |\           /|
    // |  3 ----- 0  |
    // |  |       |  |
    // |  2 ----- 1  |
    // |/           \|
    // 6 ----------- 5

    Vector2f corners[8];
    get_rect_corners(position, angle, width, height, corners);

    float radius = sqrtf(width * width + height * height);
    float r = fminf(0.5f * radius, 1.0f);
    for (int j = 0; j < 4; j++) {
        Vector2f corner = corners[j];
        corners[j + 4] = sum(corner, polar_to_cartesian(r, angle + (0.25f - j * 0.5f) * M_PI));
    }

    SDL_Vertex vertices[8];
    for (int i = 0; i < 8; i++) {
        Vector2f v = world_to_screen(camera, corners[i]);
        vertices[i].position = (SDL_FPoint) { v.x, v.y };
        int alpha = i < 4 ? 64 : 0;
        vertices[i].color = (SDL_Color) { 0, 0, 0, alpha };
    }

    // Triangle strip
    int order[10] = { 3, 7, 2, 6, 1, 5, 0, 4, 3, 7 };
    int indices[30];
    for (int i = 0; i < 8; i++) {
        indices[3 * i] = order[i];
        indices[3 * i + 1] = order[i + 1];
        indices[3 * i + 2] = order[i + 2];
    }
    indices[24] = 0;
    indices[25] = 1;
    indices[26] = 2;
    indices[27] = 0;
    indices[28] = 2;
    indices[29] = 3;

    SDL_RenderGeometry(app.renderer, NULL, vertices, 8, indices, 30);
}


void draw_shadows(int camera) {
    SDL_SetRenderTarget(app.renderer, app.shadow_texture);
    SDL_SetRenderDrawColor(app.renderer, 0, 0, 0, 0);
    SDL_RenderClear(app.renderer);

    if (CameraComponent_get(camera)->zoom < 10.0f) return;

    for (int i = 0; i < game_data->components->entities; i++) {
        ColliderComponent* collider = ColliderComponent_get(i);
        if (!collider) continue;

        ImageComponent* image = ImageComponent_get(i);
        if (!image || image->alpha == 0.0f || image->layer <= LAYER_DECALS) continue;

        Vector2f start = get_position(i);

        float radius = 1.5f * collider->radius;
        if (!on_screen(camera, start, 2.0f * radius, 2.0f * radius)) {
            continue;
        }
        
        switch (collider->type) {
            case COLLIDER_CIRCLE: {
                draw_shadow_circle(camera, start, radius);
                break;
            } case COLLIDER_RECTANGLE: {
                float angle = get_angle(i);
                draw_shadow_rectangle(camera, start, collider->width, collider->height, angle);
                break;
            }
        }
    }

    SDL_SetRenderTarget(app.renderer, NULL);
}


void draw_lights(int camera, float ambient_light) {
    Color color = get_color(ambient_light, ambient_light, ambient_light, 1.0f);

    SDL_SetRenderTarget(app.renderer, app.light_texture);
    SDL_SetRenderDrawColor(app.renderer, color.r, color.g, color.b, color.a);
    SDL_RenderClear(app.renderer);
    SDL_SetRenderDrawBlendMode(app.renderer, SDL_BLENDMODE_ADD);

    if (CameraComponent_get(camera)->zoom < 10.0f) return;

    for (int i = 0; i < game_data->components->entities; i++) {
        LightComponent* light = LightComponent_get(i);
        if (!light) continue;

        Vector2f start = get_position(i);

        if (!on_screen(camera, start, 2.0f * light->range, 2.0f * light->range)) {
            continue;
        }

        float brightness = light->brightness;
        float range = light->range;

        if (light->enabled) {
            float f = 1.0 - light->flicker * 0.25 * (sinf(8.0 * light->time) + sinf(12.345 * light->time) + 2.0);
            brightness *= f;
            range *= f;
        }

        Color color = light->color;
        color.a = 255 * brightness;

        SDL_Vertex* vertices = malloc((light->rays + 1) * sizeof(SDL_Vertex));
        Vector2f pos = world_to_screen(camera, start);
        vertices[0] = (SDL_Vertex) { pos.x, pos.y, color.r, color.g, color.b, color.a };

        float angle = get_angle(i) - 0.5 * light->angle;
        Vector2f velocity = polar_to_cartesian(1.0, angle);

        float delta_angle = light->angle / (light->rays - 1);
        Matrix2f rot = rotation_matrix(delta_angle);

        for (int j = 1; j < light->rays + 1; j++) {
            HitInfo info = raycast(start, velocity, range, GROUP_LIGHTS);
            Vector2f end = info.position;

            end = sum(end, mult(0.25, velocity));

            color.a = 255 * brightness * (1.0 - dist(start, end) / (range + 0.25));

            pos = world_to_screen(camera, end);
            vertices[j] = (SDL_Vertex) { pos.x, pos.y, color.r, color.g, color.b, color.a };

            velocity = matrix_mult(rot, velocity);
        }

        int* indices = malloc(3 * (light->rays + 1) * sizeof(int));
        for (int j = 0; j < light->rays + 1; j++) {
            indices[3 * j] = 0;
            indices[3 * j + 1] = j;
            indices[3 * j + 2] = (j + 1)  % (light->rays + 1);
        }

        SDL_RenderGeometry(app.renderer, NULL, vertices, light->rays + 1, indices, 3 * (light->rays + 1));

        free(vertices);
        free(indices);
    }

    SDL_SetRenderTarget(app.renderer, NULL);
}
