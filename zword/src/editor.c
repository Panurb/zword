#include <math.h>

#include "editor.h"
#include "widget.h"
#include "component.h"
#include "util.h"
#include "collider.h"
#include "navigation.h"


static int selection_box = -1;
static List* selections = NULL;
static sfVector2f delta_pos = { 0.0f, 0.0f };
static bool destroy_selections = false;


void update_editor(GameData data, sfRenderWindow* window, float time_step) {
    UNUSED(time_step);

    update_widgets(data.components, window, data.camera);

    if (selection_box != -1) {
        if (!selections) {
            selections = List_create();
        }
        List_clear(selections);
        collides_with(data.components, data.grid, selection_box, selections);

        ListNode* node;
        FOREACH (node, selections) {
            int i = node->value;
            if (CoordinateComponent_get(data.components, i)->parent != -1) {
                List_remove(selections, i);
            }
        }
    }

    if (selections) {
        if (non_zero(delta_pos)) {
            ListNode* node;
            FOREACH (node, selections) {
                int i = node->value;
                clear_grid(data.components, data.grid, i);
                CoordinateComponent* coord = CoordinateComponent_get(data.components, node->value);
                coord->position = sum(coord->position, delta_pos);
                update_grid(data.components, data.grid, i);
            }
            delta_pos = zeros();
        }
    }

    if (destroy_selections) {
        ListNode* node;
        FOREACH (node, selections) {
            int i = node->value;
            clear_grid(data.components, data.grid, i);
            destroy_entity_recursive(data.components, i);
        }
        List_clear(selections);
        destroy_selections = false;
    }
}


void input_editor(ComponentData* components, int camera, sfEvent event) {
    static sfVector2i mouse_screen;
    static sfVector2f selection_start;
    static bool grabbed = false;

    bool mouse_middle = sfMouse_isButtonPressed(sfMouseMiddle);
    sfVector2f mouse_world = screen_to_world(components, camera, mouse_screen);

    CoordinateComponent* cam_coord = CoordinateComponent_get(components, camera);
    CameraComponent* cam = CameraComponent_get(components, camera);

    if (event.type == sfEvtMouseMoved) {
        sfVector2i mouse_new = { event.mouseMove.x, event.mouseMove.y };
        sfVector2f mouse_delta = { mouse_new.x - mouse_screen.x, mouse_screen.y - mouse_new.y };
        mouse_screen = mouse_new;

        if (mouse_middle) {
            cam_coord->position = sum(cam_coord->position, mult(-1.0f / cam->zoom, mouse_delta));
        }

        if (selection_box != -1) {
            CoordinateComponent* coord = CoordinateComponent_get(components, selection_box);
            ColliderComponent* collider = ColliderComponent_get(components, selection_box);
            collider->width = fabsf(mouse_world.x - selection_start.x);
            collider->height = fabsf(mouse_world.y - selection_start.y);
            coord->position = mult(0.5f, sum(selection_start, mouse_world));
        }

        if (grabbed) {
            float dx = mouse_world.x - selection_start.x;
            dx = fabsf(dx) > TILE_WIDTH ? copysignf(TILE_WIDTH, dx) : 0.0f;
            float dy = mouse_world.y - selection_start.y;
            dy = fabsf(dy) > TILE_HEIGHT ? copysignf(TILE_WIDTH, dy) : 0.0f;
            delta_pos = sum(delta_pos, vec(dx, dy));
            selection_start = sum(selection_start, vec(dx, dy));
        }
    } else if (event.type == sfEvtMouseButtonPressed) {
        if (event.mouseButton.button == sfMouseLeft) {
            selection_start = mouse_world;

            if (selections) {
                ListNode* node;
                FOREACH (node, selections) {
                    int i = node->value;
                    if (inside_collider(components, i, mouse_world)) {
                        grabbed = true;
                        break;
                    }
                }
            }
            if (!grabbed) {
                selection_box = create_entity(components);
                CoordinateComponent_add(components, selection_box, selection_start, 0.0f);
                ColliderComponent_add_rectangle(components, selection_box, 0.0f, 0.0f, GROUP_ALL);
            }
        } else if (event.mouseButton.button == sfMouseRight) {
            if (selections) {
                destroy_selections = true;
            }
        }
    } else if (event.type == sfEvtMouseButtonReleased && event.mouseButton.button == sfMouseLeft) {
        destroy_entity(components, selection_box);
        selection_box = -1;
        grabbed = false;
    }

    if (event.type == sfEvtMouseWheelScrolled) {
        cam->zoom_target = clamp(cam->zoom_target * powf(1.5f, event.mouseWheelScroll.delta), 10.0f, 100.0f);
    }

    input_widgets(components, camera, event);
}


void draw_editor(GameData data, sfRenderWindow* window) {
    draw_game(data, window);
    draw_grid(data.components, data.grid, window, data.camera);
    draw_waypoints(data.components, window, data.camera);
    // draw_widgets(data.components, window, data.camera);

    sfVector2f pos = screen_to_world(data.components, data.camera, sfMouse_getPosition((sfWindow*) window));
    draw_circle(window, data.components, data.camera, NULL, pos, 0.1f, sfWhite);

    if (selection_box != -1) {
        pos = get_position(data.components, selection_box);
        ColliderComponent* collider = ColliderComponent_get(data.components, selection_box);
        draw_rectangle_outline(window, data.components, data.camera, NULL, pos, collider->width, collider->height, 0.0f, 
            0.05f, sfWhite);
    }

    if (selections) {
        ListNode* node;
        FOREACH (node, selections) {
            int i = node->value;
            pos = get_position(data.components, i);
            float angle = get_angle(data.components, i);

            ImageComponent* image = ImageComponent_get(data.components, i);
            if (image) {
                draw_sprite(window, data.components, data.camera, image->sprite, pos, angle, image->scale, 1);
            }

            ColliderComponent* collider = ColliderComponent_get(data.components, i);
            draw_rectangle_outline(window, data.components, data.camera, NULL, pos, collider->width, collider->height, 
                angle, 0.05f, sfWhite);
        }
    }
}
