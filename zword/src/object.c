#define _USE_MATH_DEFINES
#include <math.h>
#include <string.h>
#include <stdio.h>

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


int create_bench(Vector2f position, float angle) {
    int i = create_entity();
    CoordinateComponent_add(i, position, angle);
    ImageComponent_add(i, "bench", 1.0f, 3.0f, LAYER_ITEMS);
    ColliderComponent_add_rectangle(i, 0.8f, 2.8f, GROUP_WALLS);
    ParticleComponent_add_type(i, PARTICLE_SPLINTER, 0.0f);
    HealthComponent_add(i, 100, "", "", "wood_destroy");
    PhysicsComponent_add(i, 2.0f);
    SoundComponent_add(i, "wood_hit");

    int j = create_entity();
    CoordinateComponent_add(j, (Vector2f) {0.0f, -1.0f}, 0.0f);
    ImageComponent_add(j, "bench_debris", 1.0f, 2.0f, LAYER_CORPSES)->alpha = 0.0f;
    ColliderComponent_add_rectangle(j, 0.8f, 1.2f, GROUP_DEBRIS)->enabled = false;
    PhysicsComponent_add(j, 1.0f);
    add_child(i, j);

    j = create_entity();
    CoordinateComponent_add(j, (Vector2f) {0.0f, 0.5f}, 0.0f);
    ImageComponent_add(j, "bench_destroyed", 1.0f, 2.0f, LAYER_CORPSES)->alpha = 0.0f;
    ColliderComponent_add_rectangle(j, 0.8f, 1.4f, GROUP_DEBRIS)->enabled = false;
    PhysicsComponent_add(j, 1.0f);
    add_child(i, j);

    return i;
}


int create_table(Vector2f position) {
    int i = create_entity();
    CoordinateComponent_add(i, position, rand_angle());
    ImageComponent_add(i, "table", 3.0f, 3.0f, LAYER_ITEMS)->alpha = 1.0f;
    ColliderComponent_add_circle(i, 1.4f, GROUP_WALLS);
    ParticleComponent_add_type(i, PARTICLE_SPLINTER, 0.0f);
    HealthComponent_add(i, 100, "", "", "wood_destroy");
    PhysicsComponent_add(i, 2.0f);
    SoundComponent_add(i, "wood_hit");

    int j = create_entity();
    CoordinateComponent_add(j, (Vector2f) {-0.6f, 0.0f}, 0.0f);
    ImageComponent_add(j, "table_destroyed", 2.0f, 3.0f, LAYER_CORPSES)->alpha = 0.0f;
    ColliderComponent_add_rectangle(j, 1.2f, 1.8f, GROUP_DEBRIS)->enabled = false;
    PhysicsComponent_add(j, 1.0f);
    add_child(i, j);

    j = create_entity();
    CoordinateComponent_add(j, (Vector2f) {0.6f, 0.0f}, M_PI);
    ImageComponent_add(j, "table_destroyed", 2.0f, 3.0f, LAYER_CORPSES)->alpha = 0.0f;
    ColliderComponent_add_rectangle(j, 1.2f, 1.8f, GROUP_DEBRIS)->enabled = false;
    PhysicsComponent_add(j, 1.0f);
    add_child(i, j);

    return i;
}


int create_hay_bale(Vector2f position, float angle) {
    int i = create_entity();

    CoordinateComponent_add(i, position, angle);
    ImageComponent_add(i, "hay_bale", 3.0f, 2.0f, LAYER_ITEMS);
    ColliderComponent_add_rectangle(i, 2.8f, 1.5f, GROUP_WALLS);
    ParticleComponent_add_type(i, PARTICLE_SPLINTER, 0.0f);

    return i;
}


int create_stove(Vector2f position, float angle) {
    int i = create_entity();

    CoordinateComponent_add(i, position, angle);
    ImageComponent_add(i, "stove", 2.0f, 2.0f, LAYER_ITEMS);
    ColliderComponent_add_rectangle(i, 1.8f, 1.8f, GROUP_WALLS);
    ParticleComponent_add_type(i, PARTICLE_SPARKS, 0.0f);
    SoundComponent_add(i, "metal_hit");

    return i;
}


int create_sink(Vector2f position, float angle) {
    int i = create_entity();

    CoordinateComponent_add(i, position, angle);
    ImageComponent_add(i, "sink", 2.0f, 3.0f, LAYER_ITEMS);
    ColliderComponent_add_rectangle(i, 1.8f, 2.8f, GROUP_WALLS);
    ParticleComponent_add_type(i, PARTICLE_SPARKS, 0.0f);
    SoundComponent_add(i, "metal_hit");

    return i;
}


int create_toilet(Vector2f position, float angle) {
    int i = create_entity();

    CoordinateComponent_add(i, position, angle);
    ImageComponent_add(i, "toilet", 2.0f, 2.0f, LAYER_ITEMS);
    ColliderComponent_add_rectangle(i, 1.8f, 1.3f, GROUP_WALLS);
    ParticleComponent_add_type(i, PARTICLE_SPARKS, 0.0f);
    SoundComponent_add(i, "metal_hit");

    return i;
}


int create_bed(Vector2f position, float angle) {
    int i = create_entity();

    CoordinateComponent_add(i, position, angle);
    ImageComponent_add(i, "bed", 4.0f, 2.0f, LAYER_ITEMS);
    ColliderComponent_add_rectangle(i, 3.8f, 1.8f, GROUP_WALLS);
    ParticleComponent_add_type(i, PARTICLE_SPLINTER, 0.0f);
    SoundComponent_add(i, "wood_hit");

    return i;
}


int create_candle(Vector2f pos) {
    int i = create_entity();

    CoordinateComponent_add(i, pos, rand_angle());
    ColliderComponent_add_circle(i, 0.5f, LAYER_WALLS);
    Color orange = get_color(1.0, 0.6, 0.0, 1.0);
    LightComponent_add(i, 7.5f, 2.0 * M_PI, orange, 0.8f, 10.0)->flicker = 0.1f;
    ParticleComponent_add_type(i, PARTICLE_FIRE, 0.25f);
    ImageComponent_add(i, "candle", 1.0f, 1.0f, LAYER_ITEMS);

    return i;
}


int create_lamp(Vector2f position) {
    int i = create_entity();

    CoordinateComponent_add(i, position, rand_angle());
    ImageComponent_add(i, "lamp", 1.0f, 1.0f, LAYER_ITEMS);
    ParticleComponent_add_type(i, PARTICLE_SPLINTER, 0.0f);
    SoundComponent_add(i, "wood_hit");
    LightComponent_add(i, 10.0f, 2.0f * M_PI, COLOR_WHITE, 0.5f, 5.0f)->flicker = 0.1f;
    ColliderComponent_add_circle(i, 0.5f, GROUP_VEHICLES);

    return i;
}


int create_desk(Vector2f position, float angle) {
    int i = create_entity();

    CoordinateComponent_add(i, position, angle);
    ImageComponent_add(i, "desk", 2.0, 2.0f, LAYER_ITEMS);
    ColliderComponent_add_rectangle(i, 1.3f, 1.8f, GROUP_WALLS);
    ParticleComponent_add_type(i, PARTICLE_SPLINTER, 0.0f);
    SoundComponent_add(i, "wood_hit");
    HealthComponent_add(i, 100, "", "", "wood_destroy");
    PhysicsComponent_add(i, 2.0f);

    int j = create_entity();
    CoordinateComponent_add(j, (Vector2f) {0.0f, -0.75f}, 0.0f);
    ImageComponent_add(j, "desk_destroyed", 2.0f, 2.0f, LAYER_CORPSES)->alpha = 0.0f;
    ColliderComponent_add_rectangle(j, 1.3f, 1.0f, GROUP_DEBRIS)->enabled = false;
    PhysicsComponent_add(j, 1.0f);
    add_child(i, j);

    j = create_entity();
    CoordinateComponent_add(j, (Vector2f) {0.0f, 0.25f}, 0.0f);
    ImageComponent_add(j, "desk_debris", 2.0f, 2.0f, LAYER_CORPSES)->alpha = 0.0f;
    ColliderComponent_add_rectangle(j, 1.3f, 1.0f, GROUP_DEBRIS)->enabled = false;
    PhysicsComponent_add(j, 1.0f);
    add_child(i, j);

    return i;
}


int create_fire(Vector2f pos) {
    int i = create_entity();

    CoordinateComponent_add(i, pos, 0.0);
    Color orange = get_color(1.0, 0.6, 0.0, 1.0);
    LightComponent_add(i, 18.0f, 2.0 * M_PI, orange, 0.8f, 10.0)->flicker = 0.1f;
    ParticleComponent_add_type(i, PARTICLE_FIRE, 0.6f);
    ColliderComponent_add_circle(i, 0.35, GROUP_BARRIERS);
    ImageComponent_add(i, "fire", 1.0, 1.0, LAYER_ITEMS);
    SoundComponent* sound = SoundComponent_add(i, "");
    strcpy(sound->loop_sound, "fire");

    return i;
}


int create_tree(Vector2f position) {
    int i = create_entity();
    float size = randf(1.0f, 1.5f);
    CoordinateComponent* coord = CoordinateComponent_add(i, position, rand_angle());
    coord->scale = vec(size, size);
    ColliderComponent_add_circle(i, 1.25f, GROUP_TREES);
    ImageComponent_add(i, "tree", 3.0, 3.0, LAYER_TREES);

    return i;
}


int create_rock(Vector2f position) {
    int i = create_entity();
    float size = randf(0.75f, 2.0f);
    CoordinateComponent* coord = CoordinateComponent_add(i, position, rand_angle());
    coord->scale = vec(size, size);
    ColliderComponent_add_circle(i, 1.4, GROUP_WALLS);
    ImageComponent_add(i, "rock", 3.0, 3.0, LAYER_DECALS);

    return i;
}


int create_uranium(Vector2f position) {
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


int create_object(ButtonText object_name, Vector2f position, float angle) {
    #define MATCH(x) if (strcmp(x, object_name) == 0)

    MATCH("bandage") return create_bandage(position);
    MATCH("bed") return create_bed(position, angle);
    MATCH("bench") return create_bench(position, angle);
    MATCH("blood") return create_decal(position, "blood", -1);
    MATCH("candle") return create_candle(position);
    MATCH("car") return create_car(position, angle);
    MATCH("desk") return create_desk(position, angle);
    MATCH("door") return create_door(position, angle);
    MATCH("fire") return create_fire(position);
    MATCH("flashlight") return create_flashlight(position);
    MATCH("gas") return create_gas(position);
    MATCH("hay bale") return create_hay_bale(position, angle);
    MATCH("hole") return create_decal(position, "hole", -1);
    MATCH("lamp") return create_lamp(position);
    MATCH("player") return create_player(position, angle);
    MATCH("rock") return create_rock(position);
    MATCH("stove") return create_stove(position, angle);
    MATCH("sink") return create_sink(position, angle);
    MATCH("table") return create_table(position);
    MATCH("toilet") return create_toilet(position, angle);
    MATCH("uranium") return create_uranium(position);
    MATCH("tree") return create_tree(position);
    MATCH("tutorial") create_tutorial(position);
    MATCH("waypoint") return create_waypoint(position);

    MATCH("big boy") return create_big_boy(position, angle);
    MATCH("boss") return create_boss(position, angle);
    MATCH("farmer") return create_farmer(position, angle);
    MATCH("priest") return create_priest(position, angle);
    MATCH("zombie") return create_zombie(position, angle);

    MATCH("ammo pistol") return create_ammo(position, AMMO_PISTOL);
    MATCH("ammorifle") return create_ammo(position, AMMO_RIFLE);
    MATCH("ammo shotgun") return create_ammo(position, AMMO_SHOTGUN);
    MATCH("assault rifle") return create_assault_rifle(position);
    MATCH("axe") return create_axe(position);
    MATCH("pistol") return create_pistol(position);
    MATCH("sawed-off") return create_sawed_off(position);
    MATCH("shotgun") return create_shotgun(position);
    MATCH("smg") return create_smg(position);
    MATCH("sword") return create_sword(position);

    #undef MATCH

    return -1;
}
