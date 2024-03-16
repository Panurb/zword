#define _USE_MATH_DEFINES

#include <math.h>
#include <string.h>
#include <stdio.h>

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
#include "enemy.h"
#include "input.h"
#include "game.h"


typedef enum {
    TOOL_SELECT,
    TOOL_TILE,
    TOOL_OBJECT,
    TOOL_PREFAB
} Tool;


typedef enum {
    CATEGORY_TERRAIN,
    CATEGORY_FLOOR,
    CATEGORY_DECALS,
    CATEGORY_WALLS,
    CATEGORY_OBJECTS,
    CATEGORY_WAYPOINTS
} EntityCategory;


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
static float double_click_time = 0.0f;
static int entity_settings_id = -1;
static int entity_settings_entity = -1;


char* category_names[] = { "terrain", "floor", "decals", "walls", "objects", "waypoints" };
bool selected_categories[] = { true, true, true, true, true, true };

static int selected_tile = 0;
static ButtonText tile_names[] = {
    "altar",
    "beach",
    "beach corner",
    "board",
    "brick",
    "fence",
    "grass",
    "level_end",
    "roof",
    "spawner",
    "stone",
    "tiles",
    "water",
    "wood"
};

static ButtonText selected_object_name = "";
static float selected_object_width = 1.0f;
static float selected_object_height = 1.0f;
static ButtonText object_names[] = {
    "bandage",
    "bed",
    "bench",
    "blood",
    "candle",
    "car",
    "desk",
    "door",
    "fire",
    "flashlight",
    "gas",
    "hay bale",
    "hole",
    "lamp",
    "rock",
    "sink",
    "stove",
    "table",
    "toilet",
    "tree",
    "tutorial",
    "uranium",
    "waypoint",
};

static ButtonText creature_names[] = {
    "big boy",
    "boss",
    "farmer",
    "player",
    "priest",
    "zombie"
};

static ButtonText weapon_names[] = {
    "ammo pistol",
    "ammo rifle",
    "ammo shotgun",
    "assault rifle",
    "axe",
    "pistol",
    "sawed-off",
    "shotgun",
    "smg",
    "sword"
};


bool category_selected(ComponentData* components, int entity) {
    if (WaypointComponent_get(components, entity)) {
        return selected_categories[CATEGORY_WAYPOINTS];
    }

    ImageComponent* image = ImageComponent_get(components, entity);
    if (image) {
        switch (image->layer) {
        case LAYER_GROUND:
            return selected_categories[CATEGORY_TERRAIN];
        case LAYER_ROADS:
        case LAYER_FLOOR:
            return selected_categories[CATEGORY_FLOOR];
        case LAYER_DECALS:
            return selected_categories[CATEGORY_DECALS];
        case LAYER_WALLS:
            return selected_categories[CATEGORY_WALLS];
        case LAYER_CORPSES:
        case LAYER_ITEMS:
        case LAYER_VEHICLES:
        case LAYER_ENEMIES:
        case LAYER_WEAPONS:
        case LAYER_PLAYERS:
        case LAYER_TREES:
            return selected_categories[CATEGORY_OBJECTS];
        default:
            return false;
        }
    }

    return true;
}


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
    if (selections->size != 0) {
        center = mult(1.0f / selections->size, center);
    }

    return center;
}


void select_object(ComponentData* components, int entity) {
    WidgetComponent* widget = WidgetComponent_get(components, entity);
    strcpy(selected_object_name, widget->string);
    tool = TOOL_OBJECT;

    components->added_entities = List_create();

    printf("selected object: %s\n", selected_object_name);

    int i = create_object(components, selected_object_name, zeros(), 0.0f);
    ColliderComponent* collider = ColliderComponent_get(components, i);
    if (collider) {
        selected_object_width = collider->width;
        selected_object_height = collider->height;
    } else {
        selected_object_width = 1.0f;
        selected_object_height = 1.0f;
    }

    ListNode* node;
    FOREACH(node, components->added_entities) {
      destroy_entity(components, node->value);
    }
    List_delete(components->added_entities);
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


void toggle_creatures(ComponentData* components, int entity) {
    UNUSED(entity);
    static int window_id = -1;
    window_id = toggle_list_window(components, window_id, "CREATURES", toggle_creatures, creature_names, LENGTH(creature_names), 
        select_object);
}


void toggle_weapons(ComponentData* components, int entity) {
    UNUSED(entity);
    static int window_id = -1;
    window_id = toggle_list_window(components, window_id, "WEAPONS", toggle_weapons, weapon_names, LENGTH(weapon_names), 
        select_object);
}


void set_game_mode(ComponentData* components, int entity, int delta) {
    UNUSED(delta);
    WidgetComponent* widget = WidgetComponent_get(components, entity);
    game_data->game_mode = widget->value;
}


void set_ambient_light(ComponentData* components, int entity, int delta) {
    UNUSED(delta);
    WidgetComponent* widget = WidgetComponent_get(components, entity);
    game_data->ambient_light = widget->value / 100.0f;
}


void toggle_editor_settings(ComponentData* components, int entity) {
    UNUSED(entity);
    static int window_id = -1;

    if (window_id != -1) {
        destroy_entity_recursive(components, window_id);
        window_id = -1;
        return;
    }
   
    window_id = create_window(components, vec(0.0f, 0.0f), "SETTINGS", 1, toggle_editor_settings);
    int container = create_container(components, vec(0.0f, -3 * BUTTON_HEIGHT), 1, 5);
    add_child(components, window_id, container);

    int i = create_label(components, "Game mode", zeros());
    add_widget_to_container(components, container, i);

    i = create_dropdown(components, zeros(), GAME_MODES, 3);
    add_widget_to_container(components, container, i);
    WidgetComponent* widget = WidgetComponent_get(components, i);
    widget->on_change = set_game_mode;

    i = create_label(components, "Ambient light", zeros());
    add_widget_to_container(components, container, i);

    i = create_slider(components, zeros(), 0, 100, 50, set_ambient_light);
    add_widget_to_container(components, container, i);
}


void close_entity_settings(ComponentData* components, int entity) {
    UNUSED(entity);
    destroy_entity_recursive(components, entity_settings_id);
    entity_settings_id = -1;
    entity_settings_entity = -1;
}


void change_text(ComponentData* components, int textbox, int unicode) {
    UNUSED(unicode);
    TextComponent* text = TextComponent_get(components, entity_settings_entity);
    WidgetComponent* widget = WidgetComponent_get(components, textbox);
    strcpy(text->source_string, widget->string);
    replace_actions(text->string, text->source_string);
}


void open_entity_settings(ComponentData* components, int entity) {
    if (entity_settings_id != -1) {
        close_entity_settings(components, entity);
    }

    entity_settings_entity = entity;

    ButtonText buffer;
    snprintf(buffer, BUTTON_TEXT_SIZE, "ENTITY %d", entity);
    entity_settings_id = create_window(components, vec(0.0f, 0.0f), buffer, 2, close_entity_settings);
    int container = create_container(components, vec(0.0f, -3 * BUTTON_HEIGHT), 2, 5);
    add_child(components, entity_settings_id, container);

    TextComponent* text = TextComponent_get(components, entity);
    if (text) {
        int i = create_textbox(components, zeros(), 2);
        add_widget_to_container(components, container, i);
        WidgetComponent* widget = WidgetComponent_get(components, i);
        strcpy(widget->string, text->source_string);
        widget->max_value = 100;
        widget->on_change = change_text;
    }
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
    center = snap_to_grid_center(center, grid_sizes[grid_size_index], grid_sizes[grid_size_index]);
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
            CoordinateComponent* coord = CoordinateComponent_get(data->components, node->value);
            if (ColliderComponent_get(data->components, i)) {
                clear_grid(data->components, data->grid, i);
                coord->position = sum(coord->position, delta_pos);
                update_grid(data->components, data->grid, i);
            } else {
                coord->position = sum(coord->position, delta_pos);
            }
        }
        delta_pos = zeros();
    }
}


void rotate_selections(GameData* data) {
    // TODO: snap to grid after rotation
    sfVector2f center = get_selections_center(data->components);

    ListNode* node;
    FOREACH (node, selections) {
        int i = node->value;
        CoordinateComponent* coord = CoordinateComponent_get(data->components, node->value);
        if (coord->parent != -1) continue;

        sfVector2f r = diff(coord->position, center);
        r = perp(r);
        if (ColliderComponent_get(data->components, i)) {
            clear_grid(data->components, data->grid, i);
            coord->position = sum(center, r);
            coord->angle += 0.5f * M_PI;
            update_grid(data->components, data->grid, i);
        } else {
            coord->position = sum(center, r);
            coord->angle += 0.5f * M_PI;
        }
    }
}


void destroy_selections(GameData* data) {
    ListNode* node;
    FOREACH (node, selections) {
        int i = node->value;
        if (ColliderComponent_get(data->components, i)) {
            clear_grid(data->components, data->grid, i);
        }
        destroy_entity_recursive(data->components, i);
    }
    List_clear(selections);
}


void update_selections(GameData data) {
    if (!selections) {
        selections = List_create();
    }
    List_clear(selections);

    for (int i = 0; i < data.components->entities; i++) {
        CoordinateComponent* coord = CoordinateComponent_get(data.components, i);
        if (!coord) continue;
        if (i == selection_box) continue;
        if (WidgetComponent_get(data.components, i)) continue;
        if (CameraComponent_get(data.components, i)) continue;
        if (coord->parent != -1) continue;

        if (!category_selected(data.components, i)) {
            continue;
        }

        ColliderComponent* collider = ColliderComponent_get(data.components, i);
        ImageComponent* image = ImageComponent_get(data.components, i);
        sfVector2f overlap = zeros();
        if (collider) {
            overlap = overlap_collider_collider(data.components, selection_box, i);
        } else if (image) {
            overlap = overlap_rectangle_image(data.components, selection_box, i);
        } else {
            if (point_inside_collider(data.components, selection_box, get_position(data.components, i))) {
                overlap = ones();
            }
        }
        
        if (non_zero(overlap)) {
            List_add(selections, i);
        }
    }
}


void save_map(ComponentData* components, int entity) {
    UNUSED(entity);
    save_game(game_data, map_name);
}


void quit(ComponentData* components, int entity) {
    UNUSED(components);
    UNUSED(entity);
    game_state = STATE_END;
}


void create_editor_menu(GameData* data) {
    sfVector2f size = camera_size(data->components, data->menu_camera);
    sfVector2f pos = vec(0.5f * (-size.x + BUTTON_WIDTH), 0.5f * (size.y - BUTTON_HEIGHT));
    destroy_widgets(data->components);
    create_button(data->components, "TILES", pos, toggle_tiles);
    pos = sum(pos, vec(0.0f, -BUTTON_HEIGHT));
    create_button(data->components, "OBJECTS", pos, toggle_objects);
    pos = sum(pos, vec(0.0f, -BUTTON_HEIGHT));
    create_button(data->components, "CREATURES", pos, toggle_creatures);
    pos = sum(pos, vec(0.0f, -BUTTON_HEIGHT));
    create_button(data->components, "WEAPONS", pos, toggle_weapons);
    pos = sum(pos, vec(0.0f, -BUTTON_HEIGHT));
    // pos = sum(pos, vec(BUTTON_WIDTH, 0.0f));
    // create_button(data->components, "PREFABS", pos, toggle_prefabs);
    create_button(data->components, "SETTINGS", pos, toggle_editor_settings);
    pos = sum(pos, vec(0.0f, -BUTTON_HEIGHT));
    create_button(data->components, "SAVE", pos, save_map);
    pos = sum(pos, vec(0.0f, -BUTTON_HEIGHT));
    create_button(data->components, "QUIT", pos, quit);
}


void update_editor(GameData data, sfRenderWindow* window, float time_step) {
    // update(data.components, time_step, data.grid);
    // collide(data.components, data.grid);
    update_waypoints(data.components, data.grid, data.camera);

    update_particles(data.components, data.camera, time_step);
    update_lights(data.components, time_step);
    update_camera(data.components, data.camera, time_step, false);

    draw_shadows(data.components, data.shadow_texture, data.camera);
    draw_lights(data.components, data.grid, data.light_texture, data.camera, data.ambient_light);

    animate(data.components, time_step);

    update_widgets(data.components, window, data.menu_camera);

    if (selection_box != -1) {
        update_selections(data);
    }

    double_click_time = fmaxf(double_click_time - time_step, 0.0f);
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
                    if (ColliderComponent_get(components, node->value)) {
                        if (point_inside_collider(components, node->value, mouse_world)) {
                            grabbed = true;
                            break;
                        }
                    } else if (ImageComponent_get(components, node->value)) {
                        if (point_inside_image(components, node->value, mouse_world)) {
                            grabbed = true;
                            break;
                        }
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
            if (selections) {
                rotate_selections(data);
            }
        }
    } else if (event.type == sfEvtMouseButtonReleased && event.mouseButton.button == sfMouseLeft) {
        if (selection_box != -1) {
            destroy_entity(components, selection_box);
            selection_box = -1;
        }
        grabbed = false;
        if (double_click_time > 0.0f && selections->size == 1) {
            open_entity_settings(components, selections->head->value);
            double_click_time = 0.0f;
        } else {
            double_click_time = 0.2f;
        }
    } else if (event.type == sfEvtKeyPressed) {
        if (selections) {
            switch (event.key.code) {
            case sfKeyDelete:
                destroy_selections(data);
                break;
            case sfKeyS:
                save_prefab(data->components, "prefab.json");
                break;
            case sfKeyLeft:
                move_selections(data, vec(-grid_sizes[grid_size_index], 0.0f));
                break;
            case sfKeyRight:
                move_selections(data, vec(grid_sizes[grid_size_index], 0.0f));
                break;
            case sfKeyDown:
                move_selections(data, vec(0.0f, -grid_sizes[grid_size_index]));
                break;
            case sfKeyUp:
                move_selections(data, vec(0.0f, grid_sizes[grid_size_index]));
                break;
            default:
                break;
            }
        }
    }
}


void input_tool_tile(GameData* data, sfEvent event) {
    if (event.type == sfEvtMouseMoved) {
        tile_end = snap_to_grid(mouse_world, grid_sizes[grid_size_index], grid_sizes[grid_size_index]);
    } else if (event.type == sfEvtMouseButtonPressed) {
        if (event.mouseButton.button == sfMouseLeft) {
            tile_start = snap_to_grid(mouse_world, grid_sizes[grid_size_index], grid_sizes[grid_size_index]);
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
            sfVector2f pos = snap_to_grid(mouse_world, grid_sizes[grid_size_index], grid_sizes[grid_size_index]);
            create_object(data->components, selected_object_name, pos, 0.0f);
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
            sfVector2f pos = snap_to_grid_center(mouse_world, grid_sizes[grid_size_index], grid_sizes[grid_size_index]);
            load_prefab(data, prefab_name, pos, 0.0f);
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
        if (entity_settings_entity != -1) {
            switch (event.key.code) {
            case sfKeyEscape:
                close_entity_settings(components, entity_settings_id);
                break;
            default:
                return;
            }
        }

        switch (event.key.code) {
            case sfKeyNum1:
            case sfKeyNum2:
            case sfKeyNum3:
            case sfKeyNum4:
            case sfKeyNum5:
            case sfKeyNum6:
                selected_categories[event.key.code - 27] = !selected_categories[event.key.code - 27];
                break;
            case sfKeyO:
                toggle_objects(data->components, -1);
                break;
            case sfKeyP:
                toggle_prefabs(data->components, -1);
                break;
            case sfKeyT:
                toggle_tiles(data->components, -1);
                break;
            case sfKeyW:
                toggle_weapons(data->components, -1);
                break;
            case sfKeyDash:
                grid_size_index = maxi(grid_size_index - 1, 0);
                break;
            case sfKeyEqual:
                grid_size_index = mini(grid_size_index + 1, LENGTH(grid_sizes) - 1);
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
    sfVector2f mouse_grid = snap_to_grid(mouse_pos, grid_sizes[grid_size_index], grid_sizes[grid_size_index]);
    sfVector2f mouse_grid_center = snap_to_grid_center(mouse_pos, grid_sizes[grid_size_index], 
        grid_sizes[grid_size_index]);

    switch(tool) {
        case TOOL_SELECT:
            if (selection_box != -1) {
                sfVector2f pos = get_position(data.components, selection_box);
                ColliderComponent* collider = ColliderComponent_get(data.components, selection_box);
                draw_rectangle_outline(window, data.components, data.camera, NULL, pos, collider->width, 
                    collider->height, 0.0f, 0.05f, sfWhite);
            }
            for (int i = 0; i < LENGTH(category_names); i++) {
                sfColor color = selected_categories[i] ? sfWhite : get_color(0.6f, 0.6f, 0.6f, 1.0f);
                char buffer[128];
                snprintf(buffer, 128, "%d %s", i + 1, category_names[i]);
                draw_text(window, data.components, data.menu_camera, NULL, vec(i * 5 - 15, 14), buffer, 20, color);
            }
            break;
        case TOOL_TILE:
            if (tile_started) {
                sfVector2f end = mouse_grid;
                float width = fabsf(end.x - tile_start.x);
                float height = fabsf(end.y - tile_start.y);
                sfVector2f pos = mult(0.5f, sum(end, tile_start));
                draw_rectangle_outline(window, data.components, data.camera, NULL, pos, width, height, 0.0f, 0.05f, 
                    sfWhite);
            } else {
                sfVector2f pos = mouse_grid;
                draw_line(window, data.components, data.camera, NULL, vec(pos.x - 0.2f, pos.y), 
                    vec(pos.x + 0.2f, pos.y), 0.05f, sfWhite);
                draw_line(window, data.components, data.camera, NULL, vec(pos.x, pos.y - 0.2f), 
                    vec(pos.x, pos.y + 0.2f), 0.05f, sfWhite);
            }
            break;
        case TOOL_OBJECT: {
            draw_rectangle_outline(window, data.components, data.camera, NULL, mouse_grid,
                selected_object_width, selected_object_height, 0.0f, 0.05f, sfWhite);
            break;
        } case TOOL_PREFAB:
            draw_rectangle_outline(window, data.components, data.camera, NULL, mouse_grid_center,
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
            ColliderComponent* collider = ColliderComponent_get(data.components, i);
            if (image) {
                if (image->layer > LAYER_WALLS) {
                    draw_sprite(window, data.components, data.camera, image->sprite, pos, angle, image->scale, 
                        SHADER_OUTLINE);
                } else {
                    draw_rectangle_outline(window, data.components, data.camera, NULL, pos, image->width, 
                        image->height, angle, 0.05f, sfWhite);
                }
            } else if (collider) {
                draw_rectangle_outline(window, data.components, data.camera, NULL, pos, collider->width, 
                    collider->height, angle, 0.05f, sfWhite);
            } else {
                draw_circle(window, data.components, data.camera, NULL, pos, 0.1f, sfWhite);
            }

            if (WaypointComponent_get(data.components, i)) {
                waypoint_selected = true;
            }
        }
    }

    draw_waypoints(data.components, window, data.camera, waypoint_selected);

    draw_spawners(window, data);

    draw_tutorials(window, data);

    draw_widgets(data.components, window, data.menu_camera);

    draw_circle(window, data.components, data.camera, NULL, mouse_pos, 0.1f, sfWhite);
}
