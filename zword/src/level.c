#define _USE_MATH_DEFINES

#include <stdlib.h>
#include <math.h>

#include "image.h"
#include "component.h"
#include "player.h"
#include "util.h"


void create_wall(Component* component, sfVector2f pos, float width, float height, float angle) {
    int i = component->entities;
    component->entities++;

    component->coordinate[i] = CoordinateComponent_create(pos, angle);
    component->rectangle_collider[i] = RectangleColliderComponent_create(width, height);
}


void create_prop(Component* component, sfVector2f pos) {
    int i = component->entities;
    component->entities++;

    component->coordinate[i] = CoordinateComponent_create(pos, float_rand(0.0, 2 * M_PI));
    component->rectangle_collider[i] = RectangleColliderComponent_create(1.0, 1.0);
    component->physics[i] = PhysicsComponent_create(1.0, 0.5, 0.5, 2.0);
}


void create_enemy(Component* component, sfVector2f pos) {
    int i = component->entities;
    component->entities++;

    component->coordinate[i] = CoordinateComponent_create(pos, float_rand(0.0, 2 * M_PI));
    component->image[i] = ImageComponent_create("player", 1.0);
    component->circle_collider[i] = CircleColliderComponent_create(0.5);
    component->physics[i] = PhysicsComponent_create(1.0, 0.0, 0.0, 5.0);
    component->physics[i]->max_speed = 2.0;
    component->enemy[i] = EnemyComponent_create();
    component->particle[i] = ParticleComponent_create(0.0, 2 * M_PI, 0.1, 3.0, 10.0, sfRed);
}


void create_pointlight(Component* component, sfVector2f pos) {
    int i = component->entities;
    component->entities++;

    component->coordinate[i] = CoordinateComponent_create(pos, 0.0);
    component->light[i] = LightComponent_create(10.0, 2.0 * M_PI, 201, 0.25);
}


void create_house(Component* component, float x, float y) {
    float angle = float_rand(0.0, 2 * M_PI);

    sfVector2f w = polar_to_cartesian(3.0, angle);
    sfVector2f h = perp(w);

    sfVector2f pos = { x, y };

    create_wall(component, sum(pos, w), 0.5, 5.5, angle);
    create_wall(component, diff(pos, w), 0.5, 5.5, angle);
    create_wall(component, diff(pos, h), 6.5, 0.5, angle + M_PI);

    create_wall(component, sum(pos, sum(h, mult(2.0 / 3.0, w))), 2.5, 0.5, angle + M_PI);
    create_wall(component, sum(pos, diff(h, mult(2.0 / 3.0, w))), 2.5, 0.5, angle + M_PI);

    create_enemy(component, sum(pos, polar_to_cartesian(2.0, float_rand(0.0, 2 * M_PI))));
    create_enemy(component, sum(pos, polar_to_cartesian(2.0, float_rand(0.0, 2 * M_PI))));
    create_enemy(component, sum(pos, polar_to_cartesian(2.0, float_rand(0.0, 2 * M_PI))));

    create_pointlight(component, pos);
}


void create_car(Component* component, float x, float y) {
    int i = component->entities;
    component->entities++;

    sfVector2f pos = { x, y };
    component->coordinate[i] = CoordinateComponent_create(pos, 0.0);
    component->rectangle_collider[i] = RectangleColliderComponent_create(6.0, 3.0);
    component->physics[i] = PhysicsComponent_create(10.0, 0.0, 0.5, 10.0);
    component->vehicle[i] = VehicleComponent_create();

    i = component->entities;
    component->entities++;

    component->coordinate[i] = CoordinateComponent_create((sfVector2f) { 3.1, 1.0 }, 0.0);
    component->coordinate[i]->parent = i - 1;
    component->light[i] = LightComponent_create(10.0, 1.0, 51, 0.4);
    component->light[i]->enabled = false;

    i = component->entities;
    component->entities++;

    component->coordinate[i] = CoordinateComponent_create((sfVector2f) { 3.1, -1.0 }, 0.0);
    component->coordinate[i]->parent = i - 2;
    component->light[i] = LightComponent_create(10.0, 1.0, 51, 0.4);
    component->light[i]->enabled = false;
}


void create_weapon(Component* component, float x, float y) {
    int i = component->entities;
    component->entities++;

    sfVector2f pos = { x, y };
    component->coordinate[i] = CoordinateComponent_create(pos, float_rand(0.0, 2 * M_PI));
    component->rectangle_collider[i] = RectangleColliderComponent_create(1.5, 0.25);
    //component->physics[i] = PhysicsComponent_create(10.0, 0.0, 0.5, 10.0);
    component->weapon[i] = WeaponComponent_create(5.0, 15.0, 0.75);
    component->particle[i] = ParticleComponent_create(0.0, 0.0, 0.1, 100.0, 1, sfWhite);
}


void create_level(Component* component) {
    create_house(component, -20.0, 0.0);
    create_house(component, 20.0, 0.0);
    create_house(component, 10.0, 10.0);

    create_player(component, (sfVector2f) { 0.0, 0.0 });

    create_car(component, 0.0, -5.0);
    create_weapon(component, 0.0, 5.0);
}
