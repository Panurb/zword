#include <math.h>
#include <string.h>

#include "component.h"
#include "level.h"
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


void die(ComponentData* components, ColliderGrid* grid, int entity) {
    HealthComponent* health = components->health[entity];

    if (AnimationComponent_get(entity)) {
        stop_animation(components, entity);
    }

    change_texture(components, entity, health->dead_image, 0.0f, 0.0f);
    change_layer(components, entity, LAYER_CORPSES);

    CoordinateComponent* coord = CoordinateComponent_get(entity);
    EnemyComponent* enemy = EnemyComponent_get(entity);
    PlayerComponent* player = PlayerComponent_get(entity);

    if (player) {
        float angle = coord->angle;
        for (int j = 0; j < player->inventory_size; j++) {
            if (player->inventory[j] != -1) {
                coord->angle = rand_angle();
                drop_item(components, entity);
            }
        }
        coord->angle = angle;

        for (int j = 1; j < player->ammo_size; j++) {
            AmmoComponent* ammo = AmmoComponent_get(player->ammo[j]);
            while (ammo && ammo->size > 0) {
                sfVector2f pos = get_position(entity);
                int k = create_ammo(components, sum(pos, polar_to_cartesian(1.0f, rand_angle())), ammo->type);
                int size = min(AmmoComponent_get(k)->size, ammo->size);
                AmmoComponent* drop = AmmoComponent_get(k);
                drop->size = size;
                ammo->size -= size;
            }
        }

        player->state = PLAYER_DEAD;
        WaypointComponent_remove(entity);
    } else {
        if (enemy) {
            coord->lifetime = 30.0f;

            enemy->state = ENEMY_DEAD;
            List_clear(enemy->path);
            LightComponent_remove(entity);
            destroy_entity(enemy->weapon);
            List_remove(coord->children, enemy->weapon);
            enemy->weapon = -1;
            WaypointComponent_remove(entity);

            float probs[5] = { 0.1f, 0.15f, 0.15f, 0.1f, 0.5f };
            int j = -1;
            switch (rand_choice(probs, 5)) {
                case 0:
                    j = create_bandage(components, zeros());
                    break;
                case 1:
                    j = create_ammo(components, zeros(), AMMO_PISTOL);
                    break;
                case 2:
                    j = create_ammo(components, zeros(), AMMO_SHOTGUN);
                    break;
                case 3:
                    j = create_ammo(components, zeros(), AMMO_RIFLE);
                    break;
                default:
                    break;
            }
            if (j != -1) {
                add_child(entity, j);
            }
        }

        for (ListNode* node = coord->children->head; node; node = node->next) {
            int i = node->value;
            CoordinateComponent* co = CoordinateComponent_get(i);
            ImageComponent_get(i)->alpha = 1.0f;

            co->position = get_position(i);
            co->angle = get_angle(i);

            sfVector2f r = normalized(diff(co->position, get_position(entity)));
            if (!non_zero(r)) {
                r = rand_vector();
            }
            apply_force(components, i, mult(150.0f, r));
            PhysicsComponent_get(i)->angular_velocity = randf(-5.0f, 5.0f);
            ColliderComponent_get(i)->enabled = true;
        }
        remove_children(entity);

        if (!enemy) {
            clear_grid(entity);
            ColliderComponent_remove(entity);
        }
    }

    if (SoundComponent_get(entity) && health->die_sound[0] != '\0') {
        add_sound(components, entity, health->die_sound, 1.0f, randf(0.9f, 1.1f));
    }
}


void damage(ComponentData* components, ColliderGrid* grid, int entity, sfVector2f pos, sfVector2f dir, int dmg, 
        int dealer) {
    HealthComponent* health = components->health[entity];
    EnemyComponent* enemy = components->enemy[entity];
    if (health) {
        int prev_health = health->health;
        health->health = max(0, health->health - dmg);

        if (health->decal[0] != '\0') {
            // TODO: use decal
            sfVector2f pos = sum(get_position(entity), mult(0.5f, rand_vector()));
            if (dmg < 40) {
                create_decal(components, pos, "blood", 60.0f);
            } else {
                create_decal(components, pos, "blood_large", 60.0f);
            }
        }

        if (prev_health > 0 && health->health == 0) {
            die(components, grid, entity);

            PlayerComponent* player = PlayerComponent_get(dealer);
            if (player && enemy) {
                add_money(components, dealer, enemy->bounty);
            }
        }
    }
    
    if (enemy) {
        enemy->desired_angle = polar_angle(mult(-1.0f, dir));
    }
    
    PhysicsComponent* physics = PhysicsComponent_get(entity);
    if (physics) {
        apply_force(components, entity, mult(fminf(10.0f * dmg, 500.0f), normalized(dir)));
    }

    ParticleComponent* particle = ParticleComponent_get(entity);
    if (particle) {
        particle->origin = diff(pos, get_position(entity));
        add_particles(components, entity, particle->rate / 50.0f * dmg);
    }

    SoundComponent* scomp = SoundComponent_get(entity);
    if (scomp && scomp->hit_sound[0] != '\0') {
        add_sound(components, entity, scomp->hit_sound, fminf(1.0f, dmg / 50.0f), randf(0.9f, 1.1f));
    }

    ImageComponent* image = ImageComponent_get(entity);
    if (image) {
        if (!strstr(image->filename, "tile")) {
            image->stretch_speed = randf(-1.0f, 1.0f) * 0.05f * dmg;
        }
    }
}


void blunt_damage(ComponentData* components, ColliderGrid* grid, int entity, sfVector2f vel) {
    HealthComponent* health = components->health[entity];
    float v = norm(vel);
    if (health) {
        if (v > 10.0f) {
            if (health->health > 0) {
                SoundComponent* sound = SoundComponent_get(entity);
                if (sound) {
                    add_sound(components, entity, sound->hit_sound, 1.0, 0.8);
                }
                damage(components, grid, entity, get_position(entity), vel, 100, -1);
            }
        }
    }

    SoundComponent* sound = SoundComponent_get(entity);
    if (sound) {
        if (v > 5.0f) {
            // add_sound(components, entity, sound->hit_sound, 0.5f, 1.0f);
        }
    }
}
