#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#include "navigation.h"
#include "util.h"
#include "heap.h"
#include "camera.h"
#include "grid.h"
#include "raycast.h"


#define INFINITY 10000.0


void reconstruct_path(Component* component, int current, int* path) {
    int i = 0;
    while(current != -1) {
        path[i] = current;
        current = component->waypoint[current]->came_from;
        i++;
    }
}


float heuristic(Component* component, int start, int goal) {
    return dist(get_position(component, start), get_position(component, goal));
}


bool a_star(Component* component, int start, int goal, int* path) {
    Heap* open_set = Heap_create(component);
    Heap_insert(open_set, start);

    for (int i = 0; i < MAX_PATH_LENGTH; i++) {
        path[i] = -1;
    }

    for (int i = 0; i < MAX_ENTITIES; i++) {
        if (component->waypoint[i]) {
            component->waypoint[i]->came_from = -1;
            component->waypoint[i]->g_score = INFINITY;
            component->waypoint[i]->f_score = INFINITY;
        }
    }

    component->waypoint[start]->g_score = 0.0;

    component->waypoint[start]->f_score = heuristic(component, start, goal);

    while (open_set->size > 0) {
        int current = Heap_extract(open_set);

        if (current == goal) {
            reconstruct_path(component, current, path);
            return true;
        }

        for (int i = 0; i < MAX_NEIGHBORS; i++) {
            int n = component->waypoint[current]->neighbors[i];

            if (n == -1) continue;
            
            WaypointComponent* neighbor = component->waypoint[n];

            //float d = component->waypoint[current]->weights[i];
            float d = dist(get_position(component, current), get_position(component, n));

            float tentative_g_score = component->waypoint[current]->g_score + d;

            if (tentative_g_score < neighbor->g_score) {
                neighbor->came_from = current;
                neighbor->g_score = tentative_g_score;
                neighbor->f_score = neighbor->g_score + heuristic(component, n, goal);
                
                if (Heap_find(open_set, n) == -1) {
                    Heap_insert(open_set, n);
                }
            }
        }
    }

    return false;
}


float connection_distance(Component* component, ColliderGrid* grid, int i, int j) {
    sfVector2f a = get_position(component, i);
    sfVector2f b = get_position(component, j);
    sfVector2f v = diff(b, a);
    float d = norm(v);
    HitInfo info = raycast(component, grid, a, v, 1.1 * d);

    if (info.object == j) {
        return d;
    }

    return 0.0;
}


void init_waypoints(Component* component, ColliderGrid* grid) {
    for (int i = 0; i < component->entities; i++) {
        if (!component->waypoint[i]) continue;

        for (int j = 0; j < component->entities; j++) {
            if (i == j) continue;

            if (!component->waypoint[j]) continue;

            float d = connection_distance(component, grid, i, j);
            if (d > 0.0) {
                int k = replace(-1, j, component->waypoint[i]->neighbors, MAX_NEIGHBORS);
                if (k != -1) {
                    component->waypoint[i]->weights[k] = d;
                }
            }
        }
    }

    for (int i = 0; i < component->entities; i++) {
        if (!component->waypoint[i]) continue;
        if (component->physics[i]) continue;

        free(component->circle_collider[i]);
        component->circle_collider[i] = NULL;
    }
}


void update_waypoints(Component* component, ColliderGrid* grid) {
    for (int i = 0; i < component->entities; i++) {
        if (!component->waypoint[i]) continue;

        for (int j = 0; j < MAX_NEIGHBORS; j++) {
            int n = component->waypoint[i]->neighbors[j];
            
            if (n == -1) continue;
            if (!component->physics[n]) continue;

            if (connection_distance(component, grid, i, n) == 0.0) {
                component->waypoint[i]->neighbors[j] = -1;

                replace(i, -1, component->waypoint[n]->neighbors, MAX_NEIGHBORS);
            }
        }

        for (int j = 0; j < component->entities; j++) {
            if (i == j) continue;
            if (!component->waypoint[j]) continue;
            if (!component->physics[j]) continue;

            if (find(j, component->waypoint[i]->neighbors, MAX_NEIGHBORS) != -1) {
                continue;
            }

            float d = connection_distance(component, grid, i, j);
            if (d > 0.0) {
                int k = replace(-1, j, component->waypoint[i]->neighbors, MAX_NEIGHBORS);
                if (k != -1) {
                    component->waypoint[i]->weights[k] = d;

                    int l = replace(-1, i, component->waypoint[j]->neighbors, MAX_NEIGHBORS);
                    if (l != -1) {
                        component->waypoint[j]->weights[l] = d;
                    }
                }
            }
        }
    }
}


void draw_waypoints(Component* component, sfRenderWindow* window, Camera* camera) {
    sfCircleShape* shape = sfCircleShape_create();
    sfRectangleShape* line = sfRectangleShape_create();

    float radius = 0.2 * camera->zoom;
    for (int i = 0; i < component->entities; i++) {
        if (!component->waypoint[i]) continue;
        //if (!component->enemy[i]) continue;

        sfCircleShape_setOrigin(shape, (sfVector2f) { radius, radius });

        sfVector2f pos = get_position(component, i);
        sfCircleShape_setPosition(shape, world_to_screen(pos, camera));

        sfCircleShape_setRadius(shape, radius);

        sfRenderWindow_drawCircleShape(window, shape, NULL);

        for (int j = 0; j < MAX_NEIGHBORS; j++) {
            int k = component->waypoint[i]->neighbors[j];
            if (k != -1) {
                //draw_line(window, camera, line, pos, get_position(component, k), 0.02, sfWhite);
            }
        }
    }

    sfCircleShape_destroy(shape);
    sfRectangleShape_destroy(line);
}
