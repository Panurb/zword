#include <math.h>

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


void die(ComponentData* components, int entity) {
    HealthComponent* health = components->health[entity];
    change_texture(components, entity, health->filename_dead);
    change_layer(components, entity, LAYER_CORPSES);

    CoordinateComponent* coord = CoordinateComponent_get(components, entity);
    for (ListNode* node = coord->children->head; node; node = node->next) {
        int i = node->value;
        CoordinateComponent_get(components, i)->parent = -1;
        ImageComponent_get(components, i)->alpha = 1.0f;
        apply_force(components, i, mult(100.0f, rand_vector()));
    }

    if (AnimationComponent_get(components, entity)) {
        stop_animation(components, entity);
    }

    EnemyComponent* enemy = EnemyComponent_get(components, entity);
    if (enemy) {
        enemy->state = ENEMY_DEAD;
    }

    PlayerComponent* player = PlayerComponent_get(components, entity);
    if (player) {
        for (int j = 0; j < player->inventory_size; j++) {
            if (player->inventory[j] != -1) {
                coord->angle = rand_angle();
                drop_item(components, entity);
            }
        }

        for (int j = 1; j < player->ammo_size; j++) {
            AmmoComponent* ammo = AmmoComponent_get(components, player->ammo[j]);
            while (ammo->size > 0) {
                sfVector2f pos = get_position(components, entity);
                int k = create_ammo(components, sum(pos, polar_to_cartesian(1.0f, rand_angle())), ammo->type);
                int size = fminf(AmmoComponent_get(components, k)->size, ammo->size);
                AmmoComponent* drop = AmmoComponent_get(components, k);
                drop->size = size;
                ammo->size -= size;
            }
        }

        player->state = PLAYER_DEAD;
    }
}


void damage(ComponentData* components, int entity, sfVector2f pos, sfVector2f dir, int dmg) {
    HealthComponent* health = components->health[entity];
    if (health) {
        int prev_health = health->health;
        health->health = max(0, health->health - dmg);

        sfVector2f pos = sum(get_position(components, entity), rand_vector());
        if (dmg < 50) {
            create_decal(components, pos, 1.0f, 1.0f, "blood");
        } else {
            create_decal(components, pos, 2.0f, 2.0f, "blood_large");
        }

        if (prev_health > 0 && health->health == 0) {
            die(components, entity);
        }
    }

    EnemyComponent* enemy = components->enemy[entity];
    if (enemy) {
        CoordinateComponent_get(components, entity)->angle = polar_angle(mult(-1.0, dir));
    }
    
    PhysicsComponent* physics = PhysicsComponent_get(components, entity);
    if (physics) {
        apply_force(components, entity, mult(250.0f, dir));
    }

    ParticleComponent* particle = ParticleComponent_get(components, entity);
    if (particle) {
        particle->origin = diff(pos, get_position(components, entity));
        add_particles(components, entity, particle->rate / 50.0f * dmg);
    }

    SoundComponent* scomp = SoundComponent_get(components, entity);
    if (scomp && scomp->hit_sound[0] != '\0') {
        add_sound(components, entity, scomp->hit_sound, fminf(1.0f, dmg / 50.0f), randf(0.9f, 1.1f));
    }
}
