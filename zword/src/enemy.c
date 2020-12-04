#define _USE_MATH_DEFINES

#include <math.h>

#include <SFML/Window.h>
#include <SFML/System/Vector2.h>

#include "component.h"
#include "raycast.h"
#include "grid.h"
#include "util.h"
#include "navigation.h"


void update_enemies(Component* component) {
    for (int i = 0; i < component->entities; i++) {
        if (!component->enemy[i]) continue;

        EnemyComponent* enemy = component->enemy[i];
        PhysicsComponent* phys = component->physics[i];

        if (enemy->health <= 0) {
            component->circle_collider[i]->enabled = false;
            continue;
        }

        if (enemy->target == -1) {
            for (int j = 0; j < component->entities; j++) {
                if (!component->player[j]) continue;

                enemy->target = j;
                break;
            }
        } else {
            int target = component->player[enemy->target]->vehicle;
            if (target == -1) {
                target = enemy->target;
            }

            a_star(component, target, i, enemy->path);

            if (enemy->path[1] != -1) {
                sfVector2f r = diff(get_position(component, enemy->path[1]), get_position(component, i));
                phys->acceleration = sum(phys->acceleration, mult(enemy->acceleration, normalized(r)));
            }
        }
    }
}


void create_enemy(Component* component, sfVector2f pos) {
    int i = component->entities;
    component->entities++;

    component->coordinate[i] = CoordinateComponent_create(pos, float_rand(0.0, 2 * M_PI));
    component->image[i] = ImageComponent_create("player", 1.0);
    component->circle_collider[i] = CircleColliderComponent_create(0.5);
    component->physics[i] = PhysicsComponent_create(1.0, 0.0, 0.5, 5.0, 10.0);
    component->physics[i]->max_speed = 5.0;
    component->enemy[i] = EnemyComponent_create();
    component->particle[i] = ParticleComponent_create(0.0, 2 * M_PI, 0.5, 0.0, 5.0, 10.0, sfColor_fromRGB(200, 0, 0), sfColor_fromRGB(255, 0, 0));
    component->waypoint[i] = WaypointComponent_create();
}


void draw_enemies(Component* component, sfRenderWindow* window, Camera* camera) {
    for (int i = 0; i < component->entities; i++) {
        EnemyComponent* enemy = component->enemy[i];
        if (!enemy) continue;

        for (int j = 0; j < MAX_PATH_LENGTH; j++) {
            if (enemy->path[j + 1] == -1) break;

            draw_line(window, camera, NULL, get_position(component, enemy->path[j]), get_position(component, enemy->path[j + 1]), 0.05, sfRed);
        }

        if (enemy->path[1] != -1) {
            draw_circle(window, camera, NULL, get_position(component, enemy->path[1]), 0.1, sfGreen);
        }
    }
}
