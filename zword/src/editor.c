#include <math.h>

#include "editor.h"
#include "widget.h"
#include "component.h"
#include "util.h"
#include "collider.h"
#include "navigation.h"
#include "serialize.h"


typedef enum {
    TOOL_SELECT,
    TOOL_WALL
} Tool;


static int selection_box = -1;
static sfVector2f selection_start = { 0.0f, 0.0f };
static sfVector2f selection_end = { 0.0f, 0.0f };
static List* selections = NULL;
static Tool tool = TOOL_SELECT;
static sfVector2f mouse_world = { 0.0f, 0.0f };


void save_prefab(ComponentData* components, Filename filename) {
    sfVector2f center = zeros();
    ListNode* node;
    FOREACH(node, selections) {
        int i = node->value;
        if (CoordinateComponent_get(components, i)->parent == -1) {
            center = sum(center, get_position(components, i));
        }
    }
    center = snap_to_grid(center);
    center = mult(-1.0f, center);

    cJSON* json = serialize_entities(components, selections, center);

    save_json(json, "prefabs", filename);
    cJSON_Delete(json);
}


void load_prefab(GameData* data, Filename filename, sfVector2f position, float angle) {
    cJSON* json = load_json("prefabs", filename);

    deserialize_entities(json, data, position, angle);

    cJSON_Delete(json);
}


void move_selections(GameData* data, sfVector2f delta_pos) {
    if (non_zero(delta_pos)) {
        ListNode* node;
        FOREACH (node, selections) {
            int i = node->value;
            clear_grid(data->components, data->grid, i);
            CoordinateComponent* coord = CoordinateComponent_get(data->components, node->value);
            coord->position = sum(coord->position, delta_pos);
            update_grid(data->components, data->grid, i);
        }
        delta_pos = zeros();
    }
}


void destroy_selections(GameData* data) {
    ListNode* node;
    FOREACH (node, selections) {
        int i = node->value;
        clear_grid(data->components, data->grid, i);
        destroy_entity_recursive(data->components, i);
    }
    List_clear(selections);
}


void update_selections(GameData data) {
    if (!selections) {
        selections = List_create();
    }
    List_clear(selections);

    List* collisions = List_create();
    collides_with(data.components, data.grid, selection_box, collisions);

    ListNode* node;
    FOREACH (node, collisions) {
        int i = node->value;
        if (CoordinateComponent_get(data.components, i)->parent == -1) {
            List_add(selections, i);
        }
    }
    List_delete(collisions);
}


void update_editor(GameData data, sfRenderWindow* window, float time_step) {
    UNUSED(time_step);

    update_widgets(data.components, window, data.camera);

    if (selection_box != -1) {
        update_selections(data);
    }
}


void input_tool_select(GameData* data, sfEvent event) {
    static bool grabbed = false;

    ComponentData* components = data->components;

    if (event.type == sfEvtMouseMoved) {
        if (selection_box != -1) {
            CoordinateComponent* coord = CoordinateComponent_get(components, selection_box);
            ColliderComponent* collider = ColliderComponent_get(components, selection_box);
            collider->width = fabsf(mouse_world.x - selection_start.x);
            collider->height = fabsf(mouse_world.y - selection_start.y);
            coord->position = mult(0.5f, sum(selection_start, mouse_world));
        }

        if (grabbed) {
            float dx = mouse_world.x - selection_start.x;
            dx = TILE_WIDTH * floorf(dx / TILE_WIDTH);
            float dy = mouse_world.y - selection_start.y;
            dy = TILE_HEIGHT * floorf(dy / TILE_HEIGHT);
            sfVector2f delta_pos = vec(dx, dy);
            selection_start = sum(selection_start, vec(dx, dy));

            move_selections(data, delta_pos);
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
                destroy_selections(data);
            }
        }
    } else if (event.type == sfEvtMouseButtonReleased && event.mouseButton.button == sfMouseLeft) {
        if (selection_box != -1) {
            destroy_entity(components, selection_box);
            selection_box = -1;
        }
        grabbed = false;
    }
}


void create_prefab(GameData* data, sfVector2f position, float width, float height) {
    int i = create_entity(data->components);
    CoordinateComponent_add(data->components, i, position, 0.0f);
    ColliderComponent_add_rectangle(data->components, i, width, height, GROUP_WALLS);
}


void input_tool_wall(GameData* data, sfEvent event) {
    static ButtonText wall_name;

    if (event.type == sfEvtMouseMoved) {
        selection_end = snap_to_grid(mouse_world);
    } else if (event.type == sfEvtMouseButtonPressed) {
        if (event.mouseButton.button == sfMouseLeft) {
            selection_start = snap_to_grid(mouse_world);
        } else if (event.mouseButton.button == sfMouseRight) {

        }
    } else if (event.type == sfEvtMouseButtonReleased && event.mouseButton.button == sfMouseLeft) {
        float width = fabsf(selection_end.x - selection_start.x);
        float height = fabsf(selection_end.y - selection_start.y);
        sfVector2f pos = mult(0.5f, sum(selection_end, selection_start));
        if (width > 0.0f && height > 0.0f) {
            create_prefab(data, pos, width, height);
        }
    }
}


void input_editor(GameData* data, sfRenderWindow* window, sfEvent event) {
    UNUSED(window);
    static sfVector2i mouse_screen = { 0, 0 };

    ComponentData* components = data->components;

    CoordinateComponent* cam_coord = CoordinateComponent_get(components, data->camera);
    CameraComponent* cam = CameraComponent_get(components, data->camera);

    switch (tool) {
        case TOOL_SELECT:
            input_tool_select(data, event);
            break;
        case TOOL_WALL:
            input_tool_wall(data, event);
            break;
    }

    if (event.type == sfEvtKeyPressed) {
        if (event.key.code == sfKeyS) {
            if (selections) {
                save_prefab(components, "testi.json");
            }
        }
        if (event.key.code == sfKeyL) {
            load_prefab(data, "testi.json", snap_to_grid(mouse_world), 0.0f);
        }
    }
    if (event.type == sfEvtMouseMoved) {
        sfVector2i mouse_new = { event.mouseMove.x, event.mouseMove.y };
        sfVector2f mouse_delta = { mouse_new.x - mouse_screen.x, mouse_screen.y - mouse_new.y };
        mouse_screen = mouse_new;
        mouse_world = screen_to_world(components, data->camera, mouse_screen);

        if (sfMouse_isButtonPressed(sfMouseMiddle)) {
            cam_coord->position = sum(cam_coord->position, mult(-1.0f / cam->zoom, mouse_delta));
        }
    } else if (event.type == sfEvtMouseWheelScrolled) {
        cam->zoom_target = clamp(cam->zoom_target * powf(1.5f, event.mouseWheelScroll.delta), 10.0f, 100.0f);
    }

    // input_widgets(components, camera, event);
}


void draw_editor(GameData data, sfRenderWindow* window) {
    draw_game(data, window);
    draw_grid(data.components, data.grid, window, data.camera);
    // draw_widgets(data.components, window, data.camera);

    sfVector2f pos = screen_to_world(data.components, data.camera, sfMouse_getPosition((sfWindow*) window));
    draw_circle(window, data.components, data.camera, NULL, pos, 0.1f, sfWhite);

    if (tool == TOOL_SELECT) {
        if (selection_box != -1) {
            pos = get_position(data.components, selection_box);
            ColliderComponent* collider = ColliderComponent_get(data.components, selection_box);
            draw_rectangle_outline(window, data.components, data.camera, NULL, pos, collider->width, collider->height, 0.0f, 
                0.05f, sfWhite);
        }
    } else if (tool == TOOL_WALL) {
        if (sfMouse_isButtonPressed(sfMouseLeft)) {
            sfVector2f end = screen_to_world(data.components, data.camera, sfMouse_getPosition((sfWindow*)window));
            end = snap_to_grid(end);
            float width = fabsf(end.x - selection_start.x);
            float height = fabsf(end.y - selection_start.y);
            pos = mult(0.5f, sum(end, selection_start));
            draw_rectangle_outline(window, data.components, data.camera, NULL, pos, width, height, 0.0f, 0.05f, sfWhite);
        }
    }

    bool waypoint_selected = false;
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

            if (WaypointComponent_get(data.components, i)) {
                waypoint_selected = true;
            }
        }
    }

    draw_waypoints(data.components, window, data.camera, waypoint_selected);
}
