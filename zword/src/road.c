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

void create_road(ComponentData* components, sfVector2f start, sfVector2f end, Permutation perm) {
    int n = dist(start, end) / 5.0;

    sfVector2f position = end;
    sfVector2f delta = mult(1.0 / n, diff(start, end));

    int next = -1;
    int current = -1;

    for (int i = 0; i < n; i++) {
        current = create_entity(components);

        position = sum(position, delta);

        sfVector2f grad = perlin_grad(position, perm);
        sfVector2f pos = diff(position, mult(2.0, grad));

        CoordinateComponent_add(components, current, pos, 0.0);
        RoadComponent* road = RoadComponent_add(components, current);
        road->next = next;
        // WaypointComponent_add(components, current);
        ImageComponent_add(components, current, "", 1.1, 1.0, 1);

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
        ImageComponent_add(components, i, "road_tile", length + 0.25, road->width, 1);
        ColliderComponent_add_rectangle(components, i, d, 1.0, ROADS);

        if (road->prev == -1) {
            i = create_entity(components);
            CoordinateComponent_add(components, i, pos, angle);
            ImageComponent_add(components, i, "road_end", road->width, road->width, 1);
        }

        if (next_road->next == -1) {
            i = create_entity(components);
            CoordinateComponent_add(components, i, next_pos, angle + M_PI);
            ImageComponent_add(components, i, "road_end", road->width, road->width, 1);
        }

        current = RoadComponent_get(components, current)->next;
    }
}


void draw_curve(sfRenderWindow* window, ComponentData* components, int camera, sfConvexShape* shape, int n, sfVector2f position, float range, float angle, float spread) { 
    CameraComponent* cam = CameraComponent_get(components, camera);
    sfConvexShape_setPoint(shape, 0, zeros());
    sfConvexShape_setPoint(shape, n - 1, zeros());

    float ang = 0.0;
    for (int i = 1; i < n - 1; i++) {
        sfVector2f point = polar_to_cartesian(cam->zoom * range, ang);
        sfConvexShape_setPoint(shape, i, point);
        ang += spread / (n - 3);
    }

    sfRenderWindow_drawConvexShape(window, shape, NULL);

    sfConvexShape_setPosition(shape, world_to_screen(components, camera, position));
    sfConvexShape_setRotation(shape, -to_degrees(angle + 0.5 * spread));
}



void draw_road(ComponentData* components, sfRenderWindow* window, int camera, TextureArray textures, int entity) {
    RoadComponent* road = RoadComponent_get(components, entity);
    if (!road) return;

    if (road->texture_changed) {
        int i = texture_index(road->filename);
        sfConvexShape_setTexture(road->shape, textures[i], false);
        float w = PIXELS_PER_UNIT * road->width;
        float h = PIXELS_PER_UNIT * road->width * sinf(fabs(road->curve));
        sfConvexShape_setTextureRect(road->shape, (sfIntRect) { 0, 0, w, h });
        road->texture_changed = false;
    }

    sfVector2f pos = get_position(components, entity);
    float angle = get_angle(components, entity);

    float margin = 0.5 * road->width * tanf(0.5 * fabs(road->curve));
    pos = diff(pos, polar_to_cartesian(sqrtf(margin * margin + 0.25 * road->width * road->width), angle));

    draw_curve(window, components, camera, road->shape, 12, pos, 4.0, angle, fabs(road->curve));
}
