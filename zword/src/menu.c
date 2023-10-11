#include <string.h>
#include <stdlib.h>
#include <math.h>
#include <stdio.h>

#include "menu.h"
#include "game.h"
#include "component.h"
#include "globals.h"
#include "settings.h"
#include "widget.h"
#include "light.h"
#include "input.h"


static ButtonText RESOLUTIONS[] = {"1280x720", "1360x768", "1600x900", "1920x1080", "2560x1440", "2160x3840"};
int RESOLUTION_ID = -1;
int SOUND_ID = -1;
int MUSIC_ID = -1;
int MAP_NAME_ID = -1;
static ButtonText map_name = "mansion";
static int window_play = -1;
static int window_new_map = -1;
static int window_editor = -1;
static int window_settings = -1;
static int window_controls = -1;


void reset_ids() {
    window_play = -1;
    window_new_map = -1;
    window_editor = -1;
    window_settings = -1;
    window_controls = -1;
}


void get_map_name(GameData* data, ButtonText buffer) {
    WidgetComponent* widget = WidgetComponent_get(data->components, MAP_NAME_ID);
    if (widget) {
        strcpy(buffer, widget->string);
    } else {
        strcpy(buffer, map_name);
    }
}


void change_state_end(ComponentData* components, int entity) {
    UNUSED(components);
    UNUSED(entity);
    game_state = STATE_END;
    reset_ids();
}


void change_state_create(ComponentData* components, int entity) {
    UNUSED(components);
    UNUSED(entity);
    game_state = STATE_CREATE;
    reset_ids();
}


void change_state_start(ComponentData* components, int entity) {
    WidgetComponent* widget = WidgetComponent_get(components, entity);
    strcpy(map_name, widget->string);
    game_state = STATE_START;
    reset_ids();
}


void change_state_game(ComponentData* components, int entity) {
    UNUSED(components);
    UNUSED(entity);
    game_state = STATE_GAME;
    reset_ids();
}


void change_state_load(ComponentData* components, int entity) {
    WidgetComponent* widget = WidgetComponent_get(components, entity);
    strcpy(map_name, widget->string);
    game_state = STATE_LOAD;
    reset_ids();
}


void change_state_quit(ComponentData* components, int entity) {
    UNUSED(components);
    UNUSED(entity);
    game_state = STATE_QUIT;
}


void apply(ComponentData* components, int entity) {
    UNUSED(entity);

    WidgetComponent* widget = WidgetComponent_get(components, RESOLUTION_ID);
    ButtonText text;
    strcpy(text, widget->strings[widget->value]);
    char* width = strtok(text, "x");
    char* height = strtok(NULL, "x");
    game_settings.width = strtol(width, NULL, 10);
    game_settings.height = strtol(height, NULL, 10);

    widget = WidgetComponent_get(components, SOUND_ID);
    game_settings.volume = widget->value;

    save_settings();
    game_state = STATE_APPLY;
}


void toggle_play(ComponentData* components, int entity) {
    UNUSED(entity);
    if (window_play != -1) {
        destroy_entity_recursive(components, window_play);
        window_play = -1;
        return;
    }

    sfVector2f pos = sum(vec(0.0f, 2 * BUTTON_HEIGHT), mult(BUTTON_HEIGHT, rand_vector()));
    window_play = create_window(components, pos, "SURVIVAL", 1, toggle_play);

    int container = create_container(components, vec(0.0f, -3 * BUTTON_HEIGHT), 1, 5);
    add_child(components, window_play, container);

    add_files_to_container(components, container, "maps", change_state_start);
}


void toggle_new_map(ComponentData* components, int entity) {
    UNUSED(entity);
    if (window_new_map != -1) {
        destroy_entity_recursive(components, window_new_map);
        window_new_map = -1;
        return;
    }

    sfVector2f pos = sum(vec(0.0f, 2 * BUTTON_HEIGHT), mult(BUTTON_HEIGHT, rand_vector()));
    window_new_map = create_window(components, pos, "NEW MAP", 2, toggle_new_map);

    int container = create_container(components, vec(0.0f, -1.5f * BUTTON_HEIGHT), 2, 2);
    add_child(components, window_new_map, container);

    int label = create_label(components, "NAME", zeros());
    MAP_NAME_ID = create_textbox(components, zeros(), 1);
    add_row_to_container(components, container, label, MAP_NAME_ID);
    add_button_to_container(components, container, "CREATE", change_state_create);
}


void toggle_editor(ComponentData* components, int entity) {
    UNUSED(entity);
    if (window_editor != -1) {
        destroy_entity_recursive(components, window_editor);
        window_editor = -1;
        return;
    }

    sfVector2f pos = sum(vec(0.0f, 2 * BUTTON_HEIGHT), mult(BUTTON_HEIGHT, rand_vector()));
    window_editor = create_window(components, pos, "EDITOR", 1, toggle_editor);

    int container = create_container(components, vec(0.0f, -3 * BUTTON_HEIGHT), 1, 5);
    add_child(components, window_editor, container);

    add_button_to_container(components, container, "NEW MAP", toggle_new_map);
    add_files_to_container(components, container, "maps", change_state_load);
}


void toggle_settings(ComponentData* components, int entity) {
    UNUSED(entity);
    if (window_settings != -1) {
        destroy_entity_recursive(components, window_settings);
        window_settings = -1;
        return;
    }

    sfVector2f pos = sum(vec(0.0f, 2 * BUTTON_HEIGHT), mult(BUTTON_HEIGHT, rand_vector()));
    window_settings = create_window(components, pos, "SETTINGS", 2, toggle_settings);

    int container = create_container(components, vec(0.0f, -3 * BUTTON_HEIGHT), 2, 5);
    add_child(components, window_settings, container);

    int label = create_label(components, "Resolution", zeros());
    RESOLUTION_ID = create_dropdown(components, zeros(), RESOLUTIONS, sizeof(RESOLUTIONS) / sizeof(RESOLUTIONS[0]));
    // TODO: show current resolution
    add_row_to_container(components, container, label, RESOLUTION_ID);

    label = create_label(components, "Sound", zeros());
    SOUND_ID = create_slider(components, zeros(), 0, 100, game_settings.volume, NULL);
    add_row_to_container(components, container, label, SOUND_ID);

    add_button_to_container(components, container, "Apply", apply);
}


void toggle_controls(ComponentData* components, int entity) {
    UNUSED(entity);
    if (window_controls != -1) {
        destroy_entity_recursive(components, window_controls);
        window_controls = -1;
        return;
    }

    sfVector2f pos = sum(vec(0.0f, 2 * BUTTON_HEIGHT), mult(BUTTON_HEIGHT, rand_vector()));
    window_controls = create_window(components, pos, "CONTROLS", 2, toggle_controls);

    int container = create_container(components, vec(0.0f, -3 * BUTTON_HEIGHT), 2, 5);
    add_child(components, window_controls, container);

    for (int i = 0; i < 12; i++) {
        int label = create_label(components, ACTIONS[i], zeros());
        int button = create_button(components, BUTTON_NAMES[i], zeros(), NULL);
        add_row_to_container(components, container, label, button);
    }
    add_scrollbar_to_container(components, container);
}


void create_menu(GameData data) {
    int container = create_container(data.components, vec(-20.0f, 0.0f), 1, 4);
    add_button_to_container(data.components, container, "SURVIVAL", toggle_play);
    add_button_to_container(data.components, container, "EDITOR", toggle_editor);
    add_button_to_container(data.components, container, "SETTINGS", toggle_settings);
    add_button_to_container(data.components, container, "CONTROLS", toggle_controls);
    add_button_to_container(data.components, container, "QUIT", change_state_quit);
}


void destroy_menu(GameData data) {
    for (int i = 0; i < data.components->entities; i++) {
        WidgetComponent* widget = WidgetComponent_get(data.components, i);
        if (widget) {
            destroy_entity(data.components, i);
        }
    }
}


void create_pause_menu(GameData* data) {
    int container = create_container(data->components, vec(-20.0f, 0.0f), 1, 3);
    add_button_to_container(data->components, container, "RESUME", change_state_game);
    add_button_to_container(data->components, container, "SETTINGS", toggle_settings);
    add_button_to_container(data->components, container, "QUIT TO MENU", change_state_end);
}


void update_menu(GameData data, sfRenderWindow* window) {
    update_widgets(data.components, window, data.menu_camera);
}


void input_menu(ComponentData* components, int camera, sfEvent event) {
    input_widgets(components, camera, event);
}


void draw_menu(GameData data, sfRenderWindow* window) {
    draw_widgets(data.components, window, data.menu_camera);

    sfVector2f pos = screen_to_world(data.components, data.menu_camera, sfMouse_getPosition((sfWindow*) window));
    draw_circle(window, data.components, data.menu_camera, NULL, pos, 0.1f, sfWhite);
}


void create_game_over_menu(GameData data) {
    create_button(data.components, "Restart", vec(0.0f, -1.0f * BUTTON_HEIGHT), change_state_start);
    create_button(data.components, "Quit", vec(0.0f, -2.0f * BUTTON_HEIGHT), change_state_end);
}
