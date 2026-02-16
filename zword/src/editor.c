#define _USE_MATH_DEFINES

#include <math.h>
#include <string.h>
#include <stdio.h>

#include "app.h"
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
#include "door.h"
#include "path.h"
#include "item.h"


typedef enum {
    TOOL_SELECT,
    TOOL_TILE,
    TOOL_OBJECT, 
    TOOL_PATH,
    TOOL_FOREST,
    TOOL_TEST
} Tool;


typedef enum {
    CATEGORY_TERRAIN,
    CATEGORY_FLOOR,
    CATEGORY_DECALS,
    CATEGORY_WALLS,
    CATEGORY_OBJECTS,
    CATEGORY_WAYPOINTS
} EntityCategory;


static bool shift_down = false;
static bool ctrl_down = false;

static int selection_box = -1;
static Vector2f tile_start = { 0.0f, 0.0f };
static Vector2f tile_end = { 0.0f, 0.0f };
static bool tile_started = false;
static List* selections = NULL;
static Tool tool = TOOL_SELECT;
static Vector2f mouse_world = { 0.0f, 0.0f };
static float grid_sizes[] = { 0.0f, 0.25f, 0.5f, 1.0f };
static int grid_size_index = 3;
static float double_click_time = 0.0f;
static int entity_settings_id = -1;
static int entity_settings_entity = -1;

static int tiles_window_id = -1;
static int objects_window_id = -1;
static int creatures_window_id = -1;
static int weapons_window_id = -1;
static Entity paths_window_id = NULL_ENTITY;
static int editor_settings_window_id = -1;

char* category_names[] = { "terrain", "floor", "decals", "walls", "objects", "waypoints" };
bool selected_categories[] = { true, true, true, true, true, true };

static Filename selected_object_name;
static float selected_object_width = 1.0f;
static float selected_object_height = 1.0f;
static float selected_object_angle = 0.0f;
static float selected_object_scale = 1.0f;


void reset_editor_ids() {
    tiles_window_id = -1;
    objects_window_id = -1;
    creatures_window_id = -1;
    weapons_window_id = -1;
    editor_settings_window_id = -1;
    entity_settings_id = -1;
    entity_settings_entity = -1;
    paths_window_id = NULL_ENTITY;
}


bool category_selected(int entity) {
    if (WaypointComponent_get(entity)) {
        return selected_categories[CATEGORY_WAYPOINTS];
    }

    ImageComponent* image = ImageComponent_get(entity);
    if (image) {
        switch (image->layer) {
        case LAYER_GROUND:
            return selected_categories[CATEGORY_TERRAIN];
        case LAYER_PATHS:
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


void select_object(int entity) {
    WidgetComponent* widget = WidgetComponent_get(entity);
    int window = get_parent(get_parent(entity));
    WidgetComponent* window_widget = WidgetComponent_get(window);
    sprintf(selected_object_name, "%s/%s", window_widget->string, widget->string);

    tool = TOOL_OBJECT;

    game_data->components->added_entities = List_create();

    int i = load_prefab(selected_object_name, zeros(), 0.0f, ones());
    ColliderComponent* collider = ColliderComponent_get(i);
    ImageComponent* image = ImageComponent_get(i);
    if (collider) {
        selected_object_width = collider->width;
        selected_object_height = collider->height;
    } else if (image) {
        selected_object_width = image->width;
        selected_object_height = image->height;
    } else {
        selected_object_width = 1.0f;
        selected_object_height = 1.0f;
    }

    ListNode* node;
    FOREACH(node, game_data->components->added_entities) {
      destroy_entity(node->value);
    }
    List_delete(game_data->components->added_entities);
    game_data->components->added_entities = NULL;

}


void select_tile(int entity) {
    WidgetComponent* widget = WidgetComponent_get(entity);
    int window = get_parent(get_parent(entity));
    WidgetComponent* window_widget = WidgetComponent_get(window);
    sprintf(selected_object_name, "%s/%s", window_widget->string, widget->string);

    tool = TOOL_TILE;
}


int toggle_prefabs(int window_id, Filename category, OnClick close, OnClick on_click) {
    if (window_id != -1) {
        destroy_entity_recursive(window_id);
        return -1;
    }

    Vector2f pos = sum(vec(0.0f, 2 * BUTTON_HEIGHT), mult(BUTTON_HEIGHT, rand_vector()));
    window_id = create_window(pos, category, 1, close);

    int container = create_container(vec(0.0f, -3.0f * BUTTON_HEIGHT), 1, 5);
    add_child(window_id, container);
    String path;
    snprintf(path, STRING_SIZE, "prefabs/%s", category);
    add_files_to_container(container, path, on_click);
    add_scrollbar_to_container(container);

    return window_id;
}


void toggle_tiles(int entity) {
    UNUSED(entity);
    tiles_window_id = toggle_prefabs(tiles_window_id, "tiles", toggle_tiles, select_tile);
}


void toggle_objects(int entity) {
    UNUSED(entity);
    objects_window_id = toggle_prefabs(objects_window_id, "objects", toggle_objects, select_object);
}


void toggle_creatures(int entity) {
    UNUSED(entity);
    creatures_window_id = toggle_prefabs(creatures_window_id, "creatures", toggle_creatures, select_object);
}


void toggle_weapons(int entity) {
    UNUSED(entity);
    weapons_window_id = toggle_prefabs(weapons_window_id, "weapons", toggle_weapons, select_object);
}


void select_road(int entity) {
    strcpy(selected_object_name, WidgetComponent_get(entity)->string);
    tool = TOOL_PATH;
}


void toggle_paths(Entity entity) {
    UNUSED(entity);
    if (paths_window_id != NULL_ENTITY) {
        destroy_entity_recursive(paths_window_id);
        paths_window_id = NULL_ENTITY;
        return;
    }

    Vector2f pos = sum(vec(0.0f, 2 * BUTTON_HEIGHT), mult(BUTTON_HEIGHT, rand_vector()));
    paths_window_id = create_window(pos, "Paths", 1, toggle_paths);

    int container = create_container(vec(0.0f, -3.0f * BUTTON_HEIGHT), 1, 5);
    add_child(paths_window_id, container);

    add_button_to_container(container, "road", select_road);
    add_button_to_container(container, "river", select_road);  
    add_button_to_container(container, "footpath", select_road);  

    add_scrollbar_to_container(container);
}


void set_game_mode(int entity, int delta) {
    UNUSED(delta);
    WidgetComponent* widget = WidgetComponent_get(entity);
    game_data->game_mode = widget->value;
}


void set_ambient_light(int entity, int delta) {
    UNUSED(delta);
    WidgetComponent* widget = WidgetComponent_get(entity);
    game_data->ambient_light = widget->value / 100.0f;
}


void set_weather(int entity, int delta) {
    UNUSED(delta);
    WidgetComponent* widget = WidgetComponent_get(entity);
    game_data->weather = widget->value;
}


void toggle_editor_settings(int entity) {
    UNUSED(entity);
    static int window_id = -1;

    if (window_id != -1) {
        destroy_entity_recursive(window_id);
        window_id = -1;
        return;
    }
   
    window_id = create_window(vec(0.0f, 0.0f), "SETTINGS", 1, toggle_editor_settings);
    int container = create_container(vec(0.0f, -4 * BUTTON_HEIGHT), 1, 7);
    add_child(window_id, container);

    int i = create_label("Game mode", zeros());
    add_widget_to_container(container, i);

    i = create_dropdown(zeros(), GAME_MODES, 3);
    add_widget_to_container(container, i);
    WidgetComponent* widget = WidgetComponent_get(i);
    widget->on_change = set_game_mode;

    i = create_label("Ambient light", zeros());
    add_widget_to_container(container, i);

    i = create_slider(zeros(), 0, 100, 50, set_ambient_light);
    add_widget_to_container(container, i);

    i = create_label("Weather", zeros());
    add_widget_to_container(container, i);

    i = create_dropdown(zeros(), WEATHERS, 3);
    add_widget_to_container(container, i);
    widget = WidgetComponent_get(i);
    widget->on_change = set_weather;
}


void close_entity_settings(int entity) {
    UNUSED(entity);
    destroy_entity_recursive(entity_settings_id);
    entity_settings_id = -1;
    entity_settings_entity = -1;
}


void change_text(int textbox, int unicode) {
    UNUSED(unicode);
    TextComponent* text = TextComponent_get(entity_settings_entity);
    WidgetComponent* widget = WidgetComponent_get(textbox);
    strcpy(text->source_string, widget->string);
    replace_actions(text->string, text->source_string);
}


void change_light_r(int slider, int value) {
    UNUSED(slider);
    LightComponent_get(entity_settings_entity)->color.r = value;
}


void change_light_g(int slider, int value) {
    UNUSED(slider);
    LightComponent_get(entity_settings_entity)->color.g = value;
}


void change_light_b(int slider, int value) {
    UNUSED(slider);
    LightComponent_get(entity_settings_entity)->color.b = value;
}


void ungroup(int entity) {
    UNUSED(entity);
    int root = get_root(entity_settings_entity);
    CoordinateComponent* coord = CoordinateComponent_get(entity_settings_entity);
    coord->prefab[0] = '\0';
}


void open_entity_settings(int entity) {
    if (entity_settings_id != -1) {
        close_entity_settings(entity);
    }

    entity_settings_entity = entity;

    ButtonText buffer;
    snprintf(buffer, BUTTON_TEXT_SIZE, "ENTITY %d", entity);
    entity_settings_id = create_window(vec(0.0f, 0.0f), buffer, 2, close_entity_settings);
    int container = create_container(vec(0.0f, -3 * BUTTON_HEIGHT), 2, 5);
    add_child(entity_settings_id, container);

    Filename prefab;
    prefab[0] = '\0';
    int root = get_root(entity);
    strcpy(prefab, CoordinateComponent_get(root)->prefab);
    int left = create_button("Ungroup", zeros(), ungroup);
    int right = create_label(prefab, zeros());
    add_row_to_container(container, left, right);

    LightComponent* light = LightComponent_get(entity);
    if (light) {
        int left = create_label("R", zeros());
        int right = create_slider(zeros(), 0, 255, light->color.r, change_light_r);
        add_row_to_container(container, left, right);

        left = create_label("G", zeros());
        right = create_slider(zeros(), 0, 255, light->color.g, change_light_g);
        add_row_to_container(container, left, right);


        left = create_label("B", zeros());
        right = create_slider(zeros(), 0, 255, light->color.b, change_light_b);
        add_row_to_container(container, left, right);
    }

    TextComponent* text = TextComponent_get(entity);
    if (text) {
        int i = create_textbox(zeros(), 2);
        add_widget_to_container(container, i);
        WidgetComponent* widget = WidgetComponent_get(i);
        strcpy(widget->string, text->source_string);
        widget->max_value = 100;
        widget->on_change = change_text;
    }
}


void move_selections(Vector2f delta_pos) {
    if (non_zero(delta_pos)) {
        ListNode* node;
        FOREACH (node, selections) {
            int i = node->value;
            CoordinateComponent* coord = CoordinateComponent_get(node->value);
            if (ColliderComponent_get(i)) {
                clear_grid(i);
                coord->position = sum(coord->position, delta_pos);
                update_grid(i);
            } else {
                coord->position = sum(coord->position, delta_pos);
            }
        }
        delta_pos = zeros();
    }
}


void rotate_selections(float angle) {
    // TODO: snap to grid after rotation
    Vector2f center = get_entities_center(selections);

    ListNode* node;
    FOREACH (node, selections) {
        int i = node->value;
        CoordinateComponent* coord = CoordinateComponent_get(node->value);
        if (coord->parent != -1) continue;

        Vector2f r = diff(coord->position, center);
        r = rotate(r, angle);
        if (ColliderComponent_get(i)) {
            clear_grid(i);
            coord->position = sum(center, r);
            coord->angle += angle;
            update_grid(i);
        } else {
            coord->position = sum(center, r);
            coord->angle += angle;
        }
    }
}


void scale_selections(float scale) {
    Vector2f center = get_entities_center(selections);

    ListNode* node;
    FOREACH (node, selections) {
        int i = node->value;
        CoordinateComponent* coord = CoordinateComponent_get(node->value);
        if (coord->parent != -1) continue;

        Vector2f r = diff(coord->position, center);
        r = mult(scale, r);
        if (ColliderComponent_get(i)) {
            clear_grid(i);
            coord->position = sum(center, r);
            coord->scale = mult(scale, coord->scale);
            update_grid(i);
        } else {
            coord->position = sum(center, r);
            coord->scale = mult(scale, coord->scale);
        }
    }
}


void destroy_selections() {
    ListNode* node;
    FOREACH (node, selections) {
        int i = node->value;
        if (get_parent(i) != -1) continue;

        if (ColliderComponent_get(i)) {
            clear_grid(i);
        }
        destroy_entity_recursive(i);
    }
    List_clear(selections);
}


void update_selections() {
    if (!selections) {
        selections = List_create();
    }
    List_clear(selections);

    float area = collider_width(selection_box) * collider_height(selection_box);

    int top_entity = -1;
    int top_layer = -1;
    for (int i = 0; i < game_data->components->entities; i++) {
        CoordinateComponent* coord = CoordinateComponent_get(i);
        if (!coord) continue;
        if (i == selection_box) continue;
        if (WidgetComponent_get(i)) continue;
        if (CameraComponent_get(i)) continue;
        if (coord->parent != -1) continue;

        if (!category_selected(i)) {
            continue;
        }

        ColliderComponent* collider = ColliderComponent_get(i);
        ImageComponent* image = ImageComponent_get(i);
        Vector2f overlap = zeros();
        if (collider) {
            overlap = overlap_collider_collider(selection_box, i);
        } else if (image) {
            overlap = overlap_rectangle_image(selection_box, i);
        } else {
            if (point_inside_collider(selection_box, get_position(i))) {
                overlap = ones();
            }
        }

        if (non_zero(overlap)) {
            if (image && image->layer > top_layer) {
                top_entity = i;
                top_layer = image->layer;
            }
        
            if (area > 0.0f) {
                List_add(selections, i);
                // TODO: select joint chain
            }
        }
    }

    if (area == 0.0f && top_entity != -1) {
        List_add(selections, top_entity);
    }
}


void set_tool_forest(int entity) {
    UNUSED(entity);
    tool = TOOL_FOREST;
}


void set_tool_test(int entity) {
    UNUSED(entity);
    tool = TOOL_TEST;
}


void save_current_map(int entity) {
    UNUSED(entity);
    save_map(game_data->map_name);
}


void quit_editor(int entity) {
    UNUSED(entity);
    reset_editor_ids();
    game_state = STATE_END;
}


void create_editor_menu() {
    Vector2f size = camera_size(game_data->menu_camera);
    Vector2f pos = vec(0.5f * (-size.x + BUTTON_WIDTH), 0.5f * (size.y - BUTTON_HEIGHT));
    destroy_widgets();
    create_button("TILES", pos, toggle_tiles);
    pos = sum(pos, vec(0.0f, -BUTTON_HEIGHT));
    create_button("OBJECTS", pos, toggle_objects);
    pos = sum(pos, vec(0.0f, -BUTTON_HEIGHT));
    create_button("CREATURES", pos, toggle_creatures);
    pos = sum(pos, vec(0.0f, -BUTTON_HEIGHT));
    create_button("WEAPONS", pos, toggle_weapons);
    pos = sum(pos, vec(0.0f, -BUTTON_HEIGHT));
    create_button("PATHS", pos, toggle_paths);
    pos = sum(pos, vec(0.0f, -BUTTON_HEIGHT));
    create_button("FOREST", pos, set_tool_forest);
    pos = sum(pos, vec(0.0f, -BUTTON_HEIGHT));
    create_button("SETTINGS", pos, toggle_editor_settings);
    pos = sum(pos, vec(0.0f, -BUTTON_HEIGHT));
    create_button("SAVE", pos, save_current_map);
    pos = sum(pos, vec(0.0f, -BUTTON_HEIGHT));
    create_button("TEST", pos, set_tool_test);
    pos = sum(pos, vec(0.0f, -BUTTON_HEIGHT));
    create_button("QUIT", pos, quit_editor);
}


void update_editor(float time_step) {
    update_coordinates();

    // update(data.time_step, data.grid);
    // collide(data.data.grid);
    update_waypoints();

    update_particles(game_data->camera, time_step);
    update_lights(time_step);
    update_camera(game_data->camera, time_step, false);

    draw_shadows(game_data->camera);
    draw_lights(game_data->camera, game_data->ambient_light);

    animate(time_step);

    update_widgets(game_data->menu_camera);

    if (selection_box != -1) {
        update_selections();
    }

    double_click_time = fmaxf(double_click_time - time_step, 0.0f);
}


void input_tool_select(SDL_Event event) {
    static bool grabbed = false;

    if (event.type == SDL_MOUSEMOTION) {
        if (selection_box != -1) {
            CoordinateComponent* coord = CoordinateComponent_get(selection_box);
            ColliderComponent* collider = ColliderComponent_get(selection_box);
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
            Vector2f delta_pos = vec(dx, dy);
            tile_start = sum(tile_start, vec(dx, dy));

            move_selections(delta_pos);
        }
    } else if (event.type == SDL_MOUSEBUTTONDOWN) {
        if (event.button.button == SDL_BUTTON_LEFT) {
            tile_start = mouse_world;

            if (selections) {
                ListNode* node;
                FOREACH (node, selections) {
                    if (ColliderComponent_get(node->value)) {
                        if (point_inside_collider(node->value, mouse_world)) {
                            grabbed = true;
                            break;
                        }
                    } else if (ImageComponent_get(node->value)) {
                        if (point_inside_image(node->value, mouse_world)) {
                            grabbed = true;
                            break;
                        }
                    }
                }
            }
            if (!grabbed) {
                selection_box = create_entity();
                CoordinateComponent_add(selection_box, tile_start, 0.0f);
                ColliderComponent_add_rectangle(selection_box, 0.0f, 0.0f, GROUP_ALL);
            }
        } else if (event.button.button == SDL_BUTTON_RIGHT) {
            if (selections) {
                List_clear(selections);
            }
        }
    } else if (event.type == SDL_MOUSEBUTTONUP && event.button.button == SDL_BUTTON_LEFT) {
        if (selection_box != -1) {
            destroy_entity(selection_box);
            selection_box = -1;
        }
        grabbed = false;
        if (double_click_time > 0.0f && selections->size == 1) {
            open_entity_settings(selections->head->value);
            double_click_time = 0.0f;
        } else {
            double_click_time = 0.2f;
        }
    } else if (event.type == SDL_KEYDOWN) {
        if (selections) {
            switch (event.key.keysym.sym) {
            case SDLK_DELETE:
                destroy_selections();
                break;
            case SDLK_s:
                // save_prefab("prefab.json");
                break;
            case SDLK_LEFT:
                move_selections(vec(-grid_sizes[grid_size_index], 0.0f));
                break;
            case SDLK_RIGHT:
                move_selections(vec(grid_sizes[grid_size_index], 0.0f));
                break;
            case SDLK_DOWN:
                move_selections(vec(0.0f, -grid_sizes[grid_size_index]));
                break;
            case SDLK_UP:
                move_selections(vec(0.0f, grid_sizes[grid_size_index]));
                break;
            default:
                break;
            }
        }
    } else if (event.type == SDL_MOUSEWHEEL) {
        if (shift_down) {
            rotate_selections(M_PI_4 * event.wheel.y);
        } else if (ctrl_down) {
            scale_selections(powf(1.1f, event.wheel.y));
        }
    }
}


void input_tool_tile(SDL_Event event) {
    if (event.type == SDL_MOUSEMOTION) {
        tile_end = snap_to_grid(mouse_world, grid_sizes[grid_size_index], grid_sizes[grid_size_index]);
    } else if (event.type == SDL_MOUSEBUTTONDOWN) {
        if (event.button.button == SDL_BUTTON_LEFT) {
            tile_start = snap_to_grid(mouse_world, grid_sizes[grid_size_index], grid_sizes[grid_size_index]);
            tile_started = true;
        }
    } else if (event.type == SDL_MOUSEBUTTONUP && event.button.button == SDL_BUTTON_LEFT) {
        float width = fabsf(tile_end.x - tile_start.x);
        float height = fabsf(tile_end.y - tile_start.y);
        Vector2f pos = mult(0.5f, sum(tile_end, tile_start));

        game_data->components->added_entities = List_create();
        int entity = load_prefab(selected_object_name, pos, 0.0f, ones());

        if (width > 0.0f && height > 0.0f) {
            CoordinateComponent* coord = CoordinateComponent_get(entity);
            ColliderComponent* collider = ColliderComponent_get(entity);
            ImageComponent* image = ImageComponent_get(entity);
            
            if (height > width && strcmp(selected_object_name, "water_tile") != 0) {
                coord->angle = M_PI_2;
                float temp = width;
                width = height;
                height = temp;
            }

            if (image) {
                coord->scale.x = width / image->width;
                coord->scale.y = height / image->height;
            } else if (collider) {
                coord->scale.x = width / collider->width;
                coord->scale.y = height / collider->height;
            }
        }

        ListNode* node;
        FOREACH(node, game_data->components->added_entities) {
            if (ColliderComponent_get(node->value)) {
                update_grid(node->value);
            }
        }
        List_delete(game_data->components->added_entities);
        game_data->components->added_entities = NULL;
        tile_started = false;
    }
}


Entity load_object_from_prefab(String name, Vector2f pos, Vector2f scale) {
    LOG_INFO("Loading object %s", name);
    game_data->components->added_entities = List_create();

    load_prefab(name, pos, selected_object_angle, scale);
    
    Entity root = NULL_ENTITY;
    ListNode* node;
    FOREACH(node, game_data->components->added_entities) {
        root = node->value;
        if (ColliderComponent_get(node->value)) {
            update_grid(node->value);
        }
    }
    List_delete(game_data->components->added_entities);
    game_data->components->added_entities = NULL;

    return get_root(root);
}


void input_tool_object(SDL_Event event) {
    if (event.type == SDL_MOUSEBUTTONDOWN) {
        if (event.button.button == SDL_BUTTON_LEFT) {
            Vector2f pos = snap_to_grid(mouse_world, grid_sizes[grid_size_index], grid_sizes[grid_size_index]);
            Vector2f scale = vec(selected_object_scale, selected_object_scale);

            load_object_from_prefab(selected_object_name, pos, scale);
        }
    } else if (event.type == SDL_MOUSEWHEEL) {
        if (shift_down) {
            selected_object_angle += M_PI_4 * event.wheel.y;
        } else if (ctrl_down) {
            selected_object_scale = clamp(selected_object_scale * powf(1.1f, event.wheel.y), 0.1f, 10.0f);
        }
    }
}


void input_tool_path(SDL_Event event) {
    if (event.type == SDL_MOUSEMOTION) {
        tile_end = snap_to_grid(mouse_world, grid_sizes[grid_size_index], grid_sizes[grid_size_index]);
    } else if (event.type == SDL_MOUSEBUTTONDOWN) {
        if (event.button.button == SDL_BUTTON_LEFT) {
            tile_start = snap_to_grid(mouse_world, grid_sizes[grid_size_index], grid_sizes[grid_size_index]);
            tile_started = true;
        }
    } else if (event.type == SDL_MOUSEBUTTONUP && event.button.button == SDL_BUTTON_LEFT) {
        game_data->components->added_entities = List_create();

        // TODO: better system for this
        if (strcmp(selected_object_name, "road") == 0) {
            create_road(tile_start, tile_end);
        } else if (strcmp(selected_object_name, "river") == 0) {
            create_river(tile_start, tile_end);
        } else if (strcmp(selected_object_name, "footpath") == 0) {
            create_footpath(tile_start, tile_end);
        }

        ListNode* node;
        FOREACH(node, game_data->components->added_entities) {
            if (ColliderComponent_get(node->value)) {
                update_grid(node->value);
            }
        }

        List_delete(game_data->components->added_entities);
        game_data->components->added_entities = NULL;
        tile_started = false;
    }
}


void input_tool_forest(SDL_Event event) {
    static float probs[4] = {0.35f, 0.3f, 0.3f, 0.05f};
    static String trees[4] = {"tree", "birch", "spruce", "rock"};

    if (event.type == SDL_MOUSEMOTION) {
        tile_end = snap_to_grid(mouse_world, grid_sizes[grid_size_index], grid_sizes[grid_size_index]);
    } else if (event.type == SDL_MOUSEBUTTONDOWN) {
        if (event.button.button == SDL_BUTTON_LEFT) {
            tile_start = snap_to_grid(mouse_world, grid_sizes[grid_size_index], grid_sizes[grid_size_index]);
            tile_started = true;
        }
    } else if (event.type == SDL_MOUSEBUTTONUP && event.button.button == SDL_BUTTON_LEFT) {
        float width = fabsf(tile_end.x - tile_start.x);
        float height = fabsf(tile_end.y - tile_start.y);
        Vector2f pos = mult(0.5f, sum(tile_end, tile_start));

        game_data->components->added_entities = List_create();

        float density = 0.4f;
        int nx = width * density;
        int ny = height * density;
        
        for (int i = -nx / 2; i < nx / 2 + 1; i++) {
            for (int j = -ny / 2; j < ny / 2 + 1; j++) {
                String prefab;
                int k = rand_choice(probs, LENGTH(probs));
                snprintf(prefab, STRING_SIZE, "objects/%s", trees[k]);

                Vector2f p = sum(pos, vec(i / density, j/ density));
                p = sum(p, mult(0.5f, rand_vector()));
                float scale = randf(0.8f, 1.2f);
                int e = load_prefab(prefab, p, rand_angle(), vec(scale, scale));
                List_add(game_data->components->added_entities, e);
            }
        }

        ListNode* node;
        FOREACH(node, game_data->components->added_entities) {
            if (ColliderComponent_get(node->value)) {
                update_grid(node->value);
            }
        }
        List_delete(game_data->components->added_entities);
        game_data->components->added_entities = NULL;
        tile_started = false;
    }
}


void input_tool_test(SDL_Event event) {
    if (event.type == SDL_MOUSEBUTTONDOWN) {
        if (event.button.button == SDL_BUTTON_LEFT) {
            // TODO: save temporary version of map
            save_current_map(NULL_ENTITY);

            Vector2f pos = snap_to_grid(mouse_world, grid_sizes[grid_size_index], grid_sizes[grid_size_index]);

            Entity i = NULL_ENTITY;
            ListNode* node = game_data->components->player.order->head;
            if (node) {
                i = node->value;
            }

            if (i == NULL_ENTITY) {
                load_object_from_prefab("creatures/player", pos, ones());
            } else {
                CoordinateComponent_get(i)->position = pos;
            }

            // Entity pistol = load_object_from_prefab("weapons/pistol", pos, ones());
            // add_item_to_inventory(i, pistol);

            reset_editor_ids();
            init_game();

            CameraComponent_get(game_data->camera)->zoom_target = 40.0f;

            game_data->testing = true;
            game_data->start_position = pos;
            game_state = STATE_GAME;
        }
    }
}


void input_editor(SDL_Event event) {
    if (input_widgets(game_data->menu_camera, event)) {
        return;
    }

    CoordinateComponent* cam_coord = CoordinateComponent_get(game_data->camera);
    CameraComponent* cam = CameraComponent_get(game_data->camera);

    switch (tool) {
        case TOOL_SELECT:
            input_tool_select(event);
            break;
        case TOOL_TILE:
            input_tool_tile(event);
            break;
        case TOOL_OBJECT:
            input_tool_object(event);
            break;
        case TOOL_PATH:
            input_tool_path(event);
            break;
        case TOOL_FOREST:
            input_tool_forest(event);
            break;
        case TOOL_TEST:
            input_tool_test(event);
            break;
    }

    if (event.type == SDL_MOUSEMOTION) {
        Vector2f mouse_delta = { event.motion.xrel, -event.motion.yrel };
        mouse_world = get_mouse_position(game_data->camera);

        if (event.button.button == SDL_BUTTON_MIDDLE) {
            cam_coord->position = sum(cam_coord->position, mult(-1.0f / cam->zoom, mouse_delta));
        }
    } else if (event.type == SDL_MOUSEWHEEL) {
        if (!shift_down && !ctrl_down) {
            cam->zoom_target = clamp(cam->zoom_target * powf(1.2f, event.wheel.y), 10.0f, 100.0f);
        }
    } else if (event.type == SDL_MOUSEBUTTONDOWN) {
        if (event.button.button == SDL_BUTTON_RIGHT) {
            tool = TOOL_SELECT;
        }
    } else if (event.type == SDL_KEYDOWN) {
        if (entity_settings_entity != -1) {
            switch (event.key.keysym.sym) {
                case SDLK_ESCAPE:
                    close_entity_settings(entity_settings_id);
                    break;
                default:
                    return;
            }
        }

        switch (event.key.keysym.sym) {
            case SDLK_1:
            case SDLK_2:
            case SDLK_3:
            case SDLK_4:
            case SDLK_5:
            case SDLK_6:
                selected_categories[event.key.keysym.sym - SDLK_1] = !selected_categories[event.key.keysym.sym - SDLK_1];
                break;
            case SDLK_o:
                toggle_objects(-1);
                break;
            case SDLK_t:
                toggle_tiles(-1);
                break;
            case SDLK_w:
                toggle_weapons(-1);
                break;
            case SDLK_PLUS:
                grid_size_index = maxi(grid_size_index - 1, 0);
                break;
            case SDLK_MINUS:
                grid_size_index = mini(grid_size_index + 1, LENGTH(grid_sizes) - 1);
                break;
            case SDLK_LSHIFT:
                shift_down = true;
                break;
            case SDLK_LCTRL:
                ctrl_down = true;
                break;
            default:
                break;
        }
    } else if (event.type == SDL_KEYUP) {
        switch (event.key.keysym.sym) {
            case SDLK_LSHIFT:
                shift_down = false;
                break;
            case SDLK_LCTRL:
                ctrl_down = false;
                break;
            default:
                break;
        }
    }
}


void draw_barriers() {
    for (int i = 0; i < game_data->components->entities; i++) {
        ColliderComponent* collider = ColliderComponent_get(i);
        if (!collider) continue;
        if (collider->trigger_type != TRIGGER_NONE) continue;

        ImageComponent* image = ImageComponent_get(i);
        if (image) continue;

        WidgetComponent* widget = WidgetComponent_get(i);
        if (widget) continue;
        
        Color color = COLOR_WHITE;
        Color text_color = COLOR_WHITE;
        String text = "";

        switch (collider->group) {
            case GROUP_WALLS:
                color = get_color(0.0f, 1.0f, 0.0f, 0.25f);
                text_color = COLOR_GREEN;
                strcpy(text, "wall");
                break;
            case GROUP_BARRIERS:
                color = get_color(1.0f, 0.0f, 0.0f, 0.25f);
                text_color = COLOR_RED;
                strcpy(text, "barrier");
                break;
            default:
                continue;
        }

        if (collider->trigger_type == TRIGGER_TRAP) {
            color = get_color(1.0f, 1.0f, 0.0f, 0.25f);
            text_color = COLOR_YELLOW;
            strcpy(text, "trap");
        }

        Vector2f pos = get_position(i);
        float angle = get_angle(i);
        draw_rectangle(game_data->camera, pos, collider_width(i), collider_height(i), angle, color);
        draw_text(game_data->camera, pos, text, 20, text_color);
    }
}


void draw_editor() {
    draw_game();
    draw_grid(game_data->camera, grid_sizes[grid_size_index], grid_sizes[grid_size_index]);

    Vector2f mouse_pos = get_mouse_position(game_data->camera);
    Vector2f mouse_grid = snap_to_grid(mouse_pos, grid_sizes[grid_size_index], grid_sizes[grid_size_index]);
    Vector2f mouse_grid_center = snap_to_grid_center(mouse_pos, grid_sizes[grid_size_index], 
        grid_sizes[grid_size_index]);

    switch(tool) {
        case TOOL_SELECT:
            if (selection_box != -1) {
                Vector2f pos = get_position(selection_box);
                ColliderComponent* collider = ColliderComponent_get(selection_box);
                draw_rectangle_outline(game_data->camera, pos, collider->width, 
                    collider->height, 0.0f, 0.05f, COLOR_WHITE);
            }
            break;
        case TOOL_TILE:
            if (tile_started) {
                Vector2f end = mouse_grid;
                float width = fabsf(end.x - tile_start.x);
                float height = fabsf(end.y - tile_start.y);
                Vector2f pos = mult(0.5f, sum(end, tile_start));
                draw_rectangle_outline(game_data->camera, pos, width, height, 0.0f, 0.05f, 
                    COLOR_WHITE);
                String buffer;
                snprintf(buffer, STRING_SIZE, "%.2f", width);
                draw_text(game_data->camera, vec(pos.x, fmaxf(tile_start.y, end.y) + 0.5f), buffer, 20, COLOR_WHITE);
                snprintf(buffer, STRING_SIZE, "%.2f", height);
                draw_text(game_data->camera, vec(fmaxf(tile_start.x, end.x) + 1.0f, pos.y), buffer, 20, COLOR_WHITE);
            } else {
                Vector2f pos = mouse_grid;
                draw_line(game_data->camera, vec(pos.x - 0.2f, pos.y), vec(pos.x + 0.2f, pos.y), 0.05f, COLOR_WHITE);
                draw_line(game_data->camera, vec(pos.x, pos.y - 0.2f), vec(pos.x, pos.y + 0.2f), 0.05f, COLOR_WHITE);
            }

            draw_text(game_data->menu_camera, vec(-23.0f, -14.0f), selected_object_name, 20, COLOR_WHITE);
            break;
        case TOOL_OBJECT:
            draw_rectangle_outline(game_data->camera, mouse_grid,
                selected_object_width * selected_object_scale, 
                selected_object_height * selected_object_scale, 
                selected_object_angle, 0.05f, COLOR_WHITE);

            draw_text(game_data->menu_camera, vec(-23.0f, -14.0f), selected_object_name, 20, COLOR_WHITE);
            break;
        case TOOL_PATH:
            if (tile_started) {
                draw_line(game_data->camera, tile_start, mouse_grid, 0.1f, COLOR_WHITE);
            }

            draw_text(game_data->menu_camera, vec(-23.0f, -14.0f), selected_object_name, 20, COLOR_WHITE);
            break;
        case TOOL_FOREST:
            if (tile_started) {
                float width = fabsf(mouse_grid.x - tile_start.x);
                float height = fabsf(mouse_grid.y - tile_start.y);
                Vector2f pos = mult(0.5f, sum(mouse_grid, tile_start));
                draw_rectangle_outline(game_data->camera, pos, width, height, 0.0f, 0.05f, 
                    COLOR_GREEN);
            } else {
                Vector2f pos = mouse_grid;
                draw_line(game_data->camera, vec(pos.x - 0.2f, pos.y), vec(pos.x + 0.2f, pos.y), 0.05f, COLOR_GREEN);
                draw_line(game_data->camera, vec(pos.x, pos.y - 0.2f), vec(pos.x, pos.y + 0.2f), 0.05f, COLOR_GREEN);
            }
            break;
    }

    for (int i = 0; i < LENGTH(category_names); i++) {
        Color color = selected_categories[i] ? COLOR_WHITE : get_color(0.6f, 0.6f, 0.6f, 1.0f);
        char buffer[128];
        snprintf(buffer, 128, "%d %s", i + 1, category_names[i]);
        draw_text(game_data->menu_camera, vec(i * 5 - 15, 14), buffer, 20, color);
    }

    bool waypoint_selected = false;
    if (selections) {
        ListNode* node;
        FOREACH (node, selections) {
            int i = node->value;
            Vector2f pos = get_position(i);
            float angle = get_angle(i);

            ImageComponent* image = ImageComponent_get(i);
            ColliderComponent* collider = ColliderComponent_get(i);
            if (image) {
                if (image->layer > LAYER_WALLS) {
                    draw_sprite_outline(game_data->camera, image->texture_index, image->width, image->height, 0, pos, 
                        angle, get_scale(i));
                } else {
                    draw_rectangle_outline(game_data->camera, pos, image_width(i), image_height(i), 
                        angle, 0.05f, COLOR_WHITE);
                }
            } else if (collider) {
                draw_rectangle_outline(game_data->camera, pos, collider_width(i), 
                    collider_height(i), angle, 0.05f, COLOR_WHITE);
            } else {
                draw_circle(game_data->camera, pos, 0.1f, COLOR_WHITE);
            }

            if (WaypointComponent_get(i)) {
                waypoint_selected = true;
            }
        }
    }

    draw_waypoints(game_data->camera, waypoint_selected);
    draw_spawners();
    draw_barriers();
    draw_tutorials();
    debug_draw_paths(game_data->camera);
    draw_widgets(game_data->menu_camera);

    draw_circle(game_data->camera, mouse_pos, 0.1f, COLOR_WHITE);
    String buffer;
    snprintf(buffer, STRING_SIZE, "x: %.2f y: %.2f", mouse_grid_center.x, mouse_grid_center.y);
    draw_text(game_data->menu_camera, vec(-23.0f, -14.0f), buffer, 20, COLOR_WHITE);
}
