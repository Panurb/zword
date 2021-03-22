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
#include "particle.h"
#include "sound.h"


void create_enemy(ComponentData* components, sfVector2f pos) {
    int i = create_entity(components);

    float angle = rand_angle();
    
    CoordinateComponent_add(components, i, pos, angle);
    ImageComponent_add(components, i, "zombie", 1.0, 1.0, 4)->shine = 0.5;
    ColliderComponent_add_circle(components, i, 0.5, ENEMIES);
    PhysicsComponent_add(components, i, 1.0, 0.0, 0.5, 5.0, 10.0)->max_speed = 6.0;
    EnemyComponent_add(components, i);
    ParticleComponent_add_blood(components, i);
    WaypointComponent_add(components, i);
    HealthComponent_add(components, i, 100);
    SoundComponent_add(components, i, "squish");
}


void damage(ComponentData* components, int entity, int dmg, int player) {
    HealthComponent* health = components->health[entity];
    health->health = max(0, health->health - dmg);

    EnemyComponent* enemy = components->enemy[entity];
    if (enemy) {
        enemy->target = player;
        enemy->state = CHASE;
    }

    int j = create_entity(components);
    CoordinateComponent_add(components, j, get_position(components, entity), rand_angle());

    if (dmg < 50) {
        ImageComponent_add(components, j, "blood", 1.0, 1.0, 1);
    } else {
        ImageComponent_add(components, j, "blood_large", 2.0, 2.0, 1);
    }

    SoundComponent* scomp = SoundComponent_get(components, entity);
    if (scomp) {
        add_sound(components, entity, "squish", 0.5, randf(0.9, 1.1));
    }
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
                for (int k = 0; k < components->player.size; k++) {
                    int j = components->player.order[k];
                    PlayerComponent* player = PlayerComponent_get(components, j);
                    if (player->state == PLAYER_DEAD) continue;

                    sfVector2f r = diff(get_position(components, j), get_position(components, i));
                    float angle = mod(get_angle(components, i) - polar_angle(r), 2 * M_PI);

                    if (angle < 0.5 * enemy->fov) {
                        HitInfo info = raycast(components, grid, get_position(components, i), r, enemy->vision_range, i, BULLETS);
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
                a_star(components, enemy->target, i, enemy->path);

                sfVector2f r;
                if (enemy->path[1] != -1) {
                    r = diff(get_position(components, enemy->path[1]), get_position(components, i));
                } else {
                    r = diff(get_position(components, enemy->target), get_position(components, i));
                }

                float d = norm(r);

                if (d > 1.0) {
                    phys->acceleration = sum(phys->acceleration, mult(enemy->acceleration / d, r));
                }

                components->coordinate[i]->angle = polar_angle(r);

                if (dist(get_position(components, enemy->target), get_position(components, i)) < 1.0) {
                    damage(components, enemy->target, 50, -1);
                }

                if (PlayerComponent_get(components, enemy->target)->state == PLAYER_DEAD) {
                    enemy->target = -1;
                    enemy->state = IDLE;
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

                // FIXME
                // if (WaypointComponent_get(components, i)) {
                //     WaypointComponent_remove(components, i);
                // }

                strcpy(image->filename, "zombie_dead");
                image->width = 2.0;
                image->texture_changed = true;
                change_layer(components, i, 2);

                break;
        }
    }
}


void draw_enemies(ComponentData* components, sfRenderWindow* window, int camera) {
    for (int i = 0; i < components->entities; i++) {
        EnemyComponent* enemy = EnemyComponent_get(components, i);
        if (!enemy) continue;

        for (int j = 0; j < MAX_PATH_LENGTH; j++) {
            if (enemy->path[j + 1] == -1) break;

            draw_line(window, components, camera, NULL, get_position(components, enemy->path[j]), get_position(components, enemy->path[j + 1]), 0.05, sfRed);
        }

        if (enemy->path[1] != -1) {
            draw_circle(window, components, camera, NULL, get_position(components, enemy->path[1]), 0.1, sfGreen);
        }
    }
}
