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
#include "health.h"
#include "weapon.h"


void create_zombie(ComponentData* components, ColliderGrid* grid, sfVector2f pos) {
    int i = create_entity(components);
    
    CoordinateComponent_add(components, i, pos, rand_angle());
    ColliderComponent_add_circle(components, i, 0.5, GROUP_ENEMIES);

    if (collides_with(components, grid, i)) {
        destroy_entity(components, i);
        return;
    }

    ImageComponent_add(components, i, "zombie", 1.0, 1.0, LAYER_ENEMIES);
    PhysicsComponent_add(components, i, 1.0f);
    EnemyComponent* enemy = EnemyComponent_add(components, i);
    ParticleComponent_add_blood(components, i);
    WaypointComponent_add(components, i);
    HealthComponent_add(components, i, 100, "zombie_dead", "blood", "");
    SoundComponent_add(components, i, "squish");

    sfVector2f r = { 0.5f, 0.0f };
    int j = create_entity(components);
    CoordinateComponent_add(components, j, r, 0.0f);
    WeaponComponent_add(components, j, 0.5f, 25, 7, 0.35f * M_PI, -1, 0.0f, 1.0f, 0.0f, AMMO_MELEE, "axe");
    add_child(components, i, j);
    enemy->weapon = j;
}


void create_farmer(ComponentData* components, ColliderGrid* grid, sfVector2f pos) {
    int i = create_entity(components);
    
    CoordinateComponent_add(components, i, pos, rand_angle());
    ColliderComponent_add_circle(components, i, 0.5, GROUP_ENEMIES);

    if (collides_with(components, grid, i)) {
        destroy_entity(components, i);
        return;
    }

    ImageComponent_add(components, i, "farmer", 1.0, 1.0, LAYER_ENEMIES);
    PhysicsComponent_add(components, i, 1.0f);
    EnemyComponent* enemy = EnemyComponent_add(components, i);
    enemy->walk_speed = 3.0f;
    enemy->run_speed = 3.0f;
    enemy->fov = 0.25f * M_PI;
    ParticleComponent_add_blood(components, i);
    WaypointComponent_add(components, i);
    HealthComponent_add(components, i, 100, "farmer_dead", "blood", "");
    SoundComponent_add(components, i, "squish");
    LightComponent_add(components, i, 15.0f, 0.25f * M_PI, sfWhite, 0.5f, 10.0f);

    sfVector2f r = { 0.75f, 0.0f };
    int j = create_rifle(components, r);
    ColliderComponent_remove(components, j);
    CoordinateComponent_get(components, j)->angle = 0.0f;
    add_child(components, i, j);
    enemy->weapon = j;
}


void create_priest(ComponentData* components, ColliderGrid* grid, sfVector2f pos) {
    int i = create_entity(components);
    
    CoordinateComponent_add(components, i, pos, rand_angle());
    ColliderComponent_add_circle(components, i, 0.5, GROUP_ENEMIES);

    if (collides_with(components, grid, i)) {
        destroy_entity(components, i);
        return;
    }

    ImageComponent_add(components, i, "priest", 1.0, 1.0, LAYER_ENEMIES);
    PhysicsComponent_add(components, i, 1.0f);
    EnemyComponent* enemy = EnemyComponent_add(components, i);
    enemy->idle_speed = 0.0f;
    enemy->walk_speed = 2.0f;
    enemy->run_speed = 2.0f;
    enemy->fov = 2.0f * M_PI;
    enemy->vision_range = 15.0f;
    ParticleComponent_add_blood(components, i);
    WaypointComponent_add(components, i);
    HealthComponent_add(components, i, 200, "priest_dead", "blood", "");
    SoundComponent_add(components, i, "squish");
    LightComponent_add(components, i, 2.0f, 2.0f * M_PI, get_color(0.5f, 1.0f, 0.0f, 1.0f), 0.5f, 1.0f)->flicker = 0.25f;

    sfVector2f r = { 0.5f, 0.0f };
    int j = create_entity(components);
    CoordinateComponent_add(components, j, r, 0.0f);
    WeaponComponent_add(components, j, 0.25f, 20, 5, 0.25f * M_PI, -1, 0.0f, 15.0f, 0.0f, AMMO_ENERGY, "energy");
    SoundComponent_add(components, j, "");
    add_child(components, i, j);
    enemy->weapon = j;
}


void create_big_boy(ComponentData* components, ColliderGrid* grid, sfVector2f pos) {
    int i = create_entity(components);
    
    CoordinateComponent_add(components, i, pos, rand_angle());
    ColliderComponent_add_circle(components, i, 0.9f, GROUP_ENEMIES);

    if (collides_with(components, grid, i)) {
        destroy_entity(components, i);
        return;
    }

    ImageComponent_add(components, i, "big_boy", 4.0f, 2.0f, LAYER_ENEMIES);
    AnimationComponent_add(components, i);
    PhysicsComponent* physics = PhysicsComponent_add(components, i, 10.0f);
    physics->drag_sideways = 20.0f;
    EnemyComponent* enemy = EnemyComponent_add(components, i);
    enemy->idle_speed = 0.0f;
    enemy->walk_speed = 4.0f;
    enemy->run_speed = 8.0f;
    ParticleComponent_add_blood(components, i);
    WaypointComponent_add(components, i);
    HealthComponent_add(components, i, 500, "big_boy_dead", "blood", "");
    SoundComponent_add(components, i, "squish");

    sfVector2f r = { 0.5f, 0.0f };
    int j = create_entity(components);
    CoordinateComponent_add(components, j, r, 0.0f);
    WeaponComponent_add(components, j, 0.5f, 100, 7, 0.5f * M_PI, -1, 0.0f, 1.0f, 0.0f, AMMO_MELEE, "axe");
    add_child(components, i, j);
    enemy->weapon = j;
}


void update_vision(ComponentData* components, ColliderGrid* grid, int entity) {
    EnemyComponent* enemy = EnemyComponent_get(components, entity);
    float min_dist = enemy->vision_range;
    for (ListNode* node = components->player.order->head; node; node = node->next) {
        int j = node->value;
        PlayerComponent* player = PlayerComponent_get(components, j);
        if (player->state == PLAYER_DEAD) continue;

        sfVector2f r = diff(get_position(components, j), get_position(components, entity));
        sfVector2f s = polar_to_cartesian(1.0f, get_angle(components, entity));
        float angle = fabs(signed_angle(r, s));

        float d = norm(r);
        if (d < min_dist && angle < 0.5f * enemy->fov) {
            HitInfo info = raycast(components, grid, get_position(components, entity), r, enemy->vision_range, GROUP_BULLETS);
            if (info.entity == j) {
                enemy->target = j;
                enemy->state = ENEMY_CHASE;
                min_dist = d;
            }
        }
    }
}


void update_enemies(ComponentData* components, ColliderGrid* grid, float time_step) {
    for (int i = 0; i < components->entities; i++) {
        EnemyComponent* enemy = EnemyComponent_get(components, i);
        if (!enemy) continue;

        CoordinateComponent* coord = CoordinateComponent_get(components, i);
        PhysicsComponent* phys = PhysicsComponent_get(components, i);
        WeaponComponent* weapon = WeaponComponent_get(components, enemy->weapon);
        sfVector2f pos = get_position(components, i);

        if (enemy->state != ENEMY_ATTACK && enemy->state != ENEMY_DEAD) {
            update_vision(components, grid, i);
            float delta_angle = mod(enemy->desired_angle + M_PI - coord->angle, 2.0f * M_PI) - M_PI;
            phys->angular_velocity = 5.0f * delta_angle;

            AnimationComponent* animation = AnimationComponent_get(components, i);
            if (animation) {
                animation->framerate = phys->speed;
            }
        }

        switch (enemy->state) {
            case ENEMY_IDLE: {
                sfVector2f r = polar_to_cartesian(1.0f, enemy->desired_angle);
                HitInfo info = raycast(components, grid, get_position(components, i), r, 2.0f, GROUP_ENEMIES);
                if (info.entity != -1) {
                    enemy->desired_angle = mod(enemy->desired_angle - 0.1f * sign(signed_angle(info.normal, r)), 2.0f * M_PI);
                    if (phys->speed < 0.5f) {
                        enemy->desired_angle = mod(enemy->desired_angle + M_PI, 2.0f * M_PI);
                    }
                }

                if (phys->speed < enemy->idle_speed) {
                    enemy->desired_angle = mod(enemy->desired_angle + randf(-0.05f, 0.05f), 2.0f * M_PI);
                    sfVector2f a = polar_to_cartesian(enemy->acceleration, coord->angle);
                    phys->acceleration = sum(phys->acceleration, a);
                }

                break;
            } case ENEMY_INVESTIGATE: {
                a_star(components, i, enemy->target, enemy->path);

                sfVector2f r;
                if (enemy->path->size > 1) {
                    r = diff(get_position(components, enemy->path->head->next->value), get_position(components, i));
                } else {
                    r = diff(get_position(components, enemy->target), get_position(components, i));
                }

                if (phys->speed < enemy->walk_speed) {
                    phys->acceleration = sum(phys->acceleration, mult(enemy->acceleration, normalized(r)));
                }

                enemy->desired_angle = polar_angle(r);

                break;
            } case ENEMY_CHASE: {
                sfVector2f r = diff(get_position(components, enemy->target), pos);
                float d = norm(r);
                sfVector2f v = r;

                HitInfo info = raycast(components, grid, pos, r, d, GROUP_BULLETS);
                if (info.entity != enemy->target) {
                    a_star(components, i, enemy->target, enemy->path);
                    if (enemy->path->size > 1) {
                        v = diff(get_position(components, enemy->path->head->next->value), pos);
                    }
                }

                enemy->desired_angle = polar_angle(v);

                if (d > 2.0f * enemy->vision_range) {
                    enemy->state = ENEMY_IDLE;
                    break;
                }

                r = polar_to_cartesian(1.0f, get_angle(components, i));
                info = raycast(components, grid, pos, r, fminf(weapon->range, enemy->vision_range), GROUP_BULLETS);
                if (info.entity == enemy->target) {
                    enemy->attack_timer = enemy->attack_delay;
                    enemy->state = ENEMY_ATTACK;
                } else {
                    if (phys->speed < enemy->run_speed) {
                        phys->acceleration = sum(phys->acceleration, mult(enemy->acceleration, normalized(v)));
                    }
                }

                if (PlayerComponent_get(components, enemy->target)->state == PLAYER_DEAD) {
                    enemy->target = -1;
                    enemy->state = ENEMY_IDLE;
                }

                break;
            } case ENEMY_ATTACK: {
                if (enemy->attack_timer <= 0.0f) {
                    shoot(components, grid, enemy->weapon);
                    enemy->state = ENEMY_CHASE;
                } else {
                    enemy->attack_timer -= time_step;
                }

                break;
            } case ENEMY_DEAD: {
                ColliderComponent* col = ColliderComponent_get(components, i);
                if (col) {
                    col->group = GROUP_CORPSES;
                    if (phys->speed == 0.0f) {
                        clear_grid(components, grid, i);
                        // ColliderComponent_remove(components, i);
                    }
                }

                WaypointComponent_remove(components, i);

                break;
            }
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
    }
}


void alert_enemies(ComponentData* components, ColliderGrid* grid, int player, float range) {
    List* list = get_entities(components, grid, get_position(components, player), range);
    for (ListNode* current = list->head; current; current = current->next) {
        int j = current->value;
        if (j == -1) break;

        EnemyComponent* enemy = EnemyComponent_get(components, j);
        if (enemy && enemy->state != ENEMY_DEAD && enemy->state != ENEMY_CHASE) {
            enemy->target = player;
            enemy->state = ENEMY_INVESTIGATE;
        }
    }
    List_delete(list);
}
