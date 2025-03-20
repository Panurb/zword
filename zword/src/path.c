#include <stdio.h>

#include "path.h"
#include "game.h"
#include "serialize.h"


Vector2f perlin_grad(Vector2f position, Permutation perm) {
    float dx = 0.1;
    float dy = 0.1;
    float x1 = perlin(position.x - dx, position.y, 0.0, perm, -1);
    float x2 = perlin(position.x + dx, position.y, 0.0, perm, -1);
    float y1 = perlin(position.x, position.y - dy, 0.0, perm, -1);
    float y2 = perlin(position.x, position.y + dy, 0.0, perm, -1);
    Vector2f grad = { (x2 - x1) / dx, (y2 - y1) / dy };

    return grad;
}


void create_path(Vector2f start, Vector2f end, String filename, String end_filename, float width) {
    Vector2f dir = normalized(diff(end, start));
    float length = dist(start, end);

    int n = length / 5.0f;

    float angle = polar_angle(dir);

    Entity previous = NULL_ENTITY;
    for (int i = 0; i < n; i++) {
        float x = i / (float)(n - 1);
        Vector2f pos = lin_comb(1.0f - x, start, x, end);

        String filepath;
        if (i == 0 || i == n - 2) {
            snprintf(filepath, sizeof(filepath), "paths/%s", end_filename);
        } else {
            snprintf(filepath, sizeof(filepath), "paths/%s", filename);
        }

        Entity current = load_prefab(filepath, pos, angle, ones());
        PathComponent* path = PathComponent_get(current);
        if (previous != NULL_ENTITY) {
            path->prev = previous;
            PathComponent_get(previous)->next = current;
        }
        previous = current;
    }
}


void create_road(Vector2f start, Vector2f end) {
    create_path(start, end, "road", "road_end", 1.0f);
}


void create_river(Vector2f start, Vector2f end) {
    create_path(start, end, "river", "river_end", 1.0f);
}


void create_footpath(Vector2f start, Vector2f end) {
    create_path(start, end, "footpath", "footpath_end", 0.5f);
}


void draw_path(int camera, int entity) {
    ImageComponent* image = ImageComponent_get(entity);
    PathComponent* path = PathComponent_get(entity);
    if (!path) return;
    if (path->next == NULL_ENTITY) return;

    PathComponent* next = PathComponent_get(path->next);
    
    Vector2f p0;
    Vector2f p1 = get_position(entity);
    Vector2f p2 = get_position(path->next);
    Vector2f p3;

    if (path->prev != NULL_ENTITY) {
        p0 = get_position(path->prev);
    } else {
        p0 = sum(p1, diff(p1, p2));
    }

    if (next->next != NULL_ENTITY) {
        p3 = get_position(next->next);
    } else {
        p3 = sum(p2, diff(p2, p1));
    }

    // TODO: check if on-screen
    bool flip = next->next == NULL_ENTITY;
    draw_spline(camera, image->texture_index, p0, p1, p2, p3, image->height, flip, COLOR_WHITE);
}


void resize_roads(ComponentData* components) {
    for (int i = 0; i < components->entities; i++) {
        ColliderComponent* col = ColliderComponent_get(i);
        if (col && col->group == GROUP_ROADS) {
            col->height = 1.0;
        }
    }
}


void debug_draw_paths(Entity camera) {
    Color color = get_color(1.0f, 1.0f, 0.0f, 0.5f);

    for (Entity i = 0; i < game_data->components->entities; i++) {
        PathComponent* path = PathComponent_get(i);
        if (!path) continue;

        draw_circle(camera, get_position(i), 0.5f, color);

        if (path->next == NULL_ENTITY) continue;

        PathComponent* next = PathComponent_get(path->next);
    
        Vector2f p0;
        Vector2f p1 = get_position(i);
        Vector2f p2 = get_position(path->next);
        Vector2f p3;

        if (path->prev != NULL_ENTITY) {
            p0 = get_position(path->prev);
        } else {
            p0 = sum(p1, diff(p1, p2));
        }
    
        if (next->next != NULL_ENTITY) {
            p3 = get_position(next->next);
        } else {
            p3 = sum(p2, diff(p2, p1));
        }
        // draw_spline(game_data->camera, -1, p0, p1, p2, p3, path->width, false, color);
    }
}
