#define _USE_MATH_DEFINES
#include <math.h>

#include <SFML/System.h>

#include "component.h"
#include "particle.h"
#include "enemy.h"
#include "item.h"
#include "door.h"
#include "vehicle.h"
#include "collider.h"


void create_bench(ComponentData* components, sfVector2f position, float angle) {
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
}


void create_table(ComponentData* components, sfVector2f position, float angle) {
    UNUSED(angle);

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
}


void create_hay_bale(ComponentData* components, sfVector2f position, float angle) {
    int i = create_entity(components);

    CoordinateComponent_add(components, i, position, angle);
    ImageComponent_add(components, i, "hay_bale", 3.0f, 2.0f, LAYER_ITEMS);
    ColliderComponent_add_rectangle(components, i, 2.8f, 1.5f, GROUP_WALLS);
    ParticleComponent_add_type(components, i, PARTICLE_SPLINTER, 0.0f);
}


void create_stove(ComponentData* components, sfVector2f position, float angle) {
    int i = create_entity(components);

    CoordinateComponent_add(components, i, position, angle);
    ImageComponent_add(components, i, "stove", 2.0f, 2.0f, LAYER_ITEMS);
    ColliderComponent_add_rectangle(components, i, 1.8f, 1.8f, GROUP_WALLS);
    ParticleComponent_add_type(components, i, PARTICLE_SPARKS, 0.0f);
    SoundComponent_add(components, i, "metal_hit");
}


void create_sink(ComponentData* components, sfVector2f position, float angle) {
    int i = create_entity(components);

    CoordinateComponent_add(components, i, position, angle);
    ImageComponent_add(components, i, "sink", 2.0f, 3.0f, LAYER_ITEMS);
    ColliderComponent_add_rectangle(components, i, 1.8f, 2.8f, GROUP_WALLS);
    ParticleComponent_add_type(components, i, PARTICLE_SPARKS, 0.0f);
    SoundComponent_add(components, i, "metal_hit");
}


void create_toilet(ComponentData* components, sfVector2f position, float angle) {
    int i = create_entity(components);

    CoordinateComponent_add(components, i, position, angle);
    ImageComponent_add(components, i, "toilet", 2.0f, 2.0f, LAYER_ITEMS);
    ColliderComponent_add_rectangle(components, i, 1.8f, 1.3f, GROUP_WALLS);
    ParticleComponent_add_type(components, i, PARTICLE_SPARKS, 0.0f);
    SoundComponent_add(components, i, "metal_hit");
}


void create_bed(ComponentData* components, sfVector2f position, float angle) {
    int i = create_entity(components);

    CoordinateComponent_add(components, i, position, angle);
    ImageComponent_add(components, i, "bed", 4.0f, 2.0f, LAYER_ITEMS);
    ColliderComponent_add_rectangle(components, i, 3.8f, 1.8f, GROUP_WALLS);
    ParticleComponent_add_type(components, i, PARTICLE_SPLINTER, 0.0f);
    SoundComponent_add(components, i, "wood_hit");
}


void create_candle(ComponentData* components, sfVector2f pos, float angle) {
    UNUSED(angle);
    int i = create_entity(components);

    CoordinateComponent_add(components, i, pos, rand_angle());
    ColliderComponent_add_circle(components, i, 0.5f, LAYER_WALLS);
    sfColor orange = get_color(1.0, 0.6, 0.0, 1.0);
    LightComponent_add(components, i, 7.5f, 2.0 * M_PI, orange, 0.8f, 10.0)->flicker = 0.1f;
    ParticleComponent_add_type(components, i, PARTICLE_FIRE, 0.25f);
    ImageComponent_add(components, i, "candle", 1.0f, 1.0f, LAYER_ITEMS);
}


void create_lamp(ComponentData* components, sfVector2f position, float angle) {
    UNUSED(angle);
    int i = create_entity(components);

    CoordinateComponent_add(components, i, position, rand_angle());
    ImageComponent_add(components, i, "lamp", 1.0f, 1.0f, LAYER_ITEMS);
    ParticleComponent_add_type(components, i, PARTICLE_SPLINTER, 0.0f);
    SoundComponent_add(components, i, "wood_hit");
    LightComponent_add(components, i, 10.0f, 2.0f * M_PI, sfWhite, 0.5f, 5.0f)->flicker = 0.1f;
    ColliderComponent_add_circle(components, i, 0.5f, GROUP_VEHICLES);
}


void create_desk(ComponentData* components, sfVector2f position, float angle) {
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
}


void create_fire(ComponentData* components, sfVector2f pos) {
    int i = create_entity(components);

    CoordinateComponent_add(components, i, pos, 0.0);
    sfColor orange = get_color(1.0, 0.6, 0.0, 1.0);
    LightComponent_add(components, i, 18.0f, 2.0 * M_PI, orange, 0.8f, 10.0)->flicker = 0.1f;
    ParticleComponent_add_type(components, i, PARTICLE_FIRE, 0.6f);
    ColliderComponent_add_circle(components, i, 0.35, GROUP_OBSTACLES);
    ImageComponent_add(components, i, "fire", 1.0, 1.0, LAYER_PARTICLES);
}


void create_tree(ComponentData* components, ColliderGrid* grid, sfVector2f position) {
    int i = create_entity(components);
    CoordinateComponent_add(components, i, position, rand_angle());    
    float size = randf(1.0f, 1.5f);
    ColliderComponent_add_circle(components, i, 1.25f * size, GROUP_TREES);

    if (collides_with(components, grid, i, NULL)) {
        destroy_entity(components, i);
        return;
    }

    ImageComponent_add(components, i, "tree", 3.0, 3.0, LAYER_TREES)->scale = (sfVector2f) { size, size };
}


void create_rock(ComponentData* components, ColliderGrid* grid, sfVector2f position) {
    int i = create_entity(components);
    CoordinateComponent_add(components, i, position, rand_angle());
    float size = randf(0.75f, 2.0f);
    ColliderComponent_add_circle(components, i, 1.4 * size, GROUP_TREES);

    if (collides_with(components, grid, i, NULL)) {
        destroy_entity(components, i);
        return;
    }

    ImageComponent_add(components, i, "rock", 3.0, 3.0, LAYER_DECALS)->scale = (sfVector2f) { size, size };
}


void create_object(ComponentData* components, int object, sfVector2f position, float angle) {
    static void (*object_constructors[])(ComponentData*, sfVector2f, float) = {
        create_bed,
        create_bench,
        create_big_boy,
        create_boss,
        create_candle,
        create_car,
        create_desk,
        create_door,
        create_farmer,
        create_flashlight,
        create_gas,
        create_hay_bale,
        create_lamp,
        create_priest,
        create_zombie
    };

    object_constructors[object](components, position, angle);
}
