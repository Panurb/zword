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
#include "particle.h"
#include "light.h"
#include "physics.h"
#include "object.h"
#include "animation.h"
#include "tile.h"


typedef enum {
    TOOL_SELECT,
    TOOL_TILE,
    TOOL_OBJECT,
    TOOL_PREFAB
} Tool;


static int selection_box = -1;
static sfVector2f tile_start = { 0.0f, 0.0f };
static sfVector2f tile_end = { 0.0f, 0.0f };
static bool tile_started = false;
static List* selections = NULL;
static Tool tool = TOOL_TILE;
static sfVector2f mouse_world = { 0.0f, 0.0f };
static ButtonText prefabs[] = { "testi.json", "prefab.json" };
static Filename prefab_name = "";
static float grid_sizes[] = { 0.0f, 0.25f, 0.5f, 1.0f };
static int grid_size_index = 3;
static ButtonText map_name = "";

static int selected_tile = 0;
static ButtonText tile_names[] = {
    "altar_tile",
    "beach_tile",
    "beach_corner",
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
ButtonText object_names[] = {
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
    "lamp",
    "priest",
    "zombie"
};

void set_map_name(ButtonText name) {
    strcpy(map_name, name);
}


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


void select_tile(ComponentData* components, int entity) {
    WidgetComponent* widget = WidgetComponent_get(components, entity);
    selected_tile = widget->value;
    tool = TOOL_TILE;
}


void toggle_tiles(ComponentData* components, int entity) {
    UNUSED(entity);
    static int window_id = -1;
    window_id = toggle_list_window(components, window_id, "TILES", toggle_tiles, tile_names, LENGTH(tile_names), 
        select_tile);
}


void toggle_objects(ComponentData* components, int entity) {
    UNUSED(entity);
    static int window_id = -1;
    window_id = toggle_list_window(components, window_id, "OBJECTS", toggle_objects, object_names, LENGTH(object_names), 
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


void save_map(ComponentData* components, int entity) {
    UNUSED(entity);
    cJSON* json = cJSON_CreateObject();
    serialize_map(json, components, false);
    save_json(json, "maps", map_name);
}


void create_editor_menu(GameData* data) {
    sfVector2f size = camera_size(data->components, data->menu_camera);
    sfVector2f pos = vec(0.5f * (-size.x + BUTTON_WIDTH), 0.5f * (size.y - BUTTON_HEIGHT));
    destroy_widgets(data->components);
    create_button(data->components, "TILES", pos, toggle_tiles);
    pos = sum(pos, vec(BUTTON_WIDTH, 0.0f));
    create_button(data->components, "OBJECTS", pos, toggle_objects);
    pos = sum(pos, vec(BUTTON_WIDTH, 0.0f));
    create_button(data->components, "PREFABS", pos, toggle_prefabs);
    pos = sum(pos, vec(BUTTON_WIDTH, 0.0f));
    create_button(data->components, "SAVE", pos, save_map);
}


void update_editor(GameData data, sfRenderWindow* window, float time_step) {
    update(data.components, time_step, data.grid);
    collide(data.components, data.grid);
    update_waypoints(data.components, data.grid, data.camera);

    update_particles(data.components, data.camera, time_step);
    update_lights(data.components, time_step);
    update_camera(data.components, data.camera, time_step);

    draw_shadows(data.components, data.shadow_texture, data.camera);
    draw_lights(data.components, data.grid, data.light_texture, data.camera, data.ambient_light);

    animate(data.components, time_step);

    update_widgets(data.components, window, data.menu_camera);

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
            collider->width = fabsf(mouse_world.x - tile_start.x);
            collider->height = fabsf(mouse_world.y - tile_start.y);
            coord->position = mult(0.5f, sum(tile_start, mouse_world));
        }

        if (grabbed) {
            float dx = mouse_world.x - tile_start.x;
            float dy = mouse_world.y - tile_start.y;
            if (grid_sizes[grid_size_index]) {
                dx = grid_sizes[grid_size_index] * floorf(dx / grid_sizes[grid_size_index]);
                dy = grid_sizes[grid_size_index] * floorf(dy / grid_sizes[grid_size_index]);
            }
            sfVector2f delta_pos = vec(dx, dy);
            tile_start = sum(tile_start, vec(dx, dy));

            move_selections(data, delta_pos);
        }
    } else if (event.type == sfEvtMouseButtonPressed) {
        if (event.mouseButton.button == sfMouseLeft) {
            tile_start = mouse_world;

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
                CoordinateComponent_add(components, selection_box, tile_start, 0.0f);
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


void input_tool_tile(GameData* data, sfEvent event) {
    if (event.type == sfEvtMouseMoved) {
        tile_end = snap_to_grid(mouse_world);
    } else if (event.type == sfEvtMouseButtonPressed) {
        if (event.mouseButton.button == sfMouseLeft) {
            tile_start = snap_to_grid(mouse_world);
            tile_started = true;
        }
    } else if (event.type == sfEvtMouseButtonReleased && event.mouseButton.button == sfMouseLeft) {
        float width = fabsf(tile_end.x - tile_start.x);
        float height = fabsf(tile_end.y - tile_start.y);
        sfVector2f pos = mult(0.5f, sum(tile_end, tile_start));
        if (width > 0.0f && height > 0.0f) {
            data->components->added_entities = List_create();
            create_tile(data->components, selected_tile, pos, 0.0f, width, height);
            ListNode* node;
            FOREACH(node, data->components->added_entities) {
                if (ColliderComponent_get(data->components, node->value)) {
                   update_grid(data->components, data->grid, node->value);
                }
            }
            List_delete(data->components->added_entities);
            data->components->added_entities = NULL;
        }
        tile_started = false;
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

    if (input_widgets(data->components, data->menu_camera, event)) {
        return;
    }

    ComponentData* components = data->components;

    CoordinateComponent* cam_coord = CoordinateComponent_get(components, data->camera);
    CameraComponent* cam = CameraComponent_get(components, data->camera);

    switch (tool) {
        case TOOL_SELECT:
            input_tool_select(data, event);
            break;
        case TOOL_TILE:
            input_tool_tile(data, event);
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
    } else if (event.type == sfEvtMouseButtonPressed) {
        if (event.mouseButton.button == sfMouseRight) {
            tool = TOOL_SELECT;
        }
    } else if (event.type == sfEvtKeyPressed) {
        switch (event.key.code) {
            case sfKeyO:
                toggle_objects(data->components, -1);
                break;
            case sfKeyP:
                toggle_prefabs(data->components, -1);
                break;
            case sfKeyW:
                toggle_tiles(data->components, -1);
                break;
            case sfKeyDash:
                grid_size_index = max(grid_size_index - 1, 0);
                break;
            case sfKeyEqual:
                grid_size_index = min(grid_size_index + 1, LENGTH(grid_sizes) - 1);
                break;
            default:
                break;
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
                draw_rectangle_outline(window, data.components, data.camera, NULL, pos, collider->width, 
                    collider->height, 0.0f, 0.05f, sfWhite);
            }
            break;
        case TOOL_TILE:
            if (tile_started) {
                sfVector2f end = snap_to_grid(mouse_pos);
                float width = fabsf(end.x - tile_start.x);
                float height = fabsf(end.y - tile_start.y);
                sfVector2f pos = mult(0.5f, sum(end, tile_start));
                draw_rectangle_outline(window, data.components, data.camera, NULL, pos, width, height, 0.0f, 0.05f, 
                    sfWhite);
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

    draw_widgets(data.components, window, data.menu_camera);

    draw_circle(window, data.components, data.camera, NULL, mouse_pos, 0.1f, sfWhite);
}