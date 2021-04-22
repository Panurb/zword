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


void reconstruct_path(ComponentData* components, int current, List* path) {
    while(current != -1) {
        List_add(path, current);
        current = components->waypoint[current]->came_from;
    }
}


float heuristic(ComponentData* components, int start, int goal) {
    return dist(get_position(components, start), get_position(components, goal));
}


bool a_star(ComponentData* components, int start, int goal, List* path) {
    Heap* open_set = Heap_create(components);

    Heap_insert(open_set, start);

    List_clear(path);

    for (int i = 0; i < components->entities; i++) {
        if (components->waypoint[i]) {
            components->waypoint[i]->came_from = -1;
            components->waypoint[i]->g_score = INFINITY;
            components->waypoint[i]->f_score = INFINITY;
        }
    }

    components->waypoint[start]->g_score = 0.0;
    components->waypoint[start]->f_score = heuristic(components, start, goal);

    while (open_set->size > 0) {
        int current = Heap_extract(open_set);

        if (current == goal) {
            reconstruct_path(components, current, path);
            return true;
        }

        WaypointComponent* waypoint = components->waypoint[current];

        for (int i = 0; i < waypoint->neighbors_size; i++) {
            int n = waypoint->neighbors[i];

            if (n == -1) continue;
            
            WaypointComponent* neighbor = components->waypoint[n];

            float d = waypoint->weights[i];
            // float d = heuristic(components, current, n);

            float tentative_g_score = waypoint->g_score + d;

            if (tentative_g_score < neighbor->g_score) {
                neighbor->came_from = current;
                neighbor->g_score = tentative_g_score;
                neighbor->f_score = neighbor->g_score + heuristic(components, n, goal);
                
                if (Heap_find(open_set, n) == -1) {
                    Heap_insert(open_set, n);
                }
            }
        }
    }

    return false;
}


float connection_distance(ComponentData* component, ColliderGrid* grid, int i, int j) {
    sfVector2f a = get_position(component, i);
    sfVector2f b = get_position(component, j);
    sfVector2f v = diff(b, a);
    float d = norm(v);

    if (d > component->waypoint[i]->range) {
        return 0.0;
    }

    HitInfo info = raycast(component, grid, a, v, 1.1f * d, i, BULLETS);

    if (info.object == j || info.object == -1) {
        return d;
    }

    return 0.0;
}


void init_waypoints(ComponentData* components, ColliderGrid* grid) {
    for (int i = 0; i < components->entities; i++) {
        WaypointComponent* waypoint = WaypointComponent_get(components, i);
        if (!waypoint) continue;

        for (int j = i + 1; j < components->entities; j++) {
            WaypointComponent* neighbor = WaypointComponent_get(components, j);
            if (!neighbor) continue;

            float d = connection_distance(components, grid, i, j);
            if (d > 0.0) {
                int k = replace(-1, j, waypoint->neighbors, MAX_NEIGHBORS);
                if (k != -1) {
                    waypoint->weights[k] = d;
                    waypoint->neighbors_size++;
                }

                k = replace(-1, i, neighbor->neighbors, MAX_NEIGHBORS);
                if (k != -1) {
                    neighbor->weights[k] = d;
                    neighbor->neighbors_size++;
                }
            }
        }
    }
}


void clear_waypoints(ComponentData* components) {
    for (int i = 0; i < components->entities; i++) {
        WaypointComponent* waypoint = WaypointComponent_get(components, i);
        if (!waypoint) continue;

        if (components->physics[i]) {
            waypoint->neighbors_size = 0;
        } else {
            for (int j = 0; j < waypoint->neighbors_size; j++) {
                int n = waypoint->neighbors[j];
                if (PhysicsComponent_get(components, n)) {
                    waypoint->neighbors_size = j;
                    break;
                }
            }
        }
    }
}


void update_waypoints(ComponentData* components, ColliderGrid* grid) {
    clear_waypoints(components);

    for (int i = 0; i < components->entities; i++) {
        WaypointComponent* waypoint = WaypointComponent_get(components, i);
        if (!waypoint) continue;

        List* list = get_entities(components, grid, get_position(components, i), waypoint->range);
        for (ListNode* current = list->head; current != NULL; current = current->next) {
            int n = current->value;
            if (n == i) continue;
            if (!PhysicsComponent_get(components, n)) continue;

            WaypointComponent* neighbor = WaypointComponent_get(components, n);
            if (!neighbor) continue;

            float d = connection_distance(components, grid, i, n);
            if (d > 0.0) {
                int k = waypoint->neighbors_size;
                int l = neighbor->neighbors_size;
                if (k < MAX_NEIGHBORS && l < MAX_NEIGHBORS) {
                    waypoint->neighbors[k] = n;
                    waypoint->weights[k] = d;
                    waypoint->neighbors_size++;

                    neighbor->neighbors[l] = i;
                    neighbor->weights[l] = d;
                    neighbor->neighbors_size++;
                }
            }
        }

        List_delete(list);
    }
}


void draw_waypoints(ComponentData* components, sfRenderWindow* window, int camera) {
    sfCircleShape* shape = sfCircleShape_create();
    sfRectangleShape* line = sfRectangleShape_create();

    float radius = 0.2 * CameraComponent_get(components, camera)->zoom;
    sfCircleShape_setOrigin(shape, (sfVector2f) { radius, radius });
    sfCircleShape_setRadius(shape, radius);

    for (int i = 0; i < components->entities; i++) {
        WaypointComponent* waypoint = WaypointComponent_get(components, i);
        if (!waypoint) continue;

        sfVector2f pos = get_position(components, i);
        sfCircleShape_setPosition(shape, world_to_screen(components, camera, pos));

        sfRenderWindow_drawCircleShape(window, shape, NULL);

        for (int j = 0; j < waypoint->neighbors_size; j++) {
            int k = waypoint->neighbors[j];
            if (k != -1) {
                sfColor color = sfWhite;
                color.a = 64;
                draw_line(window, components, camera, line, pos, get_position(components, k), 0.02, color);
            }
        }
    }

    sfCircleShape_destroy(shape);
    sfRectangleShape_destroy(line);
}
