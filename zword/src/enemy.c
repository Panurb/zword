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
#include "collider.h"
#include "animation.h"


void create_enemy(ComponentData* components, sfVector2f pos) {
    int i = create_entity(components);
    
    CoordinateComponent_add(components, i, pos, rand_angle());
    ImageComponent_add(components, i, "zombie", 1.0, 1.0, 4)->shine = 0.5;
    ColliderComponent_add_circle(components, i, 0.5, GROUP_ENEMIES);
    PhysicsComponent_add(components, i, 1.0, 0.0, 0.5, 5.0, 10.0)->max_speed = 6.0;
    EnemyComponent_add(components, i);
    ParticleComponent_add_blood(components, i);
    WaypointComponent_add(components, i);
    HealthComponent_add(components, i, 100);
    SoundComponent_add(components, i, "squish");
}


void create_big_boy(ComponentData* components, sfVector2f pos) {
    int i = create_entity(components);
    
    CoordinateComponent_add(components, i, pos, rand_angle());
    ImageComponent_add(components, i, "big_boy", 4.0f, 2.0f, 4);
    AnimationComponent_add(components, i);
    ColliderComponent_add_circle(components, i, 0.9f, GROUP_ENEMIES);
    PhysicsComponent_add(components, i, 10.0f, 0.0, 0.15f, 5.0, 10.0)->max_speed = 12.0f;
    EnemyComponent_add(components, i);
    ParticleComponent_add_blood(components, i);
    WaypointComponent_add(components, i);
    HealthComponent_add(components, i, 500);
    SoundComponent_add(components, i, "squish");
}


void update_enemies(ComponentData* components, ColliderGrid* grid) {
    for (int i = 0; i < components->entities; i++) {
        EnemyComponent* enemy = EnemyComponent_get(components, i);
        if (!enemy) continue;

        PhysicsComponent* phys = PhysicsComponent_get(components, i);
        ImageComponent* image = ImageComponent_get(components, i);

        if (enemy->state != DEAD && components->health[i]->health <= 0) {
            strcpy(image->filename, "zombie_dead");
            image->width = 2.0f;
            image->texture_changed = true;
            change_layer(components, i, 2);
            if (AnimationComponent_get(components, i)) {
                stop_animation(components, i);
            }
            enemy->state = DEAD;
        }

        switch (enemy->state) {
            case IDLE:
                for (ListNode* node = components->player.order->head; node; node = node->next) {
                    int j = node->value;
                    PlayerComponent* player = PlayerComponent_get(components, j);
                    if (player->state == PLAYER_DEAD) continue;

                    sfVector2f r = diff(get_position(components, j), get_position(components, i));
                    sfVector2f s = polar_to_cartesian(1.0f, get_angle(components, i));
                    float angle = acosf(dot(normalized(r), s));

                    if (norm(r) < enemy->vision_range && angle < 0.5f * enemy->fov) {
                        HitInfo info = raycast(components, grid, get_position(components, i), r, enemy->vision_range, i, GROUP_BULLETS);
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
                a_star(components, i, enemy->target, enemy->path);

                sfVector2f r;
                if (enemy->path->size > 1) {
                    r = diff(get_position(components, enemy->path->head->next->value), get_position(components, i));
                } else {
                    r = diff(get_position(components, enemy->target), get_position(components, i));
                }

                float d = norm(r);

                if (d > 1.0) {
                    phys->acceleration = sum(phys->acceleration, mult(enemy->acceleration / d, r));
                }

                components->coordinate[i]->angle = polar_angle(r);

                if (dist(get_position(components, enemy->target), get_position(components, i)) < 1.0) {
                    damage(components, enemy->target, get_position(components, enemy->target), r, 50);
                    ParticleComponent_get(components, enemy->target)->enabled = true;
                }

                if (PlayerComponent_get(components, enemy->target)->state == PLAYER_DEAD) {
                    enemy->target = -1;
                    enemy->state = IDLE;
                }

                break;
            case DEAD:;
                ColliderComponent* col = ColliderComponent_get(components, i);
                if (col) {
                    col->group = GROUP_CORPSES;
                    if (phys->speed == 0.0f) {
                        clear_grid(components, grid, i);
                        ColliderComponent_remove(components, i);
                    }
                }

                WaypointComponent_remove(components, i);

                break;
        }
    }
}


void draw_enemies(ComponentData* components, sfRenderWindow* window, int camera) {
    for (int i = 0; i < components->entities; i++) {
        EnemyComponent* enemy = EnemyComponent_get(components, i);
        if (!enemy) continue;

        for (ListNode* current = enemy->path->head; current; current = current->next) {
            if (current->next) {
                sfVector2f start = get_position(components, current->value);
                sfVector2f end = get_position(components, current->next->value);
                draw_line(window, components, camera, NULL, start, end, 0.05, sfRed);
            }
        }

        // if (enemy->path[1] != -1) {
        //     draw_circle(window, components, camera, NULL, get_position(components, enemy->path[1]), 0.1, sfGreen);
        // }
    }
}


void alert_enemies(ComponentData* components, ColliderGrid* grid, int player, float range) {
    List* list = get_entities(components, grid, get_position(components, player), range);
    for (ListNode* current = list->head; current; current = current->next) {
        int j = current->value;
        if (j == -1) break;

        EnemyComponent* enemy = EnemyComponent_get(components, j);
        if (enemy) {
            if (enemy->target == -1) {
                enemy->target = player;
                enemy->state = CHASE;
            }
        }
    }
    List_delete(list);
}
