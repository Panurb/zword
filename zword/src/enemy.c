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
#include "image.h"


void create_enemy(ComponentData* components, sfVector2f pos) {
    int i = create_entity(components);

    float angle = float_rand(0.0, 2 * M_PI);
    
    CoordinateComponent_add(components, i, pos, angle);
    ImageComponent_add(components, i, "zombie", 1.0, 1.0, 4);
    //component->image[i]->shine = 0.5;
    ColliderComponent_add_circle(components, i, 0.5, ENEMIES);
    PhysicsComponent_add(components, i, 1.0, 0.0, 0.5, 5.0, 10.0)->max_speed = 5.0;
    EnemyComponent_add(components, i);
    ParticleComponent_add(components, i, 0.0, 2 * M_PI, 0.5, 0.0, 5.0, 10.0, get_color(0.78, 0.0, 0.0, 1.0), sfRed);
    WaypointComponent_add(components, i);
    HealthComponent_add(components, i, 100);

    //EnemyComponent* enemy = component->enemy[i];
    //component->light[i] = LightComponent_create(enemy->vision_range, enemy->fov, 51, sfGreen, 0.1, 1.0);
}


void update_enemies(ComponentData* components, ColliderGrid* grid) {
    for (int i = 0; i < components->entities; i++) {
        EnemyComponent* enemy = EnemyComponent_get(components, i);
        if (!enemy) continue;

        PhysicsComponent* phys = PhysicsComponent_get(components, i);
        ImageComponent* image = ImageComponent_get(components, i);

        if (components->health[i]->health <= 0) {
            enemy->state = DEAD;
        }

        switch (enemy->state) {
            case IDLE:
                for (int j = 0; j < components->entities; j++) {
                    if (!PlayerComponent_get(components, j)) continue;

                    sfVector2f r = diff(get_position(components, j), get_position(components, i));
                    float angle = mod(get_angle(components, i) - polar_angle(r), 2 * M_PI);

                    if (angle < 0.5 * enemy->fov) {
                        HitInfo info = raycast(components, grid, get_position(components, i), r, enemy->vision_range, i);
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
                int target = PlayerComponent_get(components, enemy->target)->vehicle;
                if (target == -1) {
                    target = enemy->target;
                }

                a_star(components, target, i, enemy->path);

                if (enemy->path[1] != -1) {
                    sfVector2f r = diff(get_position(components, enemy->path[1]), get_position(components, i));

                    float d = norm(r);

                    if (d > 1.0) {
                        phys->acceleration = sum(phys->acceleration, mult(enemy->acceleration / d, r));
                    }

                    components->coordinate[i]->angle = polar_angle(r);
                }

                

                break;
            case DEAD:
                ;
                ColliderComponent* col = ColliderComponent_get(components, i);

                if (col) {
                    col->group = ITEMS;
                    if (phys->speed == 0.0) {
                        clear_grid(components, grid, i);
                        ColliderComponent_remove(components, i);
                    }
                }

                strcpy(image->filename, "zombie_dead");
                image->width = 2.0;
                image->texture_changed = true;
                change_layer(components, i, 2);

                break;
        }
    }
}


void draw_enemies(ComponentData* component, sfRenderWindow* window, Camera* camera) {
    for (int i = 0; i < component->entities; i++) {
        EnemyComponent* enemy = EnemyComponent_get(component, i);
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


void damage(ComponentData* component, int entity, int dmg, int player) {
    EnemyComponent* enemy = component->enemy[entity];
    HealthComponent* health = component->health[entity];
    ParticleComponent* particle = component->particle[entity];

    health->health = max(0, health->health - dmg);

    if (enemy) {
        enemy->target = player;
        enemy->state = CHASE;
    }

    if (particle) {
        particle->enabled = true;
        if (dmg > 50) {
            particle->rate *= 5.0;
        }
    }

    int j = create_entity(component);
    CoordinateComponent_add(component, j, get_position(component, entity), rand_angle());

    if (dmg < 50) {
        ImageComponent_add(component, j, "blood", 1.0, 1.0, 1);
    } else {
        ImageComponent_add(component, j, "blood_large", 2.0, 2.0, 1);
    }
}
