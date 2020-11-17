#include <stdbool.h>

#include <SFML/System/Vector2.h>
#include <SFML/Graphics.h>

#include "collider.h"
#include "util.h"
#include "component.h"
#include "camera.h"


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

float axis_half_width(sfVector2f half_width, sfVector2f half_height, sfVector2f axis) {
    return fabs(dot(half_width, axis)) + fabs(dot(half_height, axis));
}

float axis_overlap(float w1, sfVector2f r1, float w2, sfVector2f r2, sfVector2f axis) {
    sfVector2f r21 = { r1.x - r2.x, r1.y - r2.y };
    float r = dot(r21, axis);
    float o = w1 + w2 - abs(r);
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
    sfVector2f overlap = { 0, 0 };

    sfVector2f a = component->coordinate[i]->position;
    sfVector2f b = component->coordinate[j]->position;
    float radius = component->circle_collider[j]->radius;

    bool near_corner = true;

    float angle = component->coordinate[i]->angle;
    sfVector2f half_width = polar_to_cartesian(0.5 * component->rectangle_collider[i]->width, angle);
    sfVector2f half_height = polar_to_cartesian(0.5 * component->rectangle_collider[i]->height, angle + 0.5 * M_PI);

    sfVector2f axis_1 = polar_to_cartesian(1.0, angle);
    float overlap_1 = axis_overlap(axis_half_width(half_width, half_height, axis_1), a, radius, b, axis_1);

    if (fabs(overlap_1) < 0.001) {
        return overlap;
    }
    if (fabs(overlap_1) >= radius) {
        near_corner = false;
    }

    sfVector2f axis_2 = polar_to_cartesian(1.0, angle + 0.5 * M_PI);
    float overlap_2 = axis_overlap(axis_half_width(half_width, half_height, axis_2), a, radius, b, axis_2);

    if (fabs(overlap_2) < 0.001) {
        return overlap;
    }
    if (fabs(overlap_2) >= radius) {
        near_corner = false;
    }

    if (!near_corner) {
        if (fabs(overlap_1) < fabs(overlap_2)) {
            axis_1.x *= overlap_1;
            axis_1.y *= overlap_1;
            return axis_1;
        } else {
            axis_2.x *= overlap_1;
            axis_2.y *= overlap_1;
            return axis_2;
        }
    }

    sfVector2f corner;

    return overlap;
}


void collide(Component* component) {
    for (int i = 0; i <= component->entities; i++) {
        if (!component->physics[i] || !component->circle_collider[i]) continue;

        component->circle_collider[i]->position = component->coordinate[i]->position;

        for (int j = 0; j <= component->entities; j++) {
            if (i == j || !component->circle_collider[j]) continue;

            sfVector2f ol = overlap_circle_circle(component, i, j);

            component->coordinate[i]->position.x += ol.x;
            component->coordinate[i]->position.y += ol.y;
        }
    }
}

void debug_draw(Component* component, sfRenderWindow* window, Camera* camera) {
    for (int i = 0; i <= component->entities; i++) {
        if (!component->circle_collider[i]) continue;

        sfVector2f pos = component->coordinate[i]->position;
        pos.x -= component->circle_collider[i]->radius;
        pos.y += component->circle_collider[i]->radius;
        sfCircleShape_setPosition(component->circle_collider[i]->shape, world_to_screen(pos, camera));
        sfCircleShape_setFillColor(component->circle_collider[i]->shape, sfColor_fromRGB(0, 255, 255));
        sfCircleShape_setRadius(component->circle_collider[i]->shape, component->circle_collider[i]->radius * camera->zoom);
        sfRenderWindow_drawCircleShape(window, component->circle_collider[i]->shape, NULL);
    }
}
