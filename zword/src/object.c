#define _USE_MATH_DEFINES
#include <math.h>
#include <string.h>

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


int create_bench(ComponentData* components, sfVector2f position, float angle) {
    int i = create_entity(components);
    CoordinateComponent_add(components, i, position, angle);
    ImageComponent_add(components, i, "bench", 1.0f, 3.0f, LAYER_ITEMS);
    ColliderComponent_add_rectangle(components, i, 0.8f, 2.8f, GROUP_WALLS);
    ParticleComponent_add_type(components, i, PARTICLE_SPLINTER, 0.0f);
    HealthComponent_add(components, i, 100, "", "", "wood_destroy");
    PhysicsComponent_add(components, i, 2.0f);
    SoundComponent_add(components, i, "wood_hit");

    int j = create_entity(components);
    CoordinateComponent_add(components, j, (sfVector2f) {0.0f, -1.0f}, 0.0f);
    ImageComponent_add(components, j, "bench_debris", 1.0f, 2.0f, LAYER_CORPSES)->alpha = 0.0f;
    ColliderComponent_add_rectangle(components, j, 0.8f, 1.2f, GROUP_DEBRIS)->enabled = false;
    PhysicsComponent_add(components, j, 1.0f);
    add_child(components, i, j);

    j = create_entity(components);
    CoordinateComponent_add(components, j, (sfVector2f) {0.0f, 0.5f}, 0.0f);
    ImageComponent_add(components, j, "bench_destroyed", 1.0f, 2.0f, LAYER_CORPSES)->alpha = 0.0f;
    ColliderComponent_add_rectangle(components, j, 0.8f, 1.4f, GROUP_DEBRIS)->enabled = false;
    PhysicsComponent_add(components, j, 1.0f);
    add_child(components, i, j);

    return i;
}


int create_table(ComponentData* components, sfVector2f position) {
    int i = create_entity(components);
    CoordinateComponent_add(components, i, position, rand_angle());
    ImageComponent_add(components, i, "table", 3.0f, 3.0f, LAYER_ITEMS)->alpha = 1.0f;
    ColliderComponent_add_circle(components, i, 1.4f, GROUP_WALLS);
    ParticleComponent_add_type(components, i, PARTICLE_SPLINTER, 0.0f);
    HealthComponent_add(components, i, 100, "", "", "wood_destroy");
    PhysicsComponent_add(components, i, 2.0f);
    SoundComponent_add(components, i, "wood_hit");

    int j = create_entity(components);
    CoordinateComponent_add(components, j, (sfVector2f) {-0.6f, 0.0f}, 0.0f);
    ImageComponent_add(components, j, "table_destroyed", 2.0f, 3.0f, LAYER_CORPSES)->alpha = 0.0f;
    ColliderComponent_add_rectangle(components, j, 1.2f, 1.8f, GROUP_DEBRIS)->enabled = false;
    PhysicsComponent_add(components, j, 1.0f);
    add_child(components, i, j);

    j = create_entity(components);
    CoordinateComponent_add(components, j, (sfVector2f) {0.6f, 0.0f}, M_PI);
    ImageComponent_add(components, j, "table_destroyed", 2.0f, 3.0f, LAYER_CORPSES)->alpha = 0.0f;
    ColliderComponent_add_rectangle(components, j, 1.2f, 1.8f, GROUP_DEBRIS)->enabled = false;
    PhysicsComponent_add(components, j, 1.0f);
    add_child(components, i, j);

    return i;
}


int create_hay_bale(ComponentData* components, sfVector2f position, float angle) {
    int i = create_entity(components);

    CoordinateComponent_add(components, i, position, angle);
    ImageComponent_add(components, i, "hay_bale", 3.0f, 2.0f, LAYER_ITEMS);
    ColliderComponent_add_rectangle(components, i, 2.8f, 1.5f, GROUP_WALLS);
    ParticleComponent_add_type(components, i, PARTICLE_SPLINTER, 0.0f);

    return i;
}


int create_stove(ComponentData* components, sfVector2f position, float angle) {
    int i = create_entity(components);

    CoordinateComponent_add(components, i, position, angle);
    ImageComponent_add(components, i, "stove", 2.0f, 2.0f, LAYER_ITEMS);
    ColliderComponent_add_rectangle(components, i, 1.8f, 1.8f, GROUP_WALLS);
    ParticleComponent_add_type(components, i, PARTICLE_SPARKS, 0.0f);
    SoundComponent_add(components, i, "metal_hit");

    return i;
}


int create_sink(ComponentData* components, sfVector2f position, float angle) {
    int i = create_entity(components);

    CoordinateComponent_add(components, i, position, angle);
    ImageComponent_add(components, i, "sink", 2.0f, 3.0f, LAYER_ITEMS);
    ColliderComponent_add_rectangle(components, i, 1.8f, 2.8f, GROUP_WALLS);
    ParticleComponent_add_type(components, i, PARTICLE_SPARKS, 0.0f);
    SoundComponent_add(components, i, "metal_hit");

    return i;
}


int create_toilet(ComponentData* components, sfVector2f position, float angle) {
    int i = create_entity(components);

    CoordinateComponent_add(components, i, position, angle);
    ImageComponent_add(components, i, "toilet", 2.0f, 2.0f, LAYER_ITEMS);
    ColliderComponent_add_rectangle(components, i, 1.8f, 1.3f, GROUP_WALLS);
    ParticleComponent_add_type(components, i, PARTICLE_SPARKS, 0.0f);
    SoundComponent_add(components, i, "metal_hit");

    return i;
}


int create_bed(ComponentData* components, sfVector2f position, float angle) {
    int i = create_entity(components);

    CoordinateComponent_add(components, i, position, angle);
    ImageComponent_add(components, i, "bed", 4.0f, 2.0f, LAYER_ITEMS);
    ColliderComponent_add_rectangle(components, i, 3.8f, 1.8f, GROUP_WALLS);
    ParticleComponent_add_type(components, i, PARTICLE_SPLINTER, 0.0f);
    SoundComponent_add(components, i, "wood_hit");

    return i;
}


int create_candle(ComponentData* components, sfVector2f pos) {
    int i = create_entity(components);

    CoordinateComponent_add(components, i, pos, rand_angle());
    ColliderComponent_add_circle(components, i, 0.5f, LAYER_WALLS);
    sfColor orange = get_color(1.0, 0.6, 0.0, 1.0);
    LightComponent_add(components, i, 7.5f, 2.0 * M_PI, orange, 0.8f, 10.0)->flicker = 0.1f;
    ParticleComponent_add_type(components, i, PARTICLE_FIRE, 0.25f);
    ImageComponent_add(components, i, "candle", 1.0f, 1.0f, LAYER_ITEMS);

    return i;
}


int create_lamp(ComponentData* components, sfVector2f position) {
    int i = create_entity(components);

    CoordinateComponent_add(components, i, position, rand_angle());
    ImageComponent_add(components, i, "lamp", 1.0f, 1.0f, LAYER_ITEMS);
    ParticleComponent_add_type(components, i, PARTICLE_SPLINTER, 0.0f);
    SoundComponent_add(components, i, "wood_hit");
    LightComponent_add(components, i, 10.0f, 2.0f * M_PI, sfWhite, 0.5f, 5.0f)->flicker = 0.1f;
    ColliderComponent_add_circle(components, i, 0.5f, GROUP_VEHICLES);

    return i;
}


int create_desk(ComponentData* components, sfVector2f position, float angle) {
    int i = create_entity(components);

    CoordinateComponent_add(components, i, position, angle);
    ImageComponent_add(components, i, "desk", 2.0, 2.0f, LAYER_ITEMS);
    ColliderComponent_add_rectangle(components, i, 1.3f, 1.8f, GROUP_WALLS);
    ParticleComponent_add_type(components, i, PARTICLE_SPLINTER, 0.0f);
    SoundComponent_add(components, i, "wood_hit");
    HealthComponent_add(components, i, 100, "", "", "wood_destroy");
    PhysicsComponent_add(components, i, 2.0f);

    int j = create_entity(components);
    CoordinateComponent_add(components, j, (sfVector2f) {0.0f, -0.75f}, 0.0f);
    ImageComponent_add(components, j, "desk_destroyed", 2.0f, 2.0f, LAYER_CORPSES)->alpha = 0.0f;
    ColliderComponent_add_rectangle(components, j, 1.3f, 1.0f, GROUP_DEBRIS)->enabled = false;
    PhysicsComponent_add(components, j, 1.0f);
    add_child(components, i, j);

    j = create_entity(components);
    CoordinateComponent_add(components, j, (sfVector2f) {0.0f, 0.25f}, 0.0f);
    ImageComponent_add(components, j, "desk_debris", 2.0f, 2.0f, LAYER_CORPSES)->alpha = 0.0f;
    ColliderComponent_add_rectangle(components, j, 1.3f, 1.0f, GROUP_DEBRIS)->enabled = false;
    PhysicsComponent_add(components, j, 1.0f);
    add_child(components, i, j);

    return i;
}


int create_fire(ComponentData* components, sfVector2f pos) {
    int i = create_entity(components);

    CoordinateComponent_add(components, i, pos, 0.0);
    sfColor orange = get_color(1.0, 0.6, 0.0, 1.0);
    LightComponent_add(components, i, 18.0f, 2.0 * M_PI, orange, 0.8f, 10.0)->flicker = 0.1f;
    ParticleComponent_add_type(components, i, PARTICLE_FIRE, 0.6f);
    ColliderComponent_add_circle(components, i, 0.35, GROUP_BARRIERS);
    ImageComponent_add(components, i, "fire", 1.0, 1.0, LAYER_PARTICLES);

    return i;
}


int create_tree(ComponentData* components, sfVector2f position) {
    int i = create_entity(components);
    CoordinateComponent_add(components, i, position, rand_angle());    
    float size = randf(1.0f, 1.5f);
    ColliderComponent_add_circle(components, i, 1.25f * size, GROUP_TREES);
    ImageComponent_add(components, i, "tree", 3.0, 3.0, LAYER_TREES)->scale = (sfVector2f) { size, size };

    return i;
}


int create_rock(ComponentData* components, sfVector2f position) {
    int i = create_entity(components);
    CoordinateComponent_add(components, i, position, rand_angle());
    float size = randf(0.75f, 2.0f);
    ColliderComponent_add_circle(components, i, 1.4 * size, GROUP_TREES);
    ImageComponent_add(components, i, "rock", 3.0, 3.0, LAYER_DECALS)->scale = (sfVector2f) { size, size };

    return i;
}


int create_object(ComponentData* components, ButtonText object_name, sfVector2f position, float angle) {
    #define MATCH(x) if (strcmp(x, object_name) == 0)

    MATCH("bed") return create_bed(components, position, angle);
    MATCH("bench") return create_bench(components, position, angle);
    MATCH("big boy") return create_big_boy(components, position, angle);
    MATCH("boss") return create_boss(components, position, angle);
    MATCH("candle") return create_candle(components, position);
    MATCH("car") return create_car(components, position, angle);
    MATCH("desk") return create_desk(components, position, angle);
    MATCH("door") return create_door(components, position, angle);
    MATCH("farmer") return create_farmer(components, position, angle);
    MATCH("flashlight") return create_flashlight(components, position);
    MATCH("gas") return create_gas(components, position);
    MATCH("hay bale") return create_hay_bale(components, position, angle);
    MATCH("lamp") return create_lamp(components, position);
    MATCH("player") return create_player(components, position, angle);
    MATCH("priest") return create_priest(components, position, angle);
    MATCH("rock") return create_rock(components, position);
    MATCH("table") return create_table(components, position);
    MATCH("tree") return create_tree(components, position);
    MATCH("waypoint") return create_waypoint(components, position);
    MATCH("zombie") return create_zombie(components, position, angle);

    MATCH("ammo pistol") return create_ammo(components, position, AMMO_PISTOL);
    MATCH("ammorifle") return create_ammo(components, position, AMMO_RIFLE);
    MATCH("ammo shotgun") return create_ammo(components, position, AMMO_SHOTGUN);
    MATCH("assault rifle") return create_assault_rifle(components, position);
    MATCH("axe") return create_axe(components, position);
    MATCH("pistol") return create_pistol(components, position);
    MATCH("shotgun") return create_shotgun(components, position);
}
