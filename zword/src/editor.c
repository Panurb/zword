#define _USE_MATH_DEFINES

#include <math.h>
#include <string.h>

#include "editor.h"
#include "widget.h"
#include "component.h"
#include "util.h"
#include "collider.h"
#include "navigation.h"
#include "serialize.h"
#include "building.h"


typedef enum {
    TOOL_SELECT,
    TOOL_WALL,
    TOOL_OBJECT,
    TOOL_PREFAB
} Tool;


static int selection_box = -1;
static sfVector2f selection_start = { 0.0f, 0.0f };
static sfVector2f selection_end = { 0.0f, 0.0f };
static List* selections = NULL;
static Tool tool = TOOL_WALL;
static sfVector2f mouse_world = { 0.0f, 0.0f };
static ButtonText prefabs[] = { "testi.json", "prefab.json" };
static Filename prefab_name = "";
static float grid_sizes[] = { 0.0f, 0.25f, 0.5f, 1.0f };
static int grid_size_index = 3;

static int selected_wall = 0;
static ButtonText wall_names[] = {
    "altar_tile",
    "board_tile",
    "brick_tile",
    "grass_tile",
    "roof_tile",
    "stone_tile",
    "tiles_tile",
    "water_tile",
    "wood_tile"
};

static int selected_object = 0;
static ButtonText objects[] = {
    "bed",
    "bench",
    "big boy",
    "boss",
    "candle",
    "car",
    "desk",
    "door",
    "farmer",
    "flashlight",
    "gas",
    "hay bale",
    "lamp"
};


sfVector2f get_selections_center(ComponentData* components) {
    sfVector2f center = zeros();
    ListNode* node;
    FOREACH(node, selections) {
        int i = node->value;
        if (CoordinateComponent_get(components, i)->parent == -1) {
            center = sum(center, get_position(components, i));
        }
    }
    center = mult(1.0f / selections->size, center);

    return center;
}


void select_object(ComponentData* components, int entity) {
    WidgetComponent* widget = WidgetComponent_get(components, entity);
    selected_object = widget->value;
    tool = TOOL_OBJECT;
}


int toggle_list_window(ComponentData* components, int window_id, ButtonText title, OnClick close, ButtonText* values, 
        int length, OnClick select) {
    if (window_id != -1) {
        destroy_entity_recursive(components, window_id);
        return -1;
    }

    sfVector2f pos = sum(vec(0.0f, 2 * BUTTON_HEIGHT), mult(BUTTON_HEIGHT, rand_vector()));
    window_id = create_window(components, pos, title, 1, close);

    int container = create_container(components, vec(0.0f, -3 * BUTTON_HEIGHT), 1, 5);
    add_child(components, window_id, container);

    for (int i = 0; i < length; i++) {
        int j = add_button_to_container(components, container, values[i], select);
        WidgetComponent_get(components, j)->value = i;
    }

    add_scrollbar_to_container(components, container);

    return window_id;
}


void select_wall(ComponentData* components, int entity) {
    WidgetComponent* widget = WidgetComponent_get(components, entity);
    selected_wall = widget->value;
    tool = TOOL_WALL;
}


void toggle_walls(ComponentData* components, int entity) {
    UNUSED(entity);
    static int window_id = -1;
    window_id = toggle_list_window(components, window_id, "WALLS", toggle_walls, wall_names, LENGTH(wall_names), 
        select_wall);
}


void toggle_objects(ComponentData* components, int entity) {
    UNUSED(entity);
    static int window_id = -1;
    window_id = toggle_list_window(components, window_id, "OBJECTS", toggle_objects, objects, LENGTH(objects), 
        select_object);
}


void select_prefab(ComponentData* components, int entity) {
    WidgetComponent* widget = WidgetComponent_get(components, entity);
    strcpy(prefab_name, widget->string);
    tool = TOOL_PREFAB;
}


void toggle_prefabs(ComponentData* components, int entity) {
    UNUSED(entity);
    static int window_id = -1;
    // TODO: list files in directory
    window_id = toggle_list_window(components, window_id, "PREFABS", toggle_prefabs, prefabs, LENGTH(prefabs), 
        select_prefab);
}


void save_prefab(ComponentData* components, Filename filename) {
    sfVector2f center = get_selections_center(components);
    center = snap_to_grid_center(center);
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


void rotate_selections(GameData* data) {
    sfVector2f center = get_selections_center(data->components);

    ListNode* node;
    FOREACH (node, selections) {
        int i = node->value;
        CoordinateComponent* coord = CoordinateComponent_get(data->components, node->value);
        if (coord->parent != -1) continue;

        sfVector2f r = diff(coord->position, center);
        r = perp(r);
        clear_grid(data->components, data->grid, i);
        coord->position = sum(center, r);
        coord->angle += 0.5f * M_PI;
        update_grid(data->components, data->grid, i);
    }
}


void destroy_selections(GameData* data) {
    ListNode* node;
    FOREACH (node, selections) {
        int i = node->value;
        clear_grid(data->components, data->grid, i);
        destroy_entity(data->components, i);
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
            float dy = mouse_world.y - selection_start.y;
            if (grid_sizes[grid_size_index]) {
                dx = grid_sizes[grid_size_index] * floorf(dx / grid_sizes[grid_size_index]);
                dy = grid_sizes[grid_size_index] * floorf(dy / grid_sizes[grid_size_index]);
            }
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
                    if (inside_collider(components, node->value, mouse_world)) {
                        grabbed = true;
                        break;
                    }
                }
            } else {
                // TODO: grab top entity
            }
            if (!grabbed) {
                selection_box = create_entity(components);
                CoordinateComponent_add(components, selection_box, selection_start, 0.0f);
                ColliderComponent_add_rectangle(components, selection_box, 0.0f, 0.0f, GROUP_ALL);
            }
        } else if (event.mouseButton.button == sfMouseRight) {
            rotate_selections(data);
        }
    } else if (event.type == sfEvtMouseButtonReleased && event.mouseButton.button == sfMouseLeft) {
        if (selection_box != -1) {
            destroy_entity(components, selection_box);
            selection_box = -1;
        }
        grabbed = false;
    } else if (event.type == sfEvtKeyPressed) {
        if (selections) {
            if (event.key.code == sfKeyDelete) {
                destroy_selections(data);
            } else if (event.key.code == sfKeyS) {
                save_prefab(data->components, "prefab.json");
            } else if (event.key.code == sfKeyLeft) {
                move_selections(data, vec(grid_sizes[grid_size_index], 0.0f));
            } else if (event.key.code == sfKeyRight) {
                move_selections(data, vec(-grid_sizes[grid_size_index], 0.0f));
            } else if (event.key.code == sfKeyDown) {
                move_selections(data, vec(0.0f, -grid_sizes[grid_size_index]));
            } else if (event.key.code == sfKeyUp) {
                move_selections(data, vec(0.0f, grid_sizes[grid_size_index]));
            }
        }
    }
}


void input_tool_wall(GameData* data, sfEvent event) {
    if (event.type == sfEvtMouseMoved) {
        selection_end = snap_to_grid(mouse_world);
    } else if (event.type == sfEvtMouseButtonPressed) {
        if (event.mouseButton.button == sfMouseLeft) {
            selection_start = snap_to_grid(mouse_world);
        }
    } else if (event.type == sfEvtMouseButtonReleased && event.mouseButton.button == sfMouseLeft) {
        float width = fabsf(selection_end.x - selection_start.x);
        float height = fabsf(selection_end.y - selection_start.y);
        sfVector2f pos = mult(0.5f, sum(selection_end, selection_start));
        if (width > 0.0f && height > 0.0f) {
            int i = create_wall(data->components, pos, 0.0f, width, height, wall_names[selected_wall]);
            update_grid(data->components, data->grid, i);
        }
    }
}


void input_tool_object(GameData* data, sfEvent event) {
    if (event.type == sfEvtMouseButtonPressed) {
        if (event.mouseButton.button == sfMouseLeft) {
            data->components->added_entities = List_create();
            create_object(data->components, selected_object, snap_to_grid_center(mouse_world), 0.0f);
            ListNode* node;
            FOREACH(node, data->components->added_entities) {
                if (ColliderComponent_get(data->components, node->value)) {
                   update_grid(data->components, data->grid, node->value);
                }
            }
            List_delete(data->components->added_entities);
            data->components->added_entities = NULL;
        }
    }
}


void input_tool_prefab(GameData* data, sfEvent event) {
    if (event.type == sfEvtMouseButtonPressed) {
        if (event.mouseButton.button == sfMouseLeft) {
            load_prefab(data, prefab_name, snap_to_grid_center(mouse_world), 0.0f);
        }
    }
}


void input_editor(GameData* data, sfRenderWindow* window, sfEvent event) {
    UNUSED(window);
    static sfVector2i mouse_screen = { 0, 0 };

    if (input_widgets(data->components, data->camera, event)) {
        return;
    }

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
        case TOOL_OBJECT:
            input_tool_object(data, event);
            break;
        case TOOL_PREFAB:
            input_tool_prefab(data, event);
            break;
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
    } else if (event.type == sfEvtKeyPressed) {
        if (event.key.code == sfKeyNum1) {
            tool = TOOL_SELECT;
        } else if (event.key.code == sfKeyNum2) {
            tool = TOOL_WALL;
        } else if (event.key.code == sfKeyNum3) {
            tool = TOOL_PREFAB;
        } else if (event.key.code == sfKeyO) {
            toggle_objects(data->components, -1);
        } else if (event.key.code == sfKeyP) {
            toggle_prefabs(data->components, -1);
        } else if (event.key.code == sfKeyW) {
            toggle_walls(data->components, -1);
        } else if (event.key.code == sfKeyDash) {
            grid_size_index = max(grid_size_index - 1, 0);
        } else if (event.key.code == sfKeyEqual) {
            grid_size_index = min(grid_size_index + 1, LENGTH(grid_sizes) - 1);
        }
    }
}


void draw_editor(GameData data, sfRenderWindow* window) {
    draw_game(data, window);
    draw_grid(data.components, window, data.camera, grid_sizes[grid_size_index], grid_sizes[grid_size_index]);

    sfVector2f mouse_pos = screen_to_world(data.components, data.camera, sfMouse_getPosition((sfWindow*) window));

    switch(tool) {
        case TOOL_SELECT:
            if (selection_box != -1) {
                sfVector2f pos = get_position(data.components, selection_box);
                ColliderComponent* collider = ColliderComponent_get(data.components, selection_box);
                draw_rectangle_outline(window, data.components, data.camera, NULL, pos, collider->width, collider->height, 0.0f, 
                    0.05f, sfWhite);
            }
            break;
        case TOOL_WALL:
            if (sfMouse_isButtonPressed(sfMouseLeft)) {
                sfVector2f end = snap_to_grid(mouse_pos);
                float width = fabsf(end.x - selection_start.x);
                float height = fabsf(end.y - selection_start.y);
                sfVector2f pos = mult(0.5f, sum(end, selection_start));
                draw_rectangle_outline(window, data.components, data.camera, NULL, pos, width, height, 0.0f, 0.05f, sfWhite);
            } else {
                sfVector2f pos = snap_to_grid(mouse_pos);
                draw_line(window, data.components, data.camera, NULL, vec(pos.x - 0.2f, pos.y), 
                    vec(pos.x + 0.2f, pos.y), 0.05f, sfWhite);
                draw_line(window, data.components, data.camera, NULL, vec(pos.x, pos.y - 0.2f), 
                    vec(pos.x, pos.y + 0.2f), 0.05f, sfWhite);
            }
            break;
        case TOOL_OBJECT:
            draw_rectangle_outline(window, data.components, data.camera, NULL, snap_to_grid_center(mouse_pos),
                1.0f, 1.0f, 0.0f, 0.05f, sfWhite);
            break;
        case TOOL_PREFAB:
            draw_rectangle_outline(window, data.components, data.camera, NULL, snap_to_grid_center(mouse_pos),
                1.0f, 1.0f, 0.0f, 0.05f, sfWhite);
            break;
    }

    bool waypoint_selected = false;
    if (selections) {
        ListNode* node;
        FOREACH (node, selections) {
            int i = node->value;
            sfVector2f pos = get_position(data.components, i);
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

    draw_widgets(data.components, window, data.camera);

    draw_circle(window, data.components, data.camera, NULL, mouse_pos, 0.1f, sfWhite);
}
