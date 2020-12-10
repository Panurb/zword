#define _USE_MATH_DEFINES

#include <math.h>
#include <string.h>

#include <SFML/Window.h>
#include <SFML/System/Vector2.h>

#include "component.h"
#include "raycast.h"
#include "grid.h"
#include "util.h"
#include "navigation.h"


void update_enemies(ComponentData* component, ColliderGrid* grid) {
    for (int i = 0; i < component->entities; i++) {
        if (!component->enemy[i]) continue;

        EnemyComponent* enemy = component->enemy[i];
        PhysicsComponent* phys = component->physics[i];
        ImageComponent* image = ImageComponent_get(component, i);

        switch (enemy->state) {
            case IDLE:
                for (int j = 0; j < component->entities; j++) {
                    if (!component->player[j]) continue;

                    sfVector2f r = diff(get_position(component, j), get_position(component, i));
                    float angle = mod(get_angle(component, i) - polar_angle(r), 2 * M_PI);

                    if (angle < 0.5 * enemy->fov) {
                        HitInfo info = raycast(component, grid, get_position(component, i), r, enemy->vision_range, i);
                        if (info.object == j) {
                            enemy->target = j;
                            enemy->state = CHASE;
                            break;
                        }
                    }
                }

                break;
            case INVESTIGATE:
                break;
            case CHASE:
                ;
                int target = component->player[enemy->target]->vehicle;
                if (target == -1) {
                    target = enemy->target;
                }

                a_star(component, target, i, enemy->path);

                if (enemy->path[1] != -1) {
                    sfVector2f r = diff(get_position(component, enemy->path[1]), get_position(component, i));

                    float d = norm(r);

                    if (d > 1.0) {
                        phys->acceleration = sum(phys->acceleration, mult(enemy->acceleration / d, r));
                    }

                    component->coordinate[i]->angle = polar_angle(r);
                }

                break;
            case DEAD:
                if (norm(phys->velocity) == 0.0) {
                    component->collider[i]->enabled = false;
                }
                strcpy(image->filename, "zombie_dead");
                image->width = 2.0;

                break;
        }

        if (component->health[i]->health <= 0) {
            enemy->state = DEAD;
        }
    }
}


void create_enemy(ComponentData* component, sfVector2f pos) {
    int i = get_index(component);

    float angle = float_rand(0.0, 2 * M_PI);
    component->coordinate[i] = CoordinateComponent_create(pos, angle);
    ImageComponent_add(component, i, "zombie", 1.0, 1.0, 4);
    //component->image[i]->shine = 0.5;
    component->collider[i] = ColliderComponent_create_circle(0.5);
    component->physics[i] = PhysicsComponent_create(1.0, 0.0, 0.5, 5.0, 10.0);
    component->physics[i]->max_speed = 5.0;
    component->enemy[i] = EnemyComponent_create();
    component->particle[i] = ParticleComponent_create(0.0, 2 * M_PI, 0.5, 0.0, 5.0, 10.0, sfColor_fromRGB(200, 0, 0), sfColor_fromRGB(255, 0, 0));
    component->waypoint[i] = WaypointComponent_create();
    component->health[i] = HealthComponent_create(100);

    //EnemyComponent* enemy = component->enemy[i];
    //component->light[i] = LightComponent_create(enemy->vision_range, enemy->fov, 51, sfGreen, 0.1, 1.0);
}


void draw_enemies(ComponentData* component, sfRenderWindow* window, Camera* camera) {
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
