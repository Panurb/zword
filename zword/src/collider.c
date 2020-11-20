#define _USE_MATH_DEFINES

#include <stdio.h>
#include <stdbool.h>
#include <math.h>

#include <SFML/System/Vector2.h>
#include <SFML/Graphics.h>

#include "component.h"
#include "camera.h"
#include "util.h"


sfVector2f half_width(Component* component, int i) {
    return polar_to_cartesian(0.5 * component->rectangle_collider[i]->width, component->coordinate[i]->angle);
}

sfVector2f half_height(Component* component, int i) {
    return polar_to_cartesian(0.5 * component->rectangle_collider[i]->height, component->coordinate[i]->angle + 0.5 * M_PI);
}

float axis_half_width(Component* component, int i, sfVector2f axis) {
    sfVector2f hw = half_width(component, i);
    sfVector2f hh = half_height(component, i);
    return fabs(dot(hw, axis)) + fabs(dot(hh, axis));
}

float axis_overlap(float w1, sfVector2f r1, float w2, sfVector2f r2, sfVector2f axis) {
    float r = dot(diff(r1, r2), axis);
    float o = w1 + w2 - fabs(r);
    if (o > 0.0) {
        if (fabs(r) < 1e-6) {
            return o;
        } else {
            return copysignf(o, r);
        }
    }

    return 0.0;
}

sfVector2f overlap_circle_circle(Component* component, int i, int j) {
    sfVector2f a = component->coordinate[i]->position;
    sfVector2f b = component->coordinate[j]->position;

    float d = dist(a, b);
    float r = component->circle_collider[i]->radius + component->circle_collider[j]->radius;

    if (d > r) {
        return (sfVector2f) { 0.0, 0.0 };
    }

    if (d < 1e-6) {
        return (sfVector2f) { 0.0, r };
    }

    return mult(r / d - 1, diff(a, b));
}

sfVector2f overlap_rectangle_circle(Component* component, int i, int j) {
    bool near_corner = true;

    float overlaps[2] = { 0.0, 0.0 };
    sfVector2f axes[2];
    axes[0] = polar_to_cartesian(1.0, component->coordinate[i]->angle);
    axes[1] = perp(axes[0]);

    sfVector2f a = component->coordinate[i]->position;
    sfVector2f b = component->coordinate[j]->position;
    float radius = component->circle_collider[j]->radius;

    for (int k = 0; k < 2; k++) {
        overlaps[k] = axis_overlap(axis_half_width(component, i, axes[k]), a, radius, b, axes[k]);

        if (fabs(overlaps[k]) < 1e-6) {
            return (sfVector2f) { 0.0, 0.0 };
        }

        if (fabs(overlaps[k]) >= radius) {
            near_corner = false;
        }
    }

    int k = abs_argmin(overlaps, 2);
    if (!near_corner) {
        return mult(overlaps[k], axes[k]);
    }

    sfVector2f hw = half_width(component, i);
    sfVector2f hh = half_height(component, i);

    sfVector2f corner = diff(a, sum(mult(sign(overlaps[0]), hw), mult(sign(overlaps[1]), hh)));

    sfVector2f axis = normalized(diff(corner, b));

    float overlap = axis_overlap(axis_half_width(component, i, axis), a, radius, b, axis);

    if (0.0 < fabs(overlap) && fabs(overlap) < fabs(overlaps[k])) {
        return mult(overlap, axis);
    }

    return (sfVector2f) { 0.0, 0.0 };
}

sfVector2f overlap_circle_rectangle(Component* component, int i, int j) {
    return mult(-1.0, overlap_rectangle_circle(component, j, i));
}

sfVector2f overlap_rectangle_rectangle(Component* component, int i, int j) {
    sfVector2f hw_i = polar_to_cartesian(1.0, component->coordinate[i]->angle);
    sfVector2f hw_j = polar_to_cartesian(1.0, component->coordinate[j]->angle);

    float overlaps[4];
    sfVector2f axes[4] = { hw_i, perp(hw_i), hw_j, perp(hw_j) };

    sfVector2f a = component->coordinate[i]->position;
    sfVector2f b = component->coordinate[j]->position;

    for (int k = 0; k < 4; k++) {
        overlaps[k] = axis_overlap(axis_half_width(component, i, axes[k]), a,
                                   axis_half_width(component, j, axes[k]), b, axes[k]);

        if (fabs(overlaps[k]) < 1e-6) {
            return (sfVector2f) { 0.0, 0.0 };
        }
    }

    int k = abs_argmin(overlaps, 4);

    return mult(overlaps[k], axes[k]);
}

sfVector2f overlap(Component* component, int i, int j) {
    sfVector2f ol = { 0, 0 };
    if (component->circle_collider[i]) {
        if (component->circle_collider[j]) {
            ol = overlap_circle_circle(component, i, j);
        } else if (component->rectangle_collider[j]) {
            ol = overlap_circle_rectangle(component, i, j);
        }
    } else if (component->rectangle_collider[i]) {
        if (component->circle_collider[j]) {
            ol = overlap_rectangle_circle(component, i, j);
        } else if (component->rectangle_collider[j]) {
            ol = overlap_rectangle_rectangle(component, i, j);
        }
    }

    return ol;
}

void collide(Component* component) {
    for (int i = 0; i < component->entities; i++) {
        PhysicsComponent* physics = component->physics[i];
        if (!physics) continue;

        for (int j = 0; j < component->entities; j++) {
            if (i == j) continue;

            sfVector2f ol = overlap(component, i, j);

            if (ol.x == 0.0 && ol.y == 0.0) continue;

            float m = 1.0;
            sfVector2f other_vel = { 0.0, 0.0 };
            if (component->physics[j]) {
                m = component->physics[j]->mass / (physics->mass + component->physics[j]->mass);
                other_vel = component->physics[j]->velocity;
            }

            sfVector2f dv = diff(physics->velocity, other_vel);
            sfVector2f n = normalized(ol);
            sfVector2f new_vel = diff(physics->velocity, mult(2 * m * dot(dv, n), n));

            physics->collision.velocity = sum(physics->collision.velocity, new_vel);

            physics->collision.overlap = sum(physics->collision.overlap, mult(m, ol));
        }
    }
}

void debug_draw(Component* component, sfRenderWindow* window, Camera* camera) {
    for (int i = 0; i < component->entities; i++) {
        if (component->circle_collider[i]) {
            CircleColliderComponent* col = component->circle_collider[i];

            sfCircleShape_setOrigin(col->shape, (sfVector2f) { col->radius * camera->zoom, col->radius * camera->zoom });

            sfVector2f pos = component->coordinate[i]->position;
            sfCircleShape_setPosition(col->shape, world_to_screen(pos, camera));

            sfCircleShape_setRadius(col->shape, col->radius * camera->zoom);

            sfRenderWindow_drawCircleShape(window, col->shape, NULL);
        } else if (component->rectangle_collider[i]) {
            RectangleColliderComponent* col = component->rectangle_collider[i];

            sfRectangleShape_setOrigin(col->shape, (sfVector2f) { 0.5 * col->width * camera->zoom, 0.5 * col->height * camera->zoom });

            sfVector2f pos = component->coordinate[i]->position;
            sfRectangleShape_setPosition(col->shape, world_to_screen(pos, camera));

            sfVector2f size = { col->width * camera->zoom, col->height * camera->zoom };
            sfRectangleShape_setSize(col->shape, size);

            sfRectangleShape_setRotation(col->shape, -to_degrees(component->coordinate[i]->angle));

            sfRenderWindow_drawRectangleShape(window, col->shape, NULL);
        }
    }
}
