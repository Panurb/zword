#include <stdbool.h>

#include "navigation.h"
#include "util.h"
#include "heap.h"
#include "camera.h"
#include "grid.h"


#define INFINITY 10000


void reconstruct_path(Component* component, int current, int* path) {
    path[0] = current;

    int i = 1;
    while(current != -1) {
        current = component->waypoint[current]->came_from;
        path[i] = current;
        i++;
    }
}


float heuristic(Component* component, int start, int goal) {
    return dist(get_position(component, start), get_position(component, goal));
}


bool a_star(Component* component, int start, int goal, int* path) {
    Heap* open_set = Heap_create(component);
    Heap_insert(open_set, start);

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
            WaypointComponent* neighbor = component->waypoint[n];

            float d = component->waypoint[current]->weights[i];

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


void draw_waypoints(Component* component, sfRenderWindow* window, Camera* camera) {
    float radius = 0.5 * camera->zoom;
    for (int i = 0; i < component->entities; i++) {
        if (!component->waypoint[i]) continue;

        sfCircleShape* shape = sfCircleShape_create();

        sfCircleShape_setOrigin(shape, (sfVector2f) { radius, radius });

        sfVector2f pos = get_position(component, i);
        sfCircleShape_setPosition(shape, world_to_screen(pos, camera));

        sfCircleShape_setRadius(shape, radius);

        sfRenderWindow_drawCircleShape(window, shape, NULL);
    }
}
