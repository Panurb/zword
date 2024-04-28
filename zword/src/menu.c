#include <string.h>
#include <stdlib.h>
#include <math.h>
#include <stdio.h>

#include "menu.h"
#include "game.h"
#include "component.h"
#include "settings.h"
#include "widget.h"
#include "light.h"
#include "input.h"


static ButtonText RESOLUTIONS[] = {"1280x720", "1360x768", "1600x900", "1920x1080", "2560x1440", "3840x2160"};
int RESOLUTION_ID = -1;
int SOUND_ID = -1;
int MUSIC_ID = -1;
int fullscreen_id = -1;
static int map_name_textbox = -1;
static int window_play = -1;
static int window_new_map = -1;
static int window_editor = -1;
static int window_settings = -1;
static int window_controls = -1;
static int window_credits = -1;
static int window_keyboard_controls = -1;
static int window_xbox_controls = -1;


void reset_ids() {
    RESOLUTION_ID = -1;
    SOUND_ID = -1;
    MUSIC_ID = -1;
    fullscreen_id = -1;
    window_play = -1;
    window_new_map = -1;
    window_editor = -1;
    window_settings = -1;
    window_controls = -1;
    window_credits = -1;
    window_keyboard_controls = -1;
    window_xbox_controls = -1;
}


void change_state_end(int entity) {
    UNUSED(entity);
    game_state = STATE_END;
    reset_ids();
}


void change_state_create(int entity) {
    UNUSED(entity);
    strcpy(game_data->map_name, WidgetComponent_get(map_name_textbox)->string);
    game_state = STATE_CREATE;
    reset_ids();
}


void change_state_start(int entity) {
    WidgetComponent* widget = WidgetComponent_get(entity);
    strcpy(game_data->map_name, widget->string);
    game_state = STATE_START;
    reset_ids();
}


void change_state_reset(int entity) {
    UNUSED(entity);
    game_state = STATE_RESET;
    reset_ids();
}


void change_state_game(int entity) {
    UNUSED(entity);
    game_state = STATE_GAME;
    reset_ids();
}


void change_state_load(int entity) {
    WidgetComponent* widget = WidgetComponent_get(entity);
    strcpy(game_data->map_name, widget->string);
    game_state = STATE_LOAD;
    reset_ids();
}


void change_state_quit(int entity) {
    UNUSED(entity);
    game_state = STATE_QUIT;
}


void apply(int entity) {
    UNUSED(entity);

    WidgetComponent* widget = WidgetComponent_get(RESOLUTION_ID);
    ButtonText text;
    strcpy(text, widget->strings[widget->value]);
    char* width = strtok(text, "x");
    char* height = strtok(NULL, "x");
    game_settings.width = strtol(width, NULL, 10);
    game_settings.height = strtol(height, NULL, 10);

    widget = WidgetComponent_get(fullscreen_id);
    game_settings.fullscreen = widget->value;

    widget = WidgetComponent_get(SOUND_ID);
    game_settings.volume = widget->value;

    widget = WidgetComponent_get(MUSIC_ID);
    game_settings.music = widget->value;

    game_state = STATE_APPLY;
}


void toggle_play(int entity) {
    UNUSED(entity);
    if (window_play != -1) {
        destroy_entity_recursive(window_play);
        window_play = -1;
        return;
    }

    Vector2f pos = sum(vec(0.0f, 2 * BUTTON_HEIGHT), mult(BUTTON_HEIGHT, rand_vector()));
    window_play = create_window(pos, "PLAY", 1, toggle_play);

    int container = create_container(vec(0.0f, -1.5f * BUTTON_HEIGHT), 1, 2);
    add_child(window_play, container);

    // add_button_to_container(container, "Tutorial", change_state_start);
    // add_button_to_container(container, "Survival", change_state_start);
    add_files_to_container(container, "maps", change_state_start);
}


void toggle_new_map(int entity) {
    UNUSED(entity);
    if (window_new_map != -1) {
        destroy_entity_recursive(window_new_map);
        window_new_map = -1;
        return;
    }

    Vector2f pos = sum(vec(0.0f, 2 * BUTTON_HEIGHT), mult(BUTTON_HEIGHT, rand_vector()));
    window_new_map = create_window(pos, "NEW MAP", 2, toggle_new_map);

    int container = create_container(vec(0.0f, -1.5f * BUTTON_HEIGHT), 2, 2);
    add_child(window_new_map, container);

    int label = create_label("NAME", zeros());
    map_name_textbox = create_textbox(zeros(), 1);
    add_row_to_container(container, label, map_name_textbox);
    add_button_to_container(container, "CREATE", change_state_create);
}


void toggle_editor(int entity) {
    UNUSED(entity);
    if (window_editor != -1) {
        destroy_entity_recursive(window_editor);
        window_editor = -1;
        return;
    }

    Vector2f pos = sum(vec(0.0f, 2 * BUTTON_HEIGHT), mult(BUTTON_HEIGHT, rand_vector()));
    window_editor = create_window(pos, "EDITOR", 1, toggle_editor);

    int container = create_container(vec(0.0f, -3 * BUTTON_HEIGHT), 1, 5);
    add_child(window_editor, container);

    add_button_to_container(container, "NEW MAP", toggle_new_map);
    add_files_to_container(container, "maps", change_state_load);
}


int get_resolution_index() {
    char resolution[128];
    snprintf(resolution, 128, "%dx%d", game_settings.width, game_settings.height);
    int size = sizeof(RESOLUTIONS) / sizeof(RESOLUTIONS[0]);
    for (int i = 0; i < size; i++) {
        if (strcmp(RESOLUTIONS[i], resolution) == 0) {
            return i;
        }
    }
    return 0;
}


void toggle_settings(int entity) {
    UNUSED(entity);
    if (window_settings != -1) {
        destroy_entity_recursive(window_settings);
        window_settings = -1;
        return;
    }

    Vector2f pos = sum(vec(0.0f, 2 * BUTTON_HEIGHT), mult(BUTTON_HEIGHT, rand_vector()));
    window_settings = create_window(pos, "SETTINGS", 2, toggle_settings);

    int container = create_container(vec(0.0f, -3.0f * BUTTON_HEIGHT), 2, 5);
    add_child(window_settings, container);

    int label = create_label("Resolution", zeros());
    RESOLUTION_ID = create_dropdown(zeros(), RESOLUTIONS, sizeof(RESOLUTIONS) / sizeof(RESOLUTIONS[0]));
    WidgetComponent_get(RESOLUTION_ID)->value = get_resolution_index();
    add_row_to_container(container, label, RESOLUTION_ID);

    label = create_label("Fullscreen", zeros());
    fullscreen_id = create_checkbox(zeros(), game_settings.fullscreen, NULL);
    add_row_to_container(container, label, fullscreen_id);

    label = create_label("Sound", zeros());
    SOUND_ID = create_slider(zeros(), 0, 100, game_settings.volume, NULL);
    add_row_to_container(container, label, SOUND_ID);

    label = create_label("Music", zeros());
    MUSIC_ID = create_slider(zeros(), 0, 100, game_settings.music, NULL);
    add_row_to_container(container, label, MUSIC_ID);

    add_button_to_container(container, "Apply", apply);
}


void toggle_keyboard_controls(int entity) {
    UNUSED(entity);
    if (window_keyboard_controls != -1) {
        destroy_entity_recursive(window_keyboard_controls);
        window_keyboard_controls = -1;
        return;
    }

    Vector2f pos = sum(vec(0.0f, 2 * BUTTON_HEIGHT), mult(BUTTON_HEIGHT, rand_vector()));
    window_keyboard_controls = create_window(pos, "KEYBOARD CONTROLS", 2, toggle_keyboard_controls);

    int container = create_container(vec(0.0f, -3 * BUTTON_HEIGHT), 2, 5);
    add_child(window_keyboard_controls, container);

    for (int i = 0; i < ACTIONS_SIZE; i++) {
        int label = create_label(ACTIONS[i], zeros());
        int button = create_button(keybind_to_string(game_settings.keybinds[i]), zeros(), NULL);
        add_row_to_container(container, label, button);
    }
    add_scrollbar_to_container(container);
}


void toggle_xbox_controls(int entity) {
    UNUSED(entity);
    if (window_xbox_controls != -1) {
        destroy_entity_recursive(window_xbox_controls);
        window_xbox_controls = -1;
        return;
    }

    Vector2f pos = sum(vec(0.0f, 2 * BUTTON_HEIGHT), mult(BUTTON_HEIGHT, rand_vector()));
    window_xbox_controls = create_window(pos, "CONTROLS", 2, toggle_xbox_controls);

    int container = create_container(vec(0.0f, -3 * BUTTON_HEIGHT), 2, 5);
    add_child(window_xbox_controls, container);

    for (int i = 0; i < ACTIONS_SIZE; i++) {
        int label = create_label(ACTIONS[i], zeros());
        int button = create_button(ACTION_BUTTONS_XBOX[i], zeros(), NULL);
        add_row_to_container(container, label, button);
    }
    add_scrollbar_to_container(container);
}


void toggle_controls(int entity) {
    UNUSED(entity);
    if (window_controls != -1) {
        destroy_entity_recursive(window_controls);
        window_controls = -1;
        return;
    }

    Vector2f pos = sum(vec(0.0f, 2 * BUTTON_HEIGHT), mult(BUTTON_HEIGHT, rand_vector()));
    window_controls = create_window(pos, "CONTROLS", 1, toggle_controls);

    int container = create_container(vec(0.0f, -1.5f * BUTTON_HEIGHT), 1, 2);
    add_child(window_controls, container);

    add_button_to_container(container, "KEYBOARD", toggle_keyboard_controls);
    add_button_to_container(container, "XBOX", toggle_xbox_controls);
}


void toggle_credits(int entity) {
    UNUSED(entity);
    if (window_credits != -1) {
        destroy_entity_recursive(window_credits);
        window_credits = -1;
        return;
    }

    Vector2f pos = sum(vec(0.0f, 2 * BUTTON_HEIGHT), mult(BUTTON_HEIGHT, rand_vector()));
    window_credits = create_window(pos, "CREDITS", 2, toggle_credits);

    int container = create_container(vec(0.0f, -3.5f * BUTTON_HEIGHT), 2, 6);
    add_child(window_credits, container);

    add_row_to_container(container, create_label("Programming, art, music", zeros()), create_label("Panu Keskinen", zeros()));
    #ifdef __EMSCRIPTEN__
        add_row_to_container(container, create_label("Made with", zeros()), create_label("C, SDL2, cJSON, Emscripten", zeros()));
    #else 
        add_row_to_container(container, create_label("Made with", zeros()), create_label("C, SDL2, cJSON", zeros()));
    #endif
    add_row_to_container(container, create_label("Software used", zeros()), create_label("Visual Studio Code", zeros()));
    add_row_to_container(container, create_label("", zeros()), create_label("Gimp", zeros()));
    add_row_to_container(container, create_label("", zeros()), create_label("Ableton Live Lite", zeros()));
    add_row_to_container(container, create_label("", zeros()), create_label("Audacity", zeros()));
}


void create_menu() {
    int height = game_settings.debug ? 6 : 5;
    int container = create_container(vec(-18.0f, -2.0f), 1, height);
    add_button_to_container(container, "PLAY", toggle_play);
    if (game_settings.debug) {
        add_button_to_container(container, "EDITOR", toggle_editor);
    }
    add_button_to_container(container, "SETTINGS", toggle_settings);
    add_button_to_container(container, "CONTROLS", toggle_controls);
    add_button_to_container(container, "CREDITS", toggle_credits);
    #ifndef __EMSCRIPTEN__
        add_button_to_container(container, "QUIT", change_state_quit);
    #endif
}


void destroy_menu() {
    for (int i = 0; i < game_data->components->entities; i++) {
        WidgetComponent* widget = WidgetComponent_get(i);
        if (widget) {
            destroy_entity(i);
        }
    }
}


void create_pause_menu() {
    int container = create_container(vec(-20.0f, 0.0f), 1, 3);
    add_button_to_container(container, "RESUME", change_state_game);
    add_button_to_container(container, "SETTINGS", toggle_settings);
    add_button_to_container(container, "QUIT TO MENU", change_state_end);
}


void update_menu() {
    update_widgets(game_data->menu_camera);
}


void input_menu(int camera, SDL_Event event) {
    input_widgets(camera, event);
}


void draw_menu() {
    draw_widgets(game_data->menu_camera);

    Vector2f pos = get_mouse_position(game_data->menu_camera);
    draw_circle(game_data->menu_camera, pos, 0.1f, COLOR_WHITE);
}


void create_game_over_menu() {
    create_button("Restart", vec(0.0f, -1.0f * BUTTON_HEIGHT), change_state_reset);
    create_button("Quit", vec(0.0f, -2.0f * BUTTON_HEIGHT), change_state_end);
}


void create_win_menu() {
    create_button("Continue", vec(0.0f, -1.0f * BUTTON_HEIGHT), change_state_end);
}
