#pragma once

#include <SFML/System/Vector2.h>

#include "component.h"
#include "camera.h"
#include "grid.h"
#include "raycast.h"


//                                                             V        B              
//                                                       P  E  E        A  B     C     
//                                                       L  N  H        R  U  L  O  F  V     D     E
//                                                 W  I  A  E  I  T  R  R  L  I  R  L  I     E  D  N
//                                                 A  T  Y  M  C  R  O  I  L  G  P  O  S  R  B  O  E
//                                              A  L  E  E  I  L  E  A  E  E  H  S  O  I  A  R  O  R
//                                              L  L  M  R  E  E  E  D  R  T  T  E  R  O  Y  I  R  G
//                                              L  S  S  S  S  S  S  S  S  S  S  S  S  N  S  S  S  Y
static const int COLLISION_MATRIX[18][18] = { { 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 },     // ALL
                                              { 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },     // WALLS
                                              { 1, 1, 2, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0 },     // ITEMS
                                              { 1, 1, 0, 2, 2, 1, 2, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0 },     // PLAYERS
                                              { 1, 1, 0, 2, 2, 1, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },     // ENEMIES
                                              { 1, 1, 0, 0, 0, 0, 1, 3, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0 },     // VEHICLES
                                              { 1, 1, 0, 0, 0, 1, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0 },     // TREES
                                              { 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },     // ROADS
                                              { 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },     // BARRIERS
                                              { 1, 1, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0 },     // BULLETS
                                              { 1, 1, 0, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0 },     // LIGHTS
                                              { 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },     // CORPSES
                                              { 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },     // FLOORS
                                              { 1, 1, 0, 1, 1, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },     // VISION
                                              { 1, 1, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },     // RAYS
                                              { 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },     // DEBRIS
                                              { 1, 0, 0, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },     // DOORS
                                              { 1, 1, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0 } };   // ENERGY


bool point_inside_collider(ComponentData* components, int i, sfVector2f point);

void get_corners(ComponentData* components, int i, sfVector2f* corners) ;

sfVector2f half_width(ComponentData* components, int i);

sfVector2f half_height(ComponentData* components, int i);

float axis_half_width(ComponentData* component, int i, sfVector2f axis);

sfVector2f overlap_rectangle_rectangle(ComponentData* components, int i, int j);

sfVector2f overlap_collider_collider(ComponentData* components, int i, int j);

sfVector2f overlap_rectangle_image(ComponentData* components, int i, int j);

bool collides_with(ComponentData* components, ColliderGrid* grid, int entity, List* entities);

void collide(ComponentData* component, ColliderGrid* collision_grid);

void draw_occupied_tiles(ComponentData* components, ColliderGrid* grid, sfRenderWindow* window, int camera);

void draw_colliders(ComponentData* component, sfRenderWindow* window, int camera);
