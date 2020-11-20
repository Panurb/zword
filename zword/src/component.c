#include "component.h"
#include "image.h"


CoordinateComponent* CoordinateComponent_create(float x, float y, float angle) {
    CoordinateComponent* coord = malloc(sizeof(CoordinateComponent));
    coord->position = (sfVector2f) { x, y };
    coord->angle = angle;
    return coord;
}


ImageComponent* ImageComponent_create(char filename[20], float scale) {
    ImageComponent* image = malloc(sizeof(ImageComponent));
    image->scale = (sfVector2f) { scale, scale };
    image->sprite = load_sprite(filename);
    return image;
}


PhysicsComponent* PhysicsComponent_create(float mass, float friction, float bounce) {
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
    return phys;
}

CircleColliderComponent* CircleColliderComponent_create(float radius) {
    CircleColliderComponent* col = malloc(sizeof(CircleColliderComponent));
    col->radius = radius;
    col->shape = sfCircleShape_create();
    sfCircleShape_setFillColor(col->shape, sfColor_fromRGB(255, 0, 255));
    return col;
}

RectangleColliderComponent* RectangleColliderComponent_create(float width, float height) {
    RectangleColliderComponent* col = malloc(sizeof(RectangleColliderComponent));
    col->width = width;
    col->height = height;
    col->shape = sfRectangleShape_create();
    sfRectangleShape_setFillColor(col->shape, sfColor_fromRGB(0, 255, 255));
    return col;
}

PlayerComponent* PlayerComponent_create() {
    PlayerComponent* player = malloc(sizeof(PlayerComponent));
    player->health = 100;
    player->max_speed = 0.5;
    player->acceleration = 5.0;
    player->cooldown = 0.0;
    return player;
}

void destroy_entity(Component* component, int i) {
    if (component->coordinate[i]) {
        free(component->coordinate[i]);
    }
    if (component->image[i]) {
        free(component->image[i]);
    }
    if (component->physics[i]) {
        free(component->physics[i]);
    }
    if (component->circle_collider[i]) {
        free(component->circle_collider[i]);
    }
    if (component->rectangle_collider[i]) {
        free(component->rectangle_collider[i]);
    }
    if (component->player[i]) {
        free(component->player[i]);
    }

    if (i == component->entities - 1) {
        component->entities--;
    }
}
