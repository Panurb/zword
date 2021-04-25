#pragma once

#include <SFML/System/Vector2.h>

#include "component.h"
#include "camera.h"
#include "grid.h"
#include "raycast.h"


static int COLLISION_MATRIX[12][12] = { { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },     // WALLS
                                        { 1, 2, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0 },     // ITEMS
                                        { 1, 0, 2, 2, 1, 2, 0, 1, 0, 0, 0, 0 },     // PLAYERS
                                        { 1, 0, 2, 2, 1, 2, 0, 0, 0, 0, 0, 0 },     // ENEMIES
                                        { 1, 0, 0, 0, 1, 1, 3, 1, 0, 0, 0, 0 },     // VEHICLES
                                        { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },     // TREES
                                        { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },     // ROADS
                                        { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },     // RIVERS
                                        { 1, 0, 1, 1, 1, 0, 0, 0, 0, 0, 1, 0 },     // BULLETS
                                        { 1, 0, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0 },     // LIGHTS
                                        { 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },     // CORPSES
                                        { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0} };    // FLOORS


void get_corners(ComponentData* component, int i, sfVector2f* corners) ;

sfVector2f half_width(ComponentData* component, int i);

sfVector2f half_height(ComponentData* component, int i);

float axis_half_width(ComponentData* component, int i, sfVector2f axis);

bool collides_with(ComponentData* components, ColliderGrid* grid, int entity, ColliderGroup group);

void collide(ComponentData* component, ColliderGrid* collision_grid);

void draw_occupied_tiles(ComponentData* components, ColliderGrid* grid, sfRenderWindow* window, int camera);

void debug_draw(ComponentData* component, sfRenderWindow* window, int camera);

void damage(ComponentData* components, int entity, sfVector2f pos, sfVector2f dir, int dmg);
