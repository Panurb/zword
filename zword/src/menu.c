#include <string.h>
#include <stdlib.h>
#include <math.h>

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


void change_state_start(ComponentData* components, int entity) {
    UNUSED(components);
    UNUSED(entity);
    game_state = STATE_START;
}


void change_state_game(ComponentData* components, int entity) {
    UNUSED(components);
    UNUSED(entity);
    game_state = STATE_GAME;
}


void change_state_editor(ComponentData* components, int entity) {
    UNUSED(components);
    UNUSED(entity);
    game_state = STATE_LOAD;
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
    static int window_id = -1;
    if (window_id != -1) {
        destroy_entity_recursive(components, window_id);
        window_id = -1;
        return;
    }

    sfVector2f pos = sum(vec(0.0f, 2 * BUTTON_HEIGHT), mult(BUTTON_HEIGHT, rand_vector()));
    window_id = create_window(components, pos, "NEW GAME", 2, toggle_play);

    int container = create_container(components, vec(0.0f, -3 * BUTTON_HEIGHT), 2, 5);
    add_child(components, window_id, container);

    add_button_to_container(components, container, "START", change_state_start);
}


void toggle_editor(ComponentData* components, int entity) {
    UNUSED(entity);
    static int window_id = -1;
    if (window_id != -1) {
        destroy_entity_recursive(components, window_id);
        window_id = -1;
        return;
    }

    sfVector2f pos = sum(vec(0.0f, 2 * BUTTON_HEIGHT), mult(BUTTON_HEIGHT, rand_vector()));
    window_id = create_window(components, pos, "EDITOR", 2, toggle_editor);

    int container = create_container(components, vec(0.0f, -3 * BUTTON_HEIGHT), 2, 5);
    add_child(components, window_id, container);

    add_button_to_container(components, container, "OPEN", change_state_editor);
}


void toggle_settings(ComponentData* components, int entity) {
    UNUSED(entity);
    static int window_id = -1;
    if (window_id != -1) {
        destroy_entity_recursive(components, window_id);
        window_id = -1;
        return;
    }

    sfVector2f pos = sum(vec(0.0f, 2 * BUTTON_HEIGHT), mult(BUTTON_HEIGHT, rand_vector()));
    window_id = create_window(components, pos, "SETTINGS", 2, toggle_settings);

    int container = create_container(components, vec(0.0f, -3 * BUTTON_HEIGHT), 2, 5);
    add_child(components, window_id, container);

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
    static int window_id = -1;
    if (window_id != -1) {
        destroy_entity_recursive(components, window_id);
        window_id = -1;
        return;
    }

    sfVector2f pos = sum(vec(0.0f, 2 * BUTTON_HEIGHT), mult(BUTTON_HEIGHT, rand_vector()));
    window_id = create_window(components, pos, "CONTROLS", 2, toggle_controls);

    int container = create_container(components, vec(0.0f, -3 * BUTTON_HEIGHT), 2, 5);
    add_child(components, window_id, container);

    for (int i = 0; i < 12; i++) {
        int label = create_label(components, ACTIONS[i], zeros());
        int button = create_button(components, BUTTON_NAMES[i], zeros(), NULL);
        add_row_to_container(components, container, label, button);
    }
    add_scrollbar_to_container(components, container);
}


void create_menu(GameData data) {
    int container = create_container(data.components, vec(-20.0f, 0.0f), 1, 4);
    add_button_to_container(data.components, container, "NEW GAME", toggle_play);
    add_button_to_container(data.components, container, "EDITOR", toggle_editor);
    add_button_to_container(data.components, container, "SETTINGS", toggle_settings);
    add_button_to_container(data.components, container, "CONTROLS", toggle_controls);
    add_button_to_container(data.components, container, "QUIT", change_state_quit);
}


void create_pause_menu(GameData data) {
    int container = create_container(data.components, vec(-20.0f, 0.0f), 1, 3);
    add_button_to_container(data.components, container, "RESUME", change_state_game);
    add_button_to_container(data.components, container, "SETTINGS", toggle_settings);
    add_button_to_container(data.components, container, "QUIT", change_state_quit);
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
