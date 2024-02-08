#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#include "navigation.h"
#include "util.h"
#include "heap.h"
#include "camera.h"
#include "grid.h"
#include "raycast.h"
#include "list.h"
#include "collider.h"


#define INFINITY 10000.0


int create_waypoint(ComponentData* components, sfVector2f pos) {
    int i = create_entity(components);
    CoordinateComponent_add(components, i, pos, 0.0);
    WaypointComponent_add(components, i);
    ColliderComponent_add_circle(components, i, 0.25f, GROUP_WAYPOINTS);

    return i;
}


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

        for (ListNode* node = waypoint->neighbors->head; node; node = node->next) {
            int n = node->value;
            
            WaypointComponent* neighbor = components->waypoint[n];

            float d = heuristic(components, current, n);

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

    if (d > WaypointComponent_get(component, i)->range) {
        return 0.0f;
    }

    HitInfo info_a = raycast(component, grid, a, v, 1.0f * d, GROUP_RAYS);

    if (info_a.entity != -1) {
        return 0.0f;
    }

    HitInfo info_b = raycast(component, grid, b, mult(-1.0f, v), 1.0f * d, GROUP_RAYS);

    if (info_b.entity != -1) {
        return 0.0f;
    }

    return d;
}


void update_waypoints(ComponentData* components, ColliderGrid* grid, int camera) {
    static int id = 0;
    id = (id + 1) % 30;
    if (id != 0) {
        return;
    }

    List* waypoints = List_create();

    List* list = get_entities(components, grid, get_position(components, camera), 50.0f);
    for (ListNode* node = list->head; node; node = node->next) {
        int i = node->value;
        WaypointComponent* waypoint = WaypointComponent_get(components, i);
        if (!waypoint) continue;

        List_clear(waypoint->neighbors);
        List_add(waypoints, i);
    }
    List_delete(list);

    for (ListNode* node = waypoints->head; node; node = node->next) {
        int i = node->value;
        for (ListNode* nod = waypoints->head; nod; nod = nod->next) {
            int n = nod->value;
            if (n == i) continue;

            float d = connection_distance(components, grid, i, n);
            if (d > 0.0f) {
                List_add(WaypointComponent_get(components, i)->neighbors, n);
            }
        }
    }

    List_delete(waypoints);
}


void draw_waypoints(ComponentData* components, sfRenderWindow* window, int camera, bool draw_neighbors) {
    sfCircleShape* shape = sfCircleShape_create();
    sfRectangleShape* line = sfRectangleShape_create();

    for (int i = 0; i < components->entities; i++) {
        WaypointComponent* waypoint = WaypointComponent_get(components, i);
        if (!waypoint) continue;
        if (ImageComponent_get(components, i)) continue;

        sfVector2f pos = get_position(components, i);
        float radius = ColliderComponent_get(components, i)->radius;
        draw_circle(window, components, camera, shape, pos, radius, sfWhite);

        if (draw_neighbors) {
            for (ListNode* node = waypoint->neighbors->head; node; node = node->next) {
                int k = node->value;
                sfColor color = sfWhite;
                color.a = 64;
                draw_line(window, components, camera, line, pos, get_position(components, k), 0.04f, color);
            }
        }
    }

    sfCircleShape_destroy(shape);
    sfRectangleShape_destroy(line);
}
