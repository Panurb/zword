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

        float radius = 1.5f * collider_radius(i);
        if (!on_screen(camera, start, 2.0f * radius, 2.0f * radius)) {
            continue;
        }
        
        switch (collider->type) {
            case COLLIDER_CIRCLE: {
                draw_shadow_circle(camera, start, collider_radius(i));
                break;
            } case COLLIDER_RECTANGLE: {
                float angle = get_angle(i);
                draw_shadow_rectangle(camera, start, collider_width(i), collider_height(i), angle);
                break;
            }
        }
    }

    SDL_SetRenderTarget(app.renderer, NULL);
}


typedef enum {
    SIDE_LEFT = -1,
    SIDE_NONE,
    SIDE_RIGHT
} Side;


typedef struct {
    Vector2f position;
    float angle;
    Side side;
    int entity;
} Edge;


int compare_angle(const void* a, const void* b) {
    Vector2f* va = (Vector2f*) a;
    Vector2f* vb = (Vector2f*) b;

    float angle_a = polar_angle(*va);
    float angle_b = polar_angle(*vb);

    if (angle_a < angle_b) {
        return -1;
    } else if (angle_a > angle_b) {
        return 1;
    } else {
        return 0;
    }
}


int compare_edge_angle(const void* a, const void* b) {
    Edge* va = (Edge*) a;
    Edge* vb = (Edge*) b;

    if (va->angle < vb->angle) {
        return -1;
    } else if (va->angle > vb->angle) {
        return 1;
    } else {
        return 0;
    }
}


void draw_light(int camera, Vector2f start, float angle, float light_angle, float range, int rays, Color color, 
        float brightness, int bounces) {
    Edge points[1000];
    int points_size = 0;
    float delta = 0.01f;

    List* entities = get_entities(start, range);

    ListNode* node;
    FOREACH(node, entities) {
        int entity = node->value;

        Vector2f pos = get_position(entity);
        float distance = dist(pos, start);
        if (distance == 0.0f) continue;

        ColliderComponent* collider = ColliderComponent_get(entity);

        if (collider->type == COLLIDER_CIRCLE) {
            float radius = collider_radius(entity);
            Vector2f dir = diff(pos, start);
            float a = polar_angle(dir);

            float angular_radius = asinf(radius / distance);
            float x = sqrtf(distance * distance - radius * radius);

            float left = a + angular_radius;
            float right = a - angular_radius;

            HitInfo info = raycast(start, polar_to_cartesian(1.0f, left - delta), range, GROUP_LIGHTS);
            if (info.entity == entity) {
                points[points_size].angle = left - delta;
                points[points_size].position = info.position;
                points[points_size].side = SIDE_LEFT;
                points[points_size].entity = info.entity;
                points_size++;

                info = raycast(start, polar_to_cartesian(1.0f, left + delta), range, GROUP_LIGHTS);
                points[points_size].angle = left + delta;
                points[points_size].position = info.position;
                points[points_size].side = SIDE_NONE;
                points[points_size].entity = info.entity;
                points_size++;
            }

            info = raycast(start, polar_to_cartesian(1.0f, right + delta), range, GROUP_LIGHTS);
            if (info.entity == entity) {
                points[points_size].angle = right + delta;
                points[points_size].position = info.position;
                points[points_size].side = SIDE_RIGHT;
                points[points_size].entity = info.entity;
                points_size++;

                info = raycast(start, polar_to_cartesian(1.0f, right - delta), range, GROUP_LIGHTS);
                points[points_size].angle = right - delta;
                points[points_size].position = info.position;
                points[points_size].side = SIDE_NONE;
                points[points_size].entity = info.entity;
                points_size++;
            }
        } else if (collider->type == COLLIDER_RECTANGLE) {
            Vector2f corners[4];
            get_rect_corners(pos, get_angle(entity), collider_width(entity), collider_height(entity), corners);

            for (int j = 0; j < 4; j++) {
                corners[j] = diff(corners[j], start);
            }

            qsort(corners, 4, sizeof(Vector2f), compare_angle);

            float right = polar_angle(corners[0]);
            float left = polar_angle(corners[3]);

            HitInfo info = raycast(start, polar_to_cartesian(1.0f, left - delta), range, GROUP_LIGHTS);
            if (info.entity == entity) {
                points[points_size].angle = left - delta;
                points[points_size].position = info.position;
                points[points_size].side = SIDE_LEFT;
                points[points_size].entity = info.entity;
                points_size++;

                info = raycast(start, polar_to_cartesian(1.0f, left + delta), range, GROUP_LIGHTS);
                points[points_size].angle = left + delta;
                points[points_size].position = info.position;
                points[points_size].side = SIDE_NONE;
                points[points_size].entity = info.entity;
                points_size++;
            }

            info = raycast(start, polar_to_cartesian(1.0f, right + delta), range, GROUP_LIGHTS);
            if (info.entity == entity) {
                points[points_size].angle = right + delta;
                points[points_size].position = info.position;
                points[points_size].side = SIDE_RIGHT;
                points[points_size].entity = info.entity;
                points_size++;

                info = raycast(start, polar_to_cartesian(1.0f, right - delta), range, GROUP_LIGHTS);
                points[points_size].angle = right - delta;
                points[points_size].position = info.position;
                points[points_size].side = SIDE_NONE;
                points[points_size].entity = info.entity;
                points_size++;
            }

            float distances[4];
            for (int j = 0; j < 4; j++) {
                distances[j] = norm2(corners[j]);
            }
            int nearest = argmin(distances, 4);

            if (nearest != 0 && nearest != 3) {
                info = raycast(start, corners[nearest], range, GROUP_LIGHTS);
                if (info.entity == entity) {
                    points[points_size].angle = polar_angle(corners[nearest]);
                    points[points_size].position = info.position;
                    points[points_size].side = SIDE_NONE;
                    points[points_size].entity = info.entity;
                    points_size++;
                }
            }
        }
    }

    free(entities);

    for (int i = 0; i < points_size; i++) {
        points[i].position = diff(points[i].position, start);
    }

    qsort(points, points_size, sizeof(Edge), compare_edge_angle);

    for (int i = 0; i < points_size; i++) {
        if (points[i].side == SIDE_RIGHT) {
            draw_circle(camera, sum(start, points[i].position), 0.1f, COLOR_RED);
        } else if (points[i].side == SIDE_LEFT) {
            draw_circle(camera, sum(start, points[i].position), 0.1f, COLOR_GREEN);
        } else {
            draw_circle(camera, sum(start, points[i].position), 0.1f, COLOR_BLUE);
        }
        String text;
        sprintf(text, "%d", i);
        draw_text(camera, sum(sum(start, points[i].position), vec(0, 0.5)), text, 20, COLOR_RED);
    }

    int min_points = 50;
    SDL_Vertex* vertices = malloc((points_size + 1 + min_points) * sizeof(SDL_Vertex));
    Vector2f pos = world_to_screen(camera, start);

    color.a = 255 * brightness;
    vertices[0] = (SDL_Vertex) { pos.x, pos.y, color.r, color.g, color.b, color.a };

    int side_counter = 0;
    for (int i = 0; i < points_size; i++) {
        side_counter -= points[i].side;
        if (points[i].entity == -1) {
            LOG_DEBUG("Index: %d, Angle: %f, Side: %d", i, points[i].angle, points[i].side);
            break;
        }
    }

    int vertices_size = 1;
    for (int i = 0; i < points_size; i++) {
        side_counter += points[i].side;

        String text;
        sprintf(text, "%d", side_counter);
        draw_text(camera, sum(sum(start, points[i].position), vec(0, -0.5)), text, 20, COLOR_RED);

        Vector2f end = sum(start, points[i].position);

        // end = sum(end, mult(0.25, diff(end, start)));

        color.a = 255 * brightness * (1.0 - dist(start, end) / (range + 0.25));

        pos = world_to_screen(camera, end);
        vertices[vertices_size] = (SDL_Vertex) { pos.x, pos.y, color.r, color.g, color.b, color.a };
        vertices_size++;

        float angle_diff = points[(i + 1) % points_size].angle - points[i].angle;

        if (points[i].entity == -1 && points[(i + 1) % points_size].entity == -1 && angle_diff > light_angle / min_points) {
            int n = ceilf(angle_diff / (light_angle / min_points));
            for (int j = 0; j < n; j++) {
                float a = points[i].angle + j * angle_diff / n;
                Vector2f end = polar_to_cartesian(range, a);
                pos = world_to_screen(camera, sum(start, end));
                vertices[vertices_size] = (SDL_Vertex) { pos.x, pos.y, color.r, color.g, color.b, 0 };
                vertices_size++;

                draw_circle(camera, sum(start, end), 0.1f, COLOR_YELLOW);
            }
        }
    }

    vertices[vertices_size] = vertices[1];
    vertices_size++;

    int* indices = malloc(3 * vertices_size * sizeof(int));
    for (int j = 0; j < vertices_size; j++) {
        indices[3 * j] = 0;
        indices[3 * j + 1] = j;
        indices[3 * j + 2] = (j + 1)  % vertices_size;
    }

    SDL_RenderGeometry(app.renderer, NULL, vertices, vertices_size, indices, 3 * vertices_size);

    free(vertices);
    free(indices);
}


void draw_laser(int camera, Vector2f start, float angle, float range, Color color, float brightness) {
    HitInfo info = raycast(start, polar_to_cartesian(1.0, angle), range, GROUP_LIGHTS);
    Vector2f end = info.position;

    draw_line(camera, start, end, 0.1f, color);
}


void draw_lights(int camera, float ambient_light) {
    Color color = get_color(ambient_light, ambient_light, ambient_light, 1.0f);

    SDL_SetRenderTarget(app.renderer, app.light_texture);
    SDL_SetRenderDrawColor(app.renderer, color.r, color.g, color.b, color.a);
    SDL_RenderClear(app.renderer);
    SDL_SetRenderDrawBlendMode(app.renderer, SDL_BLENDMODE_BLEND);

    if (CameraComponent_get(camera)->zoom < 10.0f) return;

    for (int i = 0; i < game_data->components->entities; i++) {
        LightComponent* light = LightComponent_get(i);
        if (!light) continue;

        if (light->brightness == 0.0f) {
            continue;
        }

        Vector2f start = get_position(i);

        if (!on_screen(camera, start, 2.0f * light->range, 2.0f * light->range)) {
            continue;
        }

        float brightness = light->brightness;
        float range = light->range;

        float f = 1.0 - light->flicker * 0.25 * (sinf(8.0 * light->time) + sinf(12.345 * light->time) + 2.0);
        brightness *= f;
        range *= f;

        Color color = light->color;
        if (light->angle == 0.0f) {
            draw_laser(camera, start, get_angle(i), range, color, brightness);
        } else {
            draw_light(camera, start, get_angle(i), light->angle, range, light->rays, color, brightness, light->bounces);
        }
    }

    SDL_SetRenderTarget(app.renderer, NULL);
}
