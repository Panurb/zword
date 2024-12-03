#include <math.h>
#include <string.h>

#include "component.h"
#include "physics.h"
#include "health.h"
#include "image.h"
#include "animation.h"
#include "particle.h"
#include "sound.h"
#include "enemy.h"
#include "player.h"
#include "item.h"
#include "weapon.h"


void die(int entity) {
    HealthComponent* health = HealthComponent_get(entity);

    if (AnimationComponent_get(entity)) {
        stop_animation(entity);
    }

    change_texture(entity, health->dead_image, 0.0f, 0.0f);
    change_layer(entity, LAYER_CORPSES);

    CoordinateComponent* coord = CoordinateComponent_get(entity);
    EnemyComponent* enemy = EnemyComponent_get(entity);
    PlayerComponent* player = PlayerComponent_get(entity);
    ParticleComponent* particle = ParticleComponent_get(entity);

    if (player) {
        player_die(entity);
    } else {
        if (enemy) {
            enemy_die(entity);
        }

        for (ListNode* node = coord->children->head; node; node = node->next) {
            int i = node->value;
            CoordinateComponent* co = CoordinateComponent_get(i);
            ImageComponent* img = ImageComponent_get(i);
            PhysicsComponent* phys = PhysicsComponent_get(i);
            ColliderComponent* col = ColliderComponent_get(i);

            if (img) {
                img->stretch_speed = randf(-1.0f, 1.0f) * 0.05f;
                img->alpha = 1.0f;
            }

            co->position = get_position(i);
            co->previous.position = co->position;
            co->angle = get_angle(i);
            co->previous.angle = co->angle;

            if (phys) {
                Vector2f r = normalized(diff(co->position, get_position(entity)));
                if (!non_zero(r)) {
                    r = rand_vector();
                }
                apply_force(i, mult(150.0f, r));
                phys->angular_velocity = randf(-10.0f, 10.0f);
            }

            if (col) {
                col->enabled = true;
            }
        }
        remove_children(entity);

        // Enemy retains collision after dying until stopping
        if (!enemy) {
            clear_grid(entity);
            ColliderComponent_remove(entity);
        }
    }

    if (SoundComponent_get(entity) && health->die_sound[0] != '\0') {
        add_sound(entity, health->die_sound, 1.0f, randf(0.9f, 1.1f));
    }
}


void damage(int entity, Vector2f pos, Vector2f dir, int dmg, int dealer) {
    HealthComponent* health = HealthComponent_get(entity);
    EnemyComponent* enemy = EnemyComponent_get(entity);
    if (health) {
        int prev_health = health->health;
        health->health = health->health - dmg;

        if (health->decal[0] != '\0') {
            Vector2f pos = sum(get_position(entity), mult(0.5f, rand_vector()));
            int i = -1;
            if (dmg < 40) {
                i = create_decal(pos, health->decal, 60.0f);
            } else {
                Filename buffer;
                snprintf(buffer, sizeof(buffer), "%s_large", health->decal);
                i = create_decal(pos, buffer, 60.0f);
            }
            ImageComponent_get(i)->alpha = 0.8f;
        }

        if (prev_health > 0 && health->health <= 0) {
            PlayerComponent* player = PlayerComponent_get(dealer);
            if (player && enemy) {
                add_money(dealer, enemy->bounty);
            }
        }
    }
    
    if (enemy) {
        enemy->desired_angle = polar_angle(mult(-1.0f, dir));
    }
    
    PhysicsComponent* physics = PhysicsComponent_get(entity);
    if (physics) {
        apply_force(entity, mult(clamp(10.0f * dmg, 100.0f, 500.0f), normalized(dir)));
    }

    ParticleComponent* particle = ParticleComponent_get(entity);
    if (particle) {
        particle->origin = diff(pos, get_position(entity));
        add_particles(entity, particle->rate / 50.0f * dmg);
    }

    SoundComponent* scomp = SoundComponent_get(entity);
    if (scomp && scomp->hit_sound[0] != '\0') {
        add_sound(entity, scomp->hit_sound, fminf(1.0f, dmg / 50.0f), randf(0.9f, 1.1f));
    }

    ImageComponent* image = ImageComponent_get(entity);
    if (image) {
        if (!strstr(image->filename, "tile")) {
            image->stretch_speed = sign(randf(-1.0f, 1.0f)) * 0.05f * dmg;
        }
    }
}


void blunt_damage(int entity, Vector2f vel) {
    HealthComponent* health = HealthComponent_get(entity);
    float v = norm(vel);
    if (health) {
        if (v > 10.0f) {
            if (health->health > 0) {
                SoundComponent* sound = SoundComponent_get(entity);
                if (sound) {
                    add_sound(entity, sound->hit_sound, 1.0, 0.8);
                }
                damage(entity, get_position(entity), vel, 100, -1);
            }
        }
    }

    SoundComponent* sound = SoundComponent_get(entity);
    if (sound) {
        if (v > 5.0f) {
            // add_sound(entity, sound->hit_sound, 0.5f, 1.0f);
        }
    }
}


void update_health(float time_step) {
    for (int i = 0; i < game_data->components->entities; i++) {
        HealthComponent* health = HealthComponent_get(i);
        if (health && health->health <= 0 && !health->dead) {
            die(i);
            health->dead = true;
        }
    }
}
