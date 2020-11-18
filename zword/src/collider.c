#define _USE_MATH_DEFINES

#include <stdbool.h>
#include <math.h>

#include <SFML/System/Vector2.h>
#include <SFML/Graphics.h>

#include "component.h"
#include "camera.h"
#include "util.h"


sfVector2f overlap_circle_circle(Component* component, int i, int j) {
    sfVector2f overlap = { 0, 0 };

    sfVector2f a = component->coordinate[i]->position;
    sfVector2f b = component->coordinate[j]->position;

    float d = dist(a, b);
    float r = component->circle_collider[i]->radius + component->circle_collider[j]->radius;

    if (d < r) {
        if (d < 0.001) {
            overlap.y = r;
        }
        else {
            overlap.x = (a.x - b.x) * (r - d) / d;
            overlap.y = (a.y - b.y) * (r - d) / d;
        }
    }

    return overlap;
}

float axis_half_width(Component* component, int i, sfVector2f axis) {
    sfVector2f half_width = polar_to_cartesian(0.5 * component->rectangle_collider[i]->width, component->coordinate[i]->angle);
    sfVector2f half_height = polar_to_cartesian(0.5 * component->rectangle_collider[i]->height, component->coordinate[i]->angle + 0.5 * M_PI);
    return fabs(dot(half_width, axis)) + fabs(dot(half_height, axis));
}

float axis_overlap(float w1, sfVector2f r1, float w2, sfVector2f r2, sfVector2f axis) {
    sfVector2f r21 = { r1.x - r2.x, r1.y - r2.y };
    float r = dot(r21, axis);
    float o = w1 + w2 - fabs(r);
    if (o > 0.0) {
        if (fabs(r) < 0.001) {
            return o;
        }
        else {
            return copysignf(o, r);
        }
    }

    return 0.0;
}

sfVector2f overlap_rectangle_circle(Component* component, int i, int j) {
    sfVector2f a = component->coordinate[i]->position;
    sfVector2f b = component->coordinate[j]->position;
    float radius = component->circle_collider[j]->radius;

    bool near_corner = true;

    float angle = component->coordinate[i]->angle;
    sfVector2f half_width = polar_to_cartesian(0.5 * component->rectangle_collider[i]->width, angle);
    sfVector2f half_height = polar_to_cartesian(0.5 * component->rectangle_collider[i]->height, angle + 0.5 * M_PI);

    sfVector2f axes[2];
    float overlaps[2];

    axes[0] = polar_to_cartesian(1.0, angle);
    axes[1] = polar_to_cartesian(1.0, angle + 0.5 * M_PI);

    for (int k = 0; k < 2; k++) {
        overlaps[k] = axis_overlap(axis_half_width(component, i, axes[k]), a, radius, b, axes[k]);

        if (fabs(overlaps[k]) < 0.001) {
            return (sfVector2f) { 0.0, 0.0 };
        }
        if (fabs(overlaps[k]) >= radius) {
            near_corner = false;
        }
    }

    int k = abs_argmin(overlaps, 2);
    if (!near_corner) {
        return (sfVector2f) { overlaps[k] * axes[k].x, overlaps[k] * axes[k].y };
    }

    sfVector2f corner;
    corner.x = a.x - copysignf(half_width.x, overlaps[0]) - copysignf(half_height.x, overlaps[1]);
    corner.y = a.y - copysignf(half_width.y, overlaps[0]) - copysignf(half_height.y, overlaps[1]);

    sfVector2f axis = { corner.x - b.x, corner.y - b.y };
    axis = normalized(axis);

    float overlap = axis_overlap(axis_half_width(component, i, axis), a, radius, b, axis);

    if (0.0 < fabs(overlap) && fabs(overlap) < fabs(overlaps[k])) {
        return (sfVector2f) { overlap * axis.x, overlap * axis.y };
    }

    return (sfVector2f) { 0.0, 0.0 };
}

sfVector2f overlap_circle_rectangle(Component* component, int i, int j) {
    sfVector2f overlap = overlap_rectangle_circle(component, j, i);
    overlap.x *= -1;
    overlap.y *= -1;
    return overlap;
}

sfVector2f overlap_rectangle_rectangle(Component* component, int i, int j) {
    sfVector2f pos_i = component->coordinate[i]->position;
    sfVector2f pos_j = component->coordinate[j]->position;

    float angle_i = component->coordinate[i]->angle;
    float angle_j = component->coordinate[i]->angle;

    sfVector2f axes[4] = {polar_to_cartesian(1.0, angle_i), polar_to_cartesian(1.0, angle_i + 0.5 * M_PI), 
                          polar_to_cartesian(1.0, angle_j), polar_to_cartesian(1.0, angle_j + 0.5 * M_PI)};
    float overlaps[4];

    for (int k = 0; k < 4; k++) {
        overlaps[k] = axis_overlap(axis_half_width(component, i, axes[k]), pos_i,
                                   axis_half_width(component, j, axes[k]), pos_j, axes[k]);

        if (fabs(overlaps[k]) < 0.001) {
            return (sfVector2f) { 0.0, 0.0 };
        }
    }

    int k = abs_argmin(overlaps, 4);

    return (sfVector2f) { overlaps[k] * axes[k].x, overlaps[k] * axes[k].y };
}

void collide(Component* component) {
    for (int i = 0; i <= component->entities; i++) {
        if (!component->physics[i]) continue;

        for (int j = 0; j <= component->entities; j++) {
            if (i == j) continue;

            sfVector2f overlap = { 0, 0 };
            if (component->circle_collider[i]) {
                if (component->circle_collider[j]) {
                    overlap = overlap_circle_circle(component, i, j);
                } else if (component->rectangle_collider[j]) {
                    overlap = overlap_circle_rectangle(component, i, j);
                }
            } else if (component->rectangle_collider[i]) {
                if (component->circle_collider[j]) {
                    overlap = overlap_rectangle_circle(component, i, j);
                } else if (component->rectangle_collider[j]) {
                    overlap = overlap_rectangle_rectangle(component, i, j);
                }
            }

            if (overlap.x == 0 && overlap.y == 0) continue;

            component->coordinate[i]->position.x += overlap.x;
            component->coordinate[i]->position.y += overlap.y;

            float v = dot(component->physics[i]->velocity, overlap);
            float n = norm2(overlap);
            component->physics[i]->velocity.x -= 2 * v * overlap.x / n;
            component->physics[i]->velocity.y -= 2 * v * overlap.y / n;

            component->physics[i]->velocity.x *= component->physics[i]->bounce;
            component->physics[i]->velocity.y *= component->physics[i]->bounce;
        }
    }
}

void debug_draw(Component* component, sfRenderWindow* window, Camera* camera) {
    for (int i = 0; i <= component->entities; i++) {
        if (component->circle_collider[i]) {
            sfVector2f pos = component->coordinate[i]->position;
            pos.x -= component->circle_collider[i]->radius;
            pos.y += component->circle_collider[i]->radius;
            sfCircleShape_setPosition(component->circle_collider[i]->shape, world_to_screen(pos, camera));
            sfCircleShape_setFillColor(component->circle_collider[i]->shape, sfColor_fromRGB(0, 255, 255));
            sfCircleShape_setRadius(component->circle_collider[i]->shape, component->circle_collider[i]->radius * camera->zoom);
            sfRenderWindow_drawCircleShape(window, component->circle_collider[i]->shape, NULL);
        } else if (component->rectangle_collider[i]) {
            RectangleColliderComponent* col = component->rectangle_collider[i];

            sfVector2f pos = component->coordinate[i]->position;
            pos.x -= 0.5 * col->width;
            pos.y += 0.5 * col->height;
            sfRectangleShape_setPosition(col->shape, world_to_screen(pos, camera));

            sfRectangleShape_setFillColor(col->shape, sfColor_fromRGB(0, 255, 255));
            sfVector2f size = { col->width * camera->zoom, col->height * camera->zoom };
            sfRectangleShape_setSize(col->shape, size);

            sfRectangleShape_setOrigin(col->shape, (sfVector2f) { 0.5 * col->width * camera->zoom, 0.5 * col->height * camera->zoom });
            sfRectangleShape_setRotation(col->shape, -to_degrees(component->coordinate[i]->angle));

            sfRenderWindow_drawRectangleShape(window, col->shape, NULL);
        }
    }
}
