#define _USE_MATH_DEFINES

#include <math.h>

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
#include "game.h"
#include "list.h"
#include "item.h"


int create_zombie(Vector2f pos, float angle) {
    int i = create_entity();
    
    angle = rand_angle();
    CoordinateComponent_add(i, pos, angle);
    ColliderComponent_add_circle(i, 0.5, GROUP_ENEMIES);

    ImageComponent_add(i, "zombie", 1.0, 1.0, LAYER_ENEMIES);
    PhysicsComponent_add(i, 1.0f);
    EnemyComponent* enemy = EnemyComponent_add(i);
    ParticleComponent_add_type(i, PARTICLE_BLOOD, 0.0f);
    WaypointComponent_add(i);
    HealthComponent_add(i, 100, "zombie_dead", "blood", "");
    SoundComponent_add(i, "squish");

    Vector2f r = { 0.5f, 0.0f };
    int j = create_entity();
    CoordinateComponent_add(j, r, 0.0f);
    WeaponComponent_add(j, 0.5f, 25, 7, 0.35f * M_PI, -1, 0.0f, 1.0f, 0.0f, AMMO_MELEE, "axe");
    add_child(i, j);
    enemy->weapon = j;

    return i;
}


int create_farmer(Vector2f pos, float angle) {
    int i = create_entity();
    
    angle = rand_angle();
    CoordinateComponent_add(i, pos, angle);
    ColliderComponent_add_circle(i, 0.5, GROUP_ENEMIES);

    ImageComponent_add(i, "farmer", 1.0, 1.0, LAYER_ENEMIES);
    PhysicsComponent_add(i, 1.0f);
    EnemyComponent* enemy = EnemyComponent_add(i);
    enemy->walk_speed = 3.0f;
    enemy->run_speed = 3.0f;
    enemy->fov = 0.25f * M_PI;
    enemy->bounty = 200;
    ParticleComponent_add_type(i, PARTICLE_BLOOD, 0.0f);
    WaypointComponent_add(i);
    HealthComponent_add(i, 100, "farmer_dead", "blood", "");
    SoundComponent_add(i, "squish");
    LightComponent_add(i, 15.0f, 0.25f * M_PI, COLOR_WHITE, 0.5f, 10.0f);

    Vector2f r = { 0.75f, 0.0f };
    int j = create_rifle(r);
    ColliderComponent_remove(j);
    CoordinateComponent_get(j)->angle = 0.0f;
    add_child(i, j);
    enemy->weapon = j;

    return i;
}


int create_priest(Vector2f pos, float angle) {
    int i = create_entity();
    
    angle = rand_angle();
    CoordinateComponent_add(i, pos, angle);
    ColliderComponent_add_circle(i, 0.5, GROUP_ENEMIES);

    ImageComponent_add(i, "priest", 1.0, 1.0, LAYER_ENEMIES);
    PhysicsComponent_add(i, 1.0f);
    EnemyComponent* enemy = EnemyComponent_add(i);
    enemy->idle_speed = 0.0f;
    enemy->walk_speed = 2.0f;
    enemy->run_speed = 2.0f;
    enemy->fov = 2.0f * M_PI;
    enemy->bounty = 500;
    ParticleComponent_add_type(i, PARTICLE_BLOOD, 0.0f);
    WaypointComponent_add(i);
    HealthComponent_add(i, 200, "priest_dead", "blood", "");
    SoundComponent_add(i, "squish");
    LightComponent_add(i, 2.0f, 2.0f * M_PI, get_color(0.5f, 1.0f, 0.0f, 1.0f), 0.5f, 1.0f)->flicker = 0.25f;

    Vector2f r = { 0.5f, 0.0f };
    int j = create_entity();
    CoordinateComponent_add(j, r, 0.0f);
    WeaponComponent_add(j, 0.25f, 20, 5, 0.25f * M_PI, -1, 0.0f, 15.0f, 0.0f, AMMO_ENERGY, "energy");
    SoundComponent_add(j, "");
    add_child(i, j);
    enemy->weapon = j;

    return i;
}


int create_big_boy(Vector2f pos, float angle) {
    int i = create_entity();
    
    angle = rand_angle();
    CoordinateComponent_add(i, pos, angle);
    ColliderComponent_add_circle(i, 0.9f, GROUP_ENEMIES);

    ImageComponent_add(i, "big_boy", 4.0f, 2.0f, LAYER_ENEMIES);
    AnimationComponent_add(i, 2);
    PhysicsComponent* physics = PhysicsComponent_add(i, 10.0f);
    physics->drag_sideways = 20.0f;
    EnemyComponent* enemy = EnemyComponent_add(i);
    enemy->idle_speed = 0.0f;
    enemy->walk_speed = 4.0f;
    enemy->run_speed = 8.0f;
    enemy->bounty = 1000;
    ParticleComponent_add_type(i, PARTICLE_BLOOD, 0.0f);
    WaypointComponent_add(i);
    HealthComponent_add(i, 500, "big_boy_dead", "blood", "");
    SoundComponent_add(i, "squish");

    Vector2f r = { 0.5f, 0.0f };
    int j = create_entity();
    CoordinateComponent_add(j, r, 0.0f);
    WeaponComponent_add(j, 0.5f, 100, 7, 0.5f * M_PI, -1, 0.0f, 1.0f, 0.0f, AMMO_MELEE, "axe");
    add_child(i, j);
    enemy->weapon = j;

    return i;
}


int create_boss(Vector2f pos, float angle) {
    int i = create_entity();
    
    CoordinateComponent_add(i, pos, angle);
    ColliderComponent_add_circle(i, 1.0f, GROUP_ENEMIES);
    ImageComponent_add(i, "boss_head", 6.0f, 4.0f, LAYER_ENEMIES);
    AnimationComponent_add(i, 1);
    PhysicsComponent_add(i, 100.0f);
    EnemyComponent* enemy = EnemyComponent_add(i);
    enemy->fov = 2.0f * M_PI;
    enemy->idle_speed = 0.0f;
    enemy->walk_speed = 2.5f;
    enemy->run_speed = 6.0f;
    enemy->turn_speed = 2.5f;
    enemy->attack_delay = 0.0f;
    enemy->attack_timer = 0.0f;
    ParticleComponent_add_type(i, PARTICLE_BLOOD, 0.0f);
    WaypointComponent_add(i);
    HealthComponent_add(i, 5000, "boss_dead", "blood", "stone_hit");
    SoundComponent_add(i, "squish");
    LightComponent_add(i, 5.0f, 2.0f * M_PI, COLOR_ENERGY, 0.5f, 1.0f)->flicker = 0.1f;

    Vector2f r = { 0.5f, 0.0f };
    int j = create_entity();
    CoordinateComponent_add(j, r, 0.0f);
    WeaponComponent_add(j, 5.0f, 50, 7, M_PI, -1, 0.0f, 3.0f, 0.0f, AMMO_MELEE, "axe");
    add_child(i, j);
    enemy->weapon = j;

    int parent = i;
    for (int k = 0; k < 5; k++) {
        j = create_entity();
        pos = sum(pos, polar_to_cartesian(3.0f, angle));
        CoordinateComponent_add(j, pos, 0.0f);
        PhysicsComponent_add(j, 10.0f);
        ColliderComponent_add_rectangle(j, 3.0f, 1.5f, GROUP_ENEMIES);
        ImageComponent_add(j, "boss_body", 6.0f, 4.0f, LAYER_ENEMIES);
        AnimationComponent_add(j, 4)->current_frame = 2 * (k % 2);
        JointComponent_add(j, parent, 3.0f, 3.0f, 1.0f);
        ParticleComponent_add_type(parent, PARTICLE_SPARKS, 0.0f);
        SoundComponent_add(j, "stone_hit");
        parent = j;
    }

    return i;
}


void update_vision(int entity) {
    EnemyComponent* enemy = EnemyComponent_get(entity);
    float min_dist = enemy->vision_range;
    ListNode* node;
    FOREACH(node, game_data->components->player.order) {
        int j = node->value;
        PlayerComponent* player = PlayerComponent_get(j);
        if (player->state == PLAYER_DEAD) continue;

        Vector2f pos = get_position(entity);

        Vector2f r = diff(get_position(j), pos);
        Vector2f s = polar_to_cartesian(1.0f, get_angle(entity));
        float angle = fabs(signed_angle(r, s));

        float d = norm(r);
        if (d < min_dist && angle < 0.5f * enemy->fov) {
            HitInfo info = raycast(pos, r, enemy->vision_range, GROUP_VISION);
            if (info.entity == j) {
                enemy->target = j;
                enemy->state = ENEMY_CHASE;
                min_dist = d;
            }
        }
    }
}


void update_enemies(float time_step) {
    for (int i = 0; i < game_data->components->entities; i++) {
        EnemyComponent* enemy = EnemyComponent_get(i);
        if (!enemy) continue;
        if (enemy->spawner) continue;

        CoordinateComponent* coord = CoordinateComponent_get(i);
        PhysicsComponent* phys = PhysicsComponent_get(i);
        WeaponComponent* weapon = WeaponComponent_get(enemy->weapon);
        Vector2f pos = get_position(i);

        if (enemy->state != ENEMY_ATTACK && enemy->state != ENEMY_DEAD) {
            update_vision(i);
            float delta_angle = angle_diff(enemy->desired_angle, coord->angle);
            phys->angular_velocity = enemy->turn_speed * delta_angle;
        }

        switch (enemy->state) {
            case ENEMY_IDLE: {
                enemy->desired_angle = get_angle(i);
                break;
            } case ENEMY_WANDER: {
                Vector2f r = polar_to_cartesian(1.0f, enemy->desired_angle);
                HitInfo info = raycast(get_position(i), r, 2.0f, GROUP_ENEMIES);
                if (info.entity != -1) {
                    enemy->desired_angle = mod(enemy->desired_angle - 0.1f * sign(signed_angle(info.normal, r)), 2.0f * M_PI);
                    if (phys->speed < 0.5f) {
                        enemy->desired_angle = mod(enemy->desired_angle + M_PI, 2.0f * M_PI);
                    }
                }

                if (phys->speed < enemy->idle_speed) {
                    enemy->desired_angle = mod(enemy->desired_angle + randf(-0.05f, 0.05f), 2.0f * M_PI);
                    Vector2f a = polar_to_cartesian(enemy->acceleration, coord->angle);
                    phys->acceleration = sum(phys->acceleration, a);
                }
                
                break;
            } case ENEMY_INVESTIGATE: {
                a_star(i, enemy->target, enemy->path);

                Vector2f r;
                if (enemy->path->size > 1) {
                    r = diff(get_position(enemy->path->head->next->value), get_position(i));
                } else {
                    r = diff(get_position(enemy->target), get_position(i));
                }

                if (phys->speed < enemy->walk_speed) {
                    phys->acceleration = sum(phys->acceleration, mult(enemy->acceleration, normalized(r)));
                }

                enemy->desired_angle = polar_angle(r);

                break;
            } case ENEMY_CHASE: {
                Vector2f r = diff(get_position(enemy->target), pos);
                float d = norm(r);
                Vector2f v = r;

                HitInfo info = raycast(pos, r, d, GROUP_BULLETS);
                if (info.entity != enemy->target) {
                    a_star(i, enemy->target, enemy->path);
                    if (enemy->path->size > 1) {
                        v = diff(get_position(enemy->path->head->next->value), pos);
                    }
                }

                enemy->desired_angle = polar_angle(v);

                r = polar_to_cartesian(1.0f, get_angle(i));
                info = raycast(pos, r, fminf(weapon->range, enemy->vision_range), GROUP_BULLETS);
                if (info.entity == enemy->target) {
                    enemy->attack_timer = enemy->attack_delay;
                    enemy->state = ENEMY_ATTACK;
                } else {
                    for (int j = 0; j < weapon->shots; j++) {
                        float angle = j * weapon->spread / (weapon->shots - 1) - 0.5f * weapon->spread;
                        Vector2f dir = polar_to_cartesian(1.0, get_angle(i) + angle);
                        HitInfo info = raycast(pos, dir, weapon->range, GROUP_BULLETS);

                        if (HealthComponent_get(info.entity) && 
                                !EnemyComponent_get(info.entity)) {
                            enemy->state = ENEMY_ATTACK;
                            break;
                        }
                    }

                    if (phys->speed < enemy->run_speed) {
                        phys->acceleration = sum(phys->acceleration, mult(enemy->acceleration, normalized(v)));
                    }
                }

                if (PlayerComponent_get(enemy->target)->state == PLAYER_DEAD) {
                    enemy->target = -1;
                    enemy->state = ENEMY_IDLE;
                }

                break;
            } case ENEMY_ATTACK: {
                if (enemy->attack_timer <= 0.0f) {
                    attack(enemy->weapon);
                    enemy->state = ENEMY_CHASE;
                } else {
                    enemy->attack_timer -= time_step;
                }

                break;
            } case ENEMY_DEAD: {
                ColliderComponent* col = ColliderComponent_get(i);
                if (col) {
                    col->group = GROUP_DEBRIS;
                    if (phys->speed == 0.0f) {
                        clear_grid(i);
                        ColliderComponent_remove(i);
                    }
                }

                break;
            }
        }
    }
}


void draw_enemies(int camera) {
    for (int i = 0; i < game_data->components->entities; i++) {
        EnemyComponent* enemy = EnemyComponent_get(i);
        if (!enemy) continue;

        for (ListNode* current = enemy->path->head; current; current = current->next) {
            if (current->next) {
                Vector2f start = get_position(current->value);
                Vector2f end = get_position(current->next->value);
                draw_line(camera, start, end, 0.05, COLOR_RED);
            }
        }

        HealthComponent* health = HealthComponent_get(i);
        if (health) {
            String buffer;
            snprintf(buffer, sizeof(buffer), "%d", health->status.type);
            draw_text(camera, get_position(i), buffer, 20, COLOR_WHITE);
        }
    }
}


void alert_enemies(int player, float range) {
    List* list = get_entities(get_position(player), range);
    for (ListNode* node = list->head; node; node = node->next) {
        int j = node->value;

        EnemyComponent* enemy = EnemyComponent_get(j);
        if (enemy && enemy->state != ENEMY_DEAD && enemy->state != ENEMY_CHASE) {
            enemy->target = player;
            enemy->state = ENEMY_INVESTIGATE;
        }
    }
    List_delete(list);
}


void drop_loot(int entity) {
    float probs[5] = { 0.1f, 0.1f, 0.1f, 0.1f, 1.0f };

    ListNode* node;
    FOREACH(node, game_data->components->player.order) {
        int i = node->value;

        PlayerComponent* player = PlayerComponent_get(i);
        HealthComponent* health = HealthComponent_get(i);

        int ammos[4] = { 0, 0, 0, 0 };

        for (int j = 0; j < player->ammo_size; j++) {
            if (player->ammo[j] == -1) continue;
            int ammo_type = AmmoComponent_get(player->ammo[j])->type;
            ammos[ammo_type] += AmmoComponent_get(player->ammo[j])->size;
        }

        int bandages = 0;
        for (int j = 0; j < player->inventory_size; j++) {
            WeaponComponent* weapon = WeaponComponent_get(player->inventory[j]);
            if (weapon && weapon->ammo_type != AMMO_MELEE) {
                int ammo = ammos[weapon->ammo_type - 1];

                if (ammo < 2 * weapon->max_magazine) {
                    probs[weapon->ammo_type] += 1.0f;
                }
            }

            ItemComponent* item = ItemComponent_get(player->inventory[j]);
            if (item && item->type == ITEM_HEAL) {
                bandages++;
            }
        }

        if (bandages == 0 && health->health < health->max_health) {
            probs[0] += 0.5f;
        }
    }

    int j = -1;
    switch (rand_choice(probs, 5)) {
        case 0:
            j = create_bandage(zeros());
            break;
        case 1:
            j = create_ammo(zeros(), AMMO_PISTOL);
            break;
        case 2:
            j = create_ammo(zeros(), AMMO_RIFLE);
            break;
        case 3:
            j = create_ammo(zeros(), AMMO_SHOTGUN);
            break;
        default:
            break;
    }
    if (j != -1) {
        add_child(entity, j);
    }
}


void enemy_die(int entity) {
    CoordinateComponent* coord = CoordinateComponent_get(entity);
    EnemyComponent* enemy = EnemyComponent_get(entity);
    HealthComponent* health = HealthComponent_get(entity);
    ParticleComponent* particle = ParticleComponent_get(entity);

    coord->lifetime = 30.0f;

    enemy->state = ENEMY_DEAD;
    List_clear(enemy->path);
    LightComponent_remove(entity);
    destroy_entity(enemy->weapon);
    List_remove(coord->children, enemy->weapon);
    enemy->weapon = -1;

    // Overkill
    if (health->health < -health->max_health) {
        ImageComponent_remove(entity);
        if (particle) {
            particle->origin = zeros();
            add_particles(entity, 100);
        }
    }

    WaypointComponent_remove(entity);

    if (game_data->game_mode == MODE_SURVIVAL) {
        drop_loot(entity);
    }
}


void create_spawner(Vector2f position, float angle, float width, float height) {
    int i = create_entity();
    CoordinateComponent_add(i, position, angle);
    ColliderComponent_add_rectangle(i, width, height, GROUP_FLOORS);
    EnemyComponent_add(i)->spawner = true;
}


void draw_spawners() {
    for (int i = 0; i <= game_data->components->entities; i++) {
        EnemyComponent* enemy = EnemyComponent_get(i);
        if (enemy && enemy->spawner) {
            ColliderComponent* col = ColliderComponent_get(i);
            Color color = get_color(1.0f, 0.0f, 1.0f, 0.25f);
            Vector2f pos = get_position(i);
            float angle = get_angle(i);
            draw_rectangle(game_data->camera, pos, collider_width(i), collider_height(i), angle, color);
            draw_text(game_data->camera, pos, "spawner", 20, COLOR_MAGENTA);
        }
    }
}


int spawn_enemy(Vector2f position, float probs[4]) {
    int j = -1;
    float angle = rand_angle();
    switch (rand_choice(probs, 4)) {
        case 0:
            j = create_zombie(position, angle);
            break;
        case 1:
            j = create_farmer(position, angle);
            break;
        case 2:
            j = create_big_boy(position, angle);
            break;
        case 3:
            j = create_priest(position, angle);
            break;
    }
    int p = game_data->components->player.order->head->value;
    EnemyComponent* enemy = EnemyComponent_get(j);
    if (enemy) {
        enemy->target = p;
        enemy->state = ENEMY_CHASE;
        // TODO: update grid?
    }

    return j;
}
