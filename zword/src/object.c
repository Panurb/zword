#define _USE_MATH_DEFINES
#include <math.h>
#include <string.h>
#include <stdio.h>

#include <SFML/System.h>

#include "object.h"
#include "component.h"
#include "particle.h"
#include "enemy.h"
#include "item.h"
#include "door.h"
#include "vehicle.h"
#include "collider.h"
#include "navigation.h"
#include "player.h"
#include "weapon.h"
#include "settings.h"
#include "input.h"


int create_bench(ComponentData* components, sfVector2f position, float angle) {
    int i = create_entity();
    CoordinateComponent_add(i, position, angle);
    ImageComponent_add(i, "bench", 1.0f, 3.0f, LAYER_ITEMS);
    ColliderComponent_add_rectangle(i, 0.8f, 2.8f, GROUP_WALLS);
    ParticleComponent_add_type(i, PARTICLE_SPLINTER, 0.0f);
    HealthComponent_add(i, 100, "", "", "wood_destroy");
    PhysicsComponent_add(i, 2.0f);
    SoundComponent_add(i, "wood_hit");

    int j = create_entity();
    CoordinateComponent_add(j, (sfVector2f) {0.0f, -1.0f}, 0.0f);
    ImageComponent_add(j, "bench_debris", 1.0f, 2.0f, LAYER_CORPSES)->alpha = 0.0f;
    ColliderComponent_add_rectangle(j, 0.8f, 1.2f, GROUP_DEBRIS)->enabled = false;
    PhysicsComponent_add(j, 1.0f);
    add_child(i, j);

    j = create_entity();
    CoordinateComponent_add(j, (sfVector2f) {0.0f, 0.5f}, 0.0f);
    ImageComponent_add(j, "bench_destroyed", 1.0f, 2.0f, LAYER_CORPSES)->alpha = 0.0f;
    ColliderComponent_add_rectangle(j, 0.8f, 1.4f, GROUP_DEBRIS)->enabled = false;
    PhysicsComponent_add(j, 1.0f);
    add_child(i, j);

    return i;
}


int create_table(ComponentData* components, sfVector2f position) {
    int i = create_entity();
    CoordinateComponent_add(i, position, rand_angle());
    ImageComponent_add(i, "table", 3.0f, 3.0f, LAYER_ITEMS)->alpha = 1.0f;
    ColliderComponent_add_circle(i, 1.4f, GROUP_WALLS);
    ParticleComponent_add_type(i, PARTICLE_SPLINTER, 0.0f);
    HealthComponent_add(i, 100, "", "", "wood_destroy");
    PhysicsComponent_add(i, 2.0f);
    SoundComponent_add(i, "wood_hit");

    int j = create_entity();
    CoordinateComponent_add(j, (sfVector2f) {-0.6f, 0.0f}, 0.0f);
    ImageComponent_add(j, "table_destroyed", 2.0f, 3.0f, LAYER_CORPSES)->alpha = 0.0f;
    ColliderComponent_add_rectangle(j, 1.2f, 1.8f, GROUP_DEBRIS)->enabled = false;
    PhysicsComponent_add(j, 1.0f);
    add_child(i, j);

    j = create_entity();
    CoordinateComponent_add(j, (sfVector2f) {0.6f, 0.0f}, M_PI);
    ImageComponent_add(j, "table_destroyed", 2.0f, 3.0f, LAYER_CORPSES)->alpha = 0.0f;
    ColliderComponent_add_rectangle(j, 1.2f, 1.8f, GROUP_DEBRIS)->enabled = false;
    PhysicsComponent_add(j, 1.0f);
    add_child(i, j);

    return i;
}


int create_hay_bale(ComponentData* components, sfVector2f position, float angle) {
    int i = create_entity();

    CoordinateComponent_add(i, position, angle);
    ImageComponent_add(i, "hay_bale", 3.0f, 2.0f, LAYER_ITEMS);
    ColliderComponent_add_rectangle(i, 2.8f, 1.5f, GROUP_WALLS);
    ParticleComponent_add_type(i, PARTICLE_SPLINTER, 0.0f);

    return i;
}


int create_stove(ComponentData* components, sfVector2f position, float angle) {
    int i = create_entity();

    CoordinateComponent_add(i, position, angle);
    ImageComponent_add(i, "stove", 2.0f, 2.0f, LAYER_ITEMS);
    ColliderComponent_add_rectangle(i, 1.8f, 1.8f, GROUP_WALLS);
    ParticleComponent_add_type(i, PARTICLE_SPARKS, 0.0f);
    SoundComponent_add(i, "metal_hit");

    return i;
}


int create_sink(ComponentData* components, sfVector2f position, float angle) {
    int i = create_entity();

    CoordinateComponent_add(i, position, angle);
    ImageComponent_add(i, "sink", 2.0f, 3.0f, LAYER_ITEMS);
    ColliderComponent_add_rectangle(i, 1.8f, 2.8f, GROUP_WALLS);
    ParticleComponent_add_type(i, PARTICLE_SPARKS, 0.0f);
    SoundComponent_add(i, "metal_hit");

    return i;
}


int create_toilet(ComponentData* components, sfVector2f position, float angle) {
    int i = create_entity();

    CoordinateComponent_add(i, position, angle);
    ImageComponent_add(i, "toilet", 2.0f, 2.0f, LAYER_ITEMS);
    ColliderComponent_add_rectangle(i, 1.8f, 1.3f, GROUP_WALLS);
    ParticleComponent_add_type(i, PARTICLE_SPARKS, 0.0f);
    SoundComponent_add(i, "metal_hit");

    return i;
}


int create_bed(ComponentData* components, sfVector2f position, float angle) {
    int i = create_entity();

    CoordinateComponent_add(i, position, angle);
    ImageComponent_add(i, "bed", 4.0f, 2.0f, LAYER_ITEMS);
    ColliderComponent_add_rectangle(i, 3.8f, 1.8f, GROUP_WALLS);
    ParticleComponent_add_type(i, PARTICLE_SPLINTER, 0.0f);
    SoundComponent_add(i, "wood_hit");

    return i;
}


int create_candle(ComponentData* components, sfVector2f pos) {
    int i = create_entity();

    CoordinateComponent_add(i, pos, rand_angle());
    ColliderComponent_add_circle(i, 0.5f, LAYER_WALLS);
    sfColor orange = get_color(1.0, 0.6, 0.0, 1.0);
    LightComponent_add(i, 7.5f, 2.0 * M_PI, orange, 0.8f, 10.0)->flicker = 0.1f;
    ParticleComponent_add_type(i, PARTICLE_FIRE, 0.25f);
    ImageComponent_add(i, "candle", 1.0f, 1.0f, LAYER_ITEMS);

    return i;
}


int create_lamp(ComponentData* components, sfVector2f position) {
    int i = create_entity();

    CoordinateComponent_add(i, position, rand_angle());
    ImageComponent_add(i, "lamp", 1.0f, 1.0f, LAYER_ITEMS);
    ParticleComponent_add_type(i, PARTICLE_SPLINTER, 0.0f);
    SoundComponent_add(i, "wood_hit");
    LightComponent_add(i, 10.0f, 2.0f * M_PI, sfWhite, 0.5f, 5.0f)->flicker = 0.1f;
    ColliderComponent_add_circle(i, 0.5f, GROUP_VEHICLES);

    return i;
}


int create_desk(ComponentData* components, sfVector2f position, float angle) {
    int i = create_entity();

    CoordinateComponent_add(i, position, angle);
    ImageComponent_add(i, "desk", 2.0, 2.0f, LAYER_ITEMS);
    ColliderComponent_add_rectangle(i, 1.3f, 1.8f, GROUP_WALLS);
    ParticleComponent_add_type(i, PARTICLE_SPLINTER, 0.0f);
    SoundComponent_add(i, "wood_hit");
    HealthComponent_add(i, 100, "", "", "wood_destroy");
    PhysicsComponent_add(i, 2.0f);

    int j = create_entity();
    CoordinateComponent_add(j, (sfVector2f) {0.0f, -0.75f}, 0.0f);
    ImageComponent_add(j, "desk_destroyed", 2.0f, 2.0f, LAYER_CORPSES)->alpha = 0.0f;
    ColliderComponent_add_rectangle(j, 1.3f, 1.0f, GROUP_DEBRIS)->enabled = false;
    PhysicsComponent_add(j, 1.0f);
    add_child(i, j);

    j = create_entity();
    CoordinateComponent_add(j, (sfVector2f) {0.0f, 0.25f}, 0.0f);
    ImageComponent_add(j, "desk_debris", 2.0f, 2.0f, LAYER_CORPSES)->alpha = 0.0f;
    ColliderComponent_add_rectangle(j, 1.3f, 1.0f, GROUP_DEBRIS)->enabled = false;
    PhysicsComponent_add(j, 1.0f);
    add_child(i, j);

    return i;
}


int create_fire(ComponentData* components, sfVector2f pos) {
    int i = create_entity();

    CoordinateComponent_add(i, pos, 0.0);
    sfColor orange = get_color(1.0, 0.6, 0.0, 1.0);
    LightComponent_add(i, 18.0f, 2.0 * M_PI, orange, 0.8f, 10.0)->flicker = 0.1f;
    ParticleComponent_add_type(i, PARTICLE_FIRE, 0.6f);
    ColliderComponent_add_circle(i, 0.35, GROUP_BARRIERS);
    ImageComponent_add(i, "fire", 1.0, 1.0, LAYER_ITEMS);
    SoundComponent* sound = SoundComponent_add(i, "");
    strcpy(sound->loop_sound, "fire");

    return i;
}


int create_tree(ComponentData* components, sfVector2f position) {
    int i = create_entity();
    CoordinateComponent_add(i, position, rand_angle());    
    float size = randf(1.0f, 1.5f);
    ColliderComponent_add_circle(i, 1.25f * size, GROUP_TREES);
    ImageComponent_add(i, "tree", 3.0, 3.0, LAYER_TREES)->scale = (sfVector2f) { size, size };

    return i;
}


int create_rock(ComponentData* components, sfVector2f position) {
    int i = create_entity();
    CoordinateComponent_add(i, position, rand_angle());
    float size = randf(0.75f, 2.0f);
    ColliderComponent_add_circle(i, 1.4 * size, GROUP_WALLS);
    ImageComponent_add(i, "rock", 3.0, 3.0, LAYER_DECALS)->scale = (sfVector2f) { size, size };

    return i;
}


int create_uranium(ComponentData* components, sfVector2f position) {
    int i = create_entity();
    CoordinateComponent_add(i, position, rand_angle());
    float r = randf(0.8f, 1.2f);
    ColliderComponent_add_circle(i, 0.5f * r, GROUP_TREES);

    ImageComponent_add(i, "uranium", r, r, LAYER_DECALS);
    LightComponent_add(i, randf(3.0, 5.0), 2.0 * M_PI, COLOR_ENERGY, 0.5, 1.0)->flicker = 0.25;
    ParticleComponent* particle = ParticleComponent_add(i, 0.0, 2.0 * M_PI, 0.1, 0.05, 2.0, 5.0, 
                                                        get_color(0.5, 1.0, 0.0, 1.0), get_color(0.5, 1.0, 0.0, 1.0));
    particle->enabled = true;
    particle->loop = true;

    SoundComponent* sound = SoundComponent_add(i, "stone_hit");
    strcpy(sound->loop_sound, "geiger");

    return i;
}


int create_object(ComponentData* components, ButtonText object_name, sfVector2f position, float angle) {
    #define MATCH(x) if (strcmp(x, object_name) == 0)

    MATCH("bandage") return create_bandage(position);
    MATCH("bed") return create_bed(components, position, angle);
    MATCH("bench") return create_bench(components, position, angle);
    MATCH("big boy") return create_big_boy(components, position, angle);
    MATCH("blood") return create_decal(position, "blood", -1);
    MATCH("boss") return create_boss(components, position, angle);
    MATCH("candle") return create_candle(components, position);
    MATCH("car") return create_car(components, position, angle);
    MATCH("desk") return create_desk(components, position, angle);
    MATCH("door") return create_door(position, angle);
    MATCH("farmer") return create_farmer(components, position, angle);
    MATCH("fire") return create_fire(components, position);
    MATCH("flashlight") return create_flashlight(position);
    MATCH("gas") return create_gas(position);
    MATCH("hay bale") return create_hay_bale(components, position, angle);
    MATCH("hole") return create_decal(position, "hole", -1);
    MATCH("lamp") return create_lamp(components, position);
    MATCH("player") return create_player(position, angle);
    MATCH("priest") return create_priest(components, position, angle);
    MATCH("rock") return create_rock(components, position);
    MATCH("stove") return create_stove(components, position, angle);
    MATCH("sink") return create_sink(components, position, angle);
    MATCH("table") return create_table(components, position);
    MATCH("toilet") return create_toilet(components, position, angle);
    MATCH("uranium") return create_uranium(components, position);
    MATCH("tree") return create_tree(components, position);
    MATCH("tutorial") create_tutorial(components, position);
    MATCH("waypoint") return create_waypoint(position);
    MATCH("zombie") return create_zombie(components, position, angle);

    MATCH("ammo pistol") return create_ammo(components, position, AMMO_PISTOL);
    MATCH("ammorifle") return create_ammo(components, position, AMMO_RIFLE);
    MATCH("ammo shotgun") return create_ammo(components, position, AMMO_SHOTGUN);
    MATCH("assault rifle") return create_assault_rifle(components, position);
    MATCH("axe") return create_axe(components, position);
    MATCH("pistol") return create_pistol(components, position);
    MATCH("sawed-off") return create_sawed_off(components, position);
    MATCH("shotgun") return create_shotgun(components, position);
    MATCH("smg") return create_smg(components, position);
    MATCH("sword") return create_sword(components, position);

    #undef MATCH

    return -1;
}
