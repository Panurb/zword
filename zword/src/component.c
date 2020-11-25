#define _USE_MATH_DEFINES

#include <stdbool.h>
#include <stdlib.h>
#include <math.h>

#include <SFML/Graphics.h>
#include <SFML/System/Vector2.h>

#include "component.h"
#include "image.h"
#include "util.h"


CoordinateComponent* CoordinateComponent_create(sfVector2f pos, float angle) {
    CoordinateComponent* coord = malloc(sizeof(CoordinateComponent));
    coord->position = pos;
    coord->angle = angle;
    return coord;
}


ImageComponent* ImageComponent_create(char filename[20], float scale) {
    ImageComponent* image = malloc(sizeof(ImageComponent));
    image->scale = (sfVector2f) { scale, scale };
    image->sprite = load_sprite(filename);
    return image;
}


PhysicsComponent* PhysicsComponent_create(float mass, float friction, float bounce, float drag) {
    PhysicsComponent* phys = malloc(sizeof(PhysicsComponent));
    phys->velocity = (sfVector2f) { 0, 0 };
    phys->acceleration = (sfVector2f) { 0, 0 };
    phys->collision.overlap = (sfVector2f) { 0, 0 };
    phys->collision.velocity = (sfVector2f) { 0.0, 0.0} ;
    phys->angular_velocity = 0.0;
    phys->angular_acceleration = 0.0;
    phys->mass = mass;
    phys->friction = friction;
    phys->bounce = bounce;
    phys->drag = drag;
    phys->max_speed = 20.0;
    return phys;
}


CircleColliderComponent* CircleColliderComponent_create(float radius) {
    CircleColliderComponent* col = malloc(sizeof(CircleColliderComponent));
    col->radius = radius;
    col->shape = sfCircleShape_create();
    sfCircleShape_setFillColor(col->shape, sfColor_fromRGB(150, 0, 150));
    return col;
}


RectangleColliderComponent* RectangleColliderComponent_create(float width, float height) {
    RectangleColliderComponent* col = malloc(sizeof(RectangleColliderComponent));
    col->width = width;
    col->height = height;
    col->shape = sfRectangleShape_create();
    sfRectangleShape_setFillColor(col->shape, sfColor_fromRGB(50, 50, 50));
    return col;
}


PlayerComponent* PlayerComponent_create() {
    PlayerComponent* player = malloc(sizeof(PlayerComponent));
    player->health = 100;
    player->acceleration = 20.0;
    player->cooldown = 0.0;
    return player;
}


LightComponent* LightComponent_create(float range, float angle, int rays, float brightness) {
    LightComponent* light = malloc(sizeof(LightComponent));
    light->range = range;
    light->angle = angle;
    light->rays = rays;
    light->color[0] = 255;
    light->color[1] = 255;
    light->color[2] = 255;
    light->brightness = brightness;
    light->smoothing = 0;
    light->shape = sfConvexShape_create();
    sfConvexShape_setPointCount(light->shape, 3);
    return light;
}


EnemyComponent* EnemyComponent_create() {
    EnemyComponent* enemy = malloc(sizeof(EnemyComponent));
    enemy->health = 100;
    enemy->acceleration = 15.0;
    enemy->target = -1;
    return enemy;
}


ParticleComponent* ParticleComponent_create() {
    ParticleComponent* particle = malloc(sizeof(ParticleComponent));
    particle->loop = false;
    particle->angle = 2 * M_PI;
    particle->particles = 0;
    particle->max_particles = 20;
    particle->iterator = 0;
    for (int i = 0; i < particle->max_particles; i++) {
        particle->position[i] = (sfVector2f) { 0.0, 0.0 };
        particle->velocity[i] = (sfVector2f) { 0.0, 0.0 };
    }
    particle->shape = sfCircleShape_create();
    sfCircleShape_setFillColor(particle->shape, sfColor_fromRGB(200, 0, 0));
    particle->rate = 0.0;
    particle->timer = 0.0;
    return particle;
}


void destroy_entity(Component* component, int i) {
    if (component->coordinate[i]) {
        free(component->coordinate[i]);
        component->coordinate[i] = NULL;
    }
    if (component->image[i]) {
        free(component->image[i]);
        component->image[i] = NULL;
    }
    if (component->physics[i]) {
        free(component->physics[i]);
        component->physics[i] = NULL;
    }
    if (component->circle_collider[i]) {
        free(component->circle_collider[i]);
        component->circle_collider[i] = NULL;
    }
    if (component->rectangle_collider[i]) {
        free(component->rectangle_collider[i]);
        component->rectangle_collider[i] = NULL;
    }
    if (component->player[i]) {
        free(component->player[i]);
        component->player[i] = NULL;
    }
    if (component->light[i]) {
        free(component->light[i]);
        component->light[i] = NULL;
    }
    if (component->enemy[i]) {
        free(component->enemy[i]);
        component->enemy[i] = NULL;
    }
    if (component->particle[i]) {
        free(component->particle[i]);
        component->particle[i] = NULL;
    }

    if (i == component->entities - 1) {
        component->entities--;
    }
}
