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
#include "game.h"


#define INFINITY 10000.0


int create_waypoint(Vector2f pos) {
    int i = create_entity();
    CoordinateComponent_add(i, pos, 0.0);
    WaypointComponent_add(i);
    ColliderComponent_add_circle(i, 0.25f, GROUP_ALL)->enabled = false;

    return i;
}


void reconstruct_path(int current, List* path) {
    while(current != -1) {
        List_add(path, current);
        WaypointComponent* waypoint = WaypointComponent_get(current);
        current = waypoint->came_from;
    }
}


float heuristic(int start, int goal) {
    return dist(get_position(start), get_position(goal));
}


bool a_star(int start, int goal, List* path) {
    Heap* open_set = Heap_create();

    Heap_insert(open_set, start);

    List_clear(path);

    for (int i = 0; i < game_data->components->entities; i++) {
        WaypointComponent* waypoint = WaypointComponent_get(i);
        if (waypoint) {
            waypoint->came_from = -1;
            waypoint->g_score = INFINITY;
            waypoint->f_score = INFINITY;
        }
    }

    WaypointComponent* start_waypoint = WaypointComponent_get(start);
    start_waypoint->g_score = 0.0;
    start_waypoint->f_score = heuristic(start, goal);

    while (open_set->size > 0) {
        int current = Heap_extract(open_set);

        if (current == goal) {
            reconstruct_path(current, path);
            Heap_destroy(open_set);
            return true;
        }

        WaypointComponent* waypoint = WaypointComponent_get(current);

        for (ListNode* node = waypoint->neighbors->head; node; node = node->next) {
            int n = node->value;
            
            WaypointComponent* neighbor = WaypointComponent_get(n);

            float d = heuristic(current, n);

            float tentative_g_score = waypoint->g_score + d;

            if (tentative_g_score < neighbor->g_score) {
                neighbor->came_from = current;
                neighbor->g_score = tentative_g_score;
                neighbor->f_score = neighbor->g_score + heuristic(n, goal);
                
                if (Heap_find(open_set, n) == -1) {
                    Heap_insert(open_set, n);
                }
            }
        }
    }

    Heap_destroy(open_set);
    return false;
}


float connection_distance(int i, int j) {
    Vector2f a = get_position(i);
    Vector2f b = get_position(j);
    Vector2f v = diff(b, a);
    float d = norm(v);

    if (d > WaypointComponent_get(i)->range) {
        return 0.0f;
    }

    EnemyComponent* enemy = EnemyComponent_get(i);

    if (enemy) {
        ColliderComponent* col = ColliderComponent_get(i);

        float angle = get_angle(i);
        float radius = 0.4f * collider_width(i);

        Vector2f left = polar_to_cartesian(radius, angle + 0.5f * M_PI);
        HitInfo info_l = raycast(sum(a, left), v, d, GROUP_RAYS);
        if (info_l.entity != -1) {
            return 0.0f;
        }

        Vector2f right = polar_to_cartesian(radius, angle - 0.5f * M_PI);
        HitInfo info_r = raycast(sum(a, right), v, d, GROUP_RAYS);
        if (info_r.entity != -1) {
            return 0.0f;
        }
    } else {
        HitInfo info_a = raycast(a, v, d, GROUP_RAYS);

        if (info_a.entity != -1) {
            return 0.0f;
        }
    }

    return d;
}


void update_waypoints(int camera, float range) {
    List* waypoints = List_create();

    if (range == INFINITY) {
        for (int i = 0; i < game_data->components->entities; i++) {
            WaypointComponent* waypoint = WaypointComponent_get(i);
            if (waypoint) {
                List_clear(waypoint->neighbors);
                List_add(waypoints, i);
            }
        }
    } else {
        List* list = get_entities(get_position(camera), range);
        for (ListNode* node = list->head; node; node = node->next) {
            int i = node->value;
            WaypointComponent* waypoint = WaypointComponent_get(i);
            if (!waypoint) continue;

            List_clear(waypoint->neighbors);
            List_add(waypoints, i);
        }
        List_delete(list);
    }

    for (ListNode* node = waypoints->head; node; node = node->next) {
        int i = node->value;
        for (ListNode* nod = waypoints->head; nod; nod = nod->next) {
            int n = nod->value;
            if (n == i) break;

            EnemyComponent* enemy = EnemyComponent_get(i);
            EnemyComponent* enemy_neighbor = EnemyComponent_get(n);

            if (enemy && enemy_neighbor) {
                continue;
            }

            float d = connection_distance(i, n);
            if (d > 0.0f) {
                if (!enemy_neighbor) {
                    List_add(WaypointComponent_get(i)->neighbors, n);
                }
                if (!enemy) {
                    List_add(WaypointComponent_get(n)->neighbors, i);
                }
            }
        }
    }

    List_delete(waypoints);
}


void draw_waypoints(int camera, bool draw_neighbors) {
    for (int i = 0; i < game_data->components->entities; i++) {
        WaypointComponent* waypoint = WaypointComponent_get(i);
        if (!waypoint) continue;
        if (ImageComponent_get(i)) continue;

        Vector2f pos = get_position(i);
        float radius = ColliderComponent_get(i)->radius;
        draw_circle(camera, pos, radius, COLOR_WHITE);

        if (draw_neighbors) {
            for (ListNode* node = waypoint->neighbors->head; node; node = node->next) {
                int k = node->value;
                Color color = COLOR_WHITE;
                color.a = 64;
                draw_line(camera, pos, get_position(k), 0.04f, color);
            }
        }
    }
}
