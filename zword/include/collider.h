#pragma once

#include <SFML/System/Vector2.h>

#include "component.h"
#include "camera.h"
#include "grid.h"
#include "raycast.h"


//                                                                                  W
//                                                          V                       A
//                                                    P  E  E           B     C     Y
//                                                    L  N  H        R  U  L  O  F  P     D
//                                              W  I  A  E  I  T  R  I  L  I  R  L  O     E
//                                              A  T  Y  M  C  R  O  V  L  G  P  O  I  R  B
//                                              L  E  E  I  L  E  A  E  E  H  S  O  N  A  R
//                                              L  M  R  E  E  E  D  R  T  T  E  R  T  Y  I
//                                              S  S  S  S  S  S  S  S  S  S  S  S  S  S  S
static const int COLLISION_MATRIX[15][15] = { { 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },     // WALLS
                                              { 1, 2, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0 },     // ITEMS
                                              { 1, 0, 2, 2, 1, 2, 0, 1, 0, 0, 0, 0, 0, 0, 0 },     // PLAYERS
                                              { 1, 0, 2, 2, 1, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0 },     // ENEMIES
                                              { 1, 0, 0, 0, 1, 1, 3, 1, 0, 0, 0, 0, 0, 0, 0 },     // VEHICLES
                                              { 1, 0, 0, 0, 1, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0 },     // TREES
                                              { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },     // ROADS
                                              { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },     // RIVERS
                                              { 1, 0, 1, 1, 1, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0 },     // BULLETS
                                              { 1, 0, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0 },     // LIGHTS
                                              { 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },     // CORPSES
                                              { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },     // FLOORS
                                              { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0 },     // WAYPOINTS
                                              { 1, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },     // RAYS
                                              { 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 } };   // DEBRIS


bool inside_collider(ComponentData* components, int i, sfVector2f point);

void get_corners(ComponentData* component, int i, sfVector2f* corners) ;

sfVector2f half_width(ComponentData* component, int i);

sfVector2f half_height(ComponentData* component, int i);

float axis_half_width(ComponentData* component, int i, sfVector2f axis);

bool collides_with(ComponentData* components, ColliderGrid* grid, int entity);

void collide(ComponentData* component, ColliderGrid* collision_grid);

void draw_occupied_tiles(ComponentData* components, ColliderGrid* grid, sfRenderWindow* window, int camera);

void debug_draw(ComponentData* component, sfRenderWindow* window, int camera);
