#include <stdio.h>

#include "road.h"


sfVector2f perlin_grad(sfVector2f position, Permutation perm) {
    float dx = 0.1;
    float dy = 0.1;
    float x1 = perlin(position.x - dx, position.y, 0.0, perm, -1);
    float x2 = perlin(position.x + dx, position.y, 0.0, perm, -1);
    float y1 = perlin(position.x, position.y - dy, 0.0, perm, -1);
    float y2 = perlin(position.x, position.y + dy, 0.0, perm, -1);
    sfVector2f grad = { (x2 - x1) / dx, (y2 - y1) / dy };

    return grad;
}


int create_road_curves(ComponentData* components, sfVector2f start, sfVector2f end, Permutation perm, float curviness, float width, Filename filename) {
    int n = dist(start, end) / 5.0;

    sfVector2f position = end;
    sfVector2f delta = mult(1.0 / n, diff(start, end));

    int next = -1;
    int current = -1;

    for (int i = 0; i < n; i++) {
        current = create_entity(components);

        position = sum(position, delta);

        sfVector2f grad = perlin_grad(position, perm);
        sfVector2f pos = diff(position, mult(curviness, grad));

        CoordinateComponent_add(components, current, pos, 0.0);
        RoadComponent* road = RoadComponent_add(components, current, width, filename);
        road->next = next;
        ImageComponent_add(components, current, "", 1.0, 1.0, 1);

        if (next != -1) {
            RoadComponent* next_road = RoadComponent_get(components, next);
            next_road->prev = current;

            if (next_road->next != -1) {
                sfVector2f next_pos = get_position(components, road->next);
                sfVector2f next_next_pos = get_position(components, next_road->next);
                sfVector2f r1 = diff(next_pos, pos);
                sfVector2f r2 = diff(next_next_pos, next_pos);
                next_road->curve = signed_angle(r1, r2);

                sfVector2f v = rotate(bisector(r1, r2), -0.5 * M_PI * sign(next_road->curve));
                CoordinateComponent_get(components, next)->angle = polar_angle(v);
            }
        }

        next = current;
    }

    return current;
}


void create_road_segments(ComponentData* components, int current) {
    while (true) {
        RoadComponent* road = RoadComponent_get(components, current);
        if (road->next == -1) {
            break;
        }
        RoadComponent* next_road = RoadComponent_get(components, road->next);

        int i = create_entity(components);

        sfVector2f pos = get_position(components, current);
        sfVector2f next_pos = get_position(components, road->next);

        float margin_1 = 0.5 * road->width * tanf(0.5 * fabs(road->curve));
        float margin_2 = 0.5 * road->width * tanf(0.5 * fabs(next_road->curve));

        float d = dist(next_pos, pos);
        float length = d - margin_1 - margin_2;

        sfVector2f r = sum(pos, mult((0.5 * length + margin_1) / d, diff(next_pos, pos)));

        float angle = polar_angle(diff(next_pos, pos));
        CoordinateComponent_add(components, i, r, angle);
        Filename filename;
        snprintf(filename, 20, "%s%s", road->filename, "_tile");
        ImageComponent_add(components, i, filename, length + 0.25, road->width, 1);
        ColliderComponent_add_rectangle(components, i, d, road->width, ROADS);

        snprintf(filename, 20, "%s%s", road->filename, "_end");

        if (road->prev == -1) {
            i = create_entity(components);
            CoordinateComponent_add(components, i, pos, angle);
            ImageComponent_add(components, i, filename, road->width, road->width, 1);
        }

        if (next_road->next == -1) {
            i = create_entity(components);
            CoordinateComponent_add(components, i, next_pos, angle + M_PI);
            ImageComponent_add(components, i, filename, road->width, road->width, 1);
        }

        current = RoadComponent_get(components, current)->next;
    }
}


void create_road(ComponentData* components, sfVector2f start, sfVector2f end, Permutation perm) {
    int current = create_road_curves(components, start, end, perm, 2.0, 4.0, "road");
    create_road_segments(components, current);
}


void create_river(ComponentData* components, sfVector2f start, sfVector2f end, Permutation perm) {
    int current = create_road_curves(components, start, end, perm, 1.0, 8.0, "river");
    create_road_segments(components, current);
}


void draw_road(ComponentData* components, sfRenderWindow* window, int camera, TextureArray textures, int entity) {
    RoadComponent* road = RoadComponent_get(components, entity);
    if (!road) return;

    float angle = get_angle(components, entity);
    float spread = fabs(road->curve);

    if (road->texture_changed) {
        Filename filename;
        snprintf(filename, 20, "%s%s", road->filename, "_curve");
        int i = texture_index(filename);

        sfConvexShape_setTexture(road->shape, textures[i], false);
        float w = PIXELS_PER_UNIT * road->width;
        float h = PIXELS_PER_UNIT * road->width * sinf(fabs(road->curve));
        sfConvexShape_setTextureRect(road->shape, (sfIntRect) { 0, 0, w, h });

        sfConvexShape_setPoint(road->shape, 0, zeros());
        sfConvexShape_setPoint(road->shape, road->points - 1, zeros());

        float ang = 0.0;
        for (int i = 1; i < road->points - 1; i++) {
            sfVector2f point = polar_to_cartesian(road->width, ang);
            sfConvexShape_setPoint(road->shape, i, point);
            ang += fabs(road->curve) / (road->points - 3);
        }

        sfConvexShape_setRotation(road->shape, -to_degrees(angle + 0.5 * spread));
        road->texture_changed = false;
    }

    sfVector2f pos = get_position(components, entity);
    float margin = 0.5 * road->width * tanf(0.5 * spread);
    pos = diff(pos, polar_to_cartesian(sqrtf(margin * margin + 0.25 * road->width * road->width), angle));

    CameraComponent* cam = CameraComponent_get(components, camera);

    sfConvexShape_setPosition(road->shape, world_to_screen(components, camera, pos));
    sfConvexShape_setScale(road->shape, (sfVector2f) { cam->zoom, cam->zoom });

    sfRenderWindow_drawConvexShape(window, road->shape, NULL);
}


void resize_roads(ComponentData* components) {
    for (int i = 0; i < components->entities; i++) {
        ColliderComponent* col = ColliderComponent_get(components, i);
        if (col && col->group == ROADS) {
            col->height = 1.0;
        }
    }
}
