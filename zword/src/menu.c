#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "menu.h"
#include "game.h"
#include "component.h"
#include "collider.h"
#include "globals.h"
#include "settings.h"
#include "widget.h"


static ButtonText RESOLUTIONS[] = {"1280x720", "1360x768", "1600x900", "1920x1080", "2560x1440", "2160x3840"};
static WidgetComponent* BUTTON_RESOLUTION = NULL;

static int SETTINGS_WINDOW = -1;
static int MOUSE_ENTITY;


void play() {
    game_state = STATE_START;
}


void quit() {
    game_state = STATE_QUIT;
}


void apply() {
    ButtonText text;
    strcpy(text, BUTTON_RESOLUTION->string);
    char* width = strtok(text, "x");
    char* height = strtok(NULL, "x");

    printf("%sx%s\n", width, height);

    game_settings.width = strtol(width, NULL, 10);
    game_settings.height = strtol(height, NULL, 10);
    // game_settings.antialiasing;
    save_settings();
    game_state = STATE_APPLY;
}


void change_resolution(ComponentData* components, int entity, int dir) {
    char* string = BUTTON_RESOLUTION->string;
    int len = sizeof(RESOLUTIONS) / sizeof(RESOLUTIONS[0]);
    for (int i = 0; i < len; i++) {
        if (strcmp(string, RESOLUTIONS[i]) == 0) {
            strcpy(BUTTON_RESOLUTION->string, RESOLUTIONS[(int) mod(i + dir, len)]);
            return;
        }
    }
    strcpy(BUTTON_RESOLUTION->string, RESOLUTIONS[0]);
}


void increment_value(ComponentData* components, int entity, int direction) {
    WidgetComponent* button = WidgetComponent_get(components, entity);
    sfVector2f v = vec(0.0f, direction * BUTTON_HEIGHT);
    button->value += direction;

    if (button->value > button->max_value) {
        if (button->cyclic) {
            button->value = button->min_value;
            v.y = button->max_value - button->min_value;
        } else {
            button->value = button->max_value;
            v.y = 0.0f;
        }
    }
    if (button->value < button->min_value) {
        if (button->cyclic) {
            button->value = button->min_value;
            v.y = button->min_value - button->max_value;
        } else {
            button->value = button->min_value;
            v.y = 0.0f;
        }
    }

    float height = ColliderComponent_get(components, entity)->height;
    CoordinateComponent* coord = CoordinateComponent_get(components, entity);
    for (ListNode* node = coord->children->head; node; node = node->next) {
        CoordinateComponent* coord_child = CoordinateComponent_get(components, node->value);
        coord_child->position = sum(coord_child->position, v);

        sfVector2f pos = coord_child->position;
        WidgetComponent_get(components, node->value)->enabled = (pos.y > -0.5f * height && pos.y < 0.5f * height);
    }
}


void close_settings(ComponentData* components, int entity) {
    destroy_entity_recursive(components, SETTINGS_WINDOW);
    SETTINGS_WINDOW = -1;
}


void open_settings(ComponentData* components, int entity) {
    if (SETTINGS_WINDOW != -1) {
        return;
    }

    SETTINGS_WINDOW = create_window(components, zeros());

    int i = create_button_small(components, "X", vec(4.0f, 0.0f), close_settings);
    add_child(components, SETTINGS_WINDOW, i);

    i = create_dropdown(components, vec(0.0f, -BUTTON_HEIGHT), RESOLUTIONS, sizeof(RESOLUTIONS) / sizeof(RESOLUTIONS[0]));
    add_child(components, SETTINGS_WINDOW, i);
}


void create_menu(GameData data) {
    MOUSE_ENTITY = create_entity(data.components);
    CoordinateComponent_add(data.components, MOUSE_ENTITY, zeros(), 0.0f);

    // create_button(data.components, "PLAY", (sfVector2f) { 0.0f, 5.0f }, MENU_MAIN, play);
    create_button(data.components, "SETTINGS", (sfVector2f) { 0.0f, 0.0f }, open_settings);
    // create_button(data.components, "QUIT", (sfVector2f) { 0.0f, -5.0f }, MENU_MAIN, quit);

    // create_button(data.components, "RESOLUTION", (sfVector2f) { 0.0f, 5.0f }, MENU_SETTINGS, NULL);
    // create_spinbox(data.components, vec(0.0f, 4.0f), MENU_SETTINGS, change_resolution);

    // create_button(data.components, "VOLUME", (sfVector2f) { 0.0f, 0.0f }, MENU_SETTINGS, NULL);
    // create_button(data.components, "APPLY", (sfVector2f) { 5.0f, -5.0f }, MENU_SETTINGS, apply);
    // create_button(data.components, "BACK", (sfVector2f) { -5.0f, -5.0f }, MENU_SETTINGS, back);
}


void update_menu(GameData data, sfRenderWindow* window) {
    update_buttons(data.components, window, data.camera);
}


void input_menu(ComponentData* components, int camera, sfEvent event) {
    for (int i = 0; i < components->entities; i++) {
        WidgetComponent* button = WidgetComponent_get(components, i);
        if (!button) continue;

        CoordinateComponent* mouse_coord = CoordinateComponent_get(components, MOUSE_ENTITY);
        CoordinateComponent* coord = CoordinateComponent_get(components, i);

        if (button->selected) {
            if (event.type == sfEvtMouseMoved) {
                sfVector2f v = screen_to_world(components, camera, (sfVector2i) { event.mouseMove.x, event.mouseMove.y });
                mouse_coord->position = v;
            }
            if (event.type == sfEvtMouseButtonPressed) {
                if (button->type == WIDGET_WINDOW) {
                    add_child(components, MOUSE_ENTITY, i);
                    coord->position = diff(coord->position, mouse_coord->position);
                }
            } else if (event.type == sfEvtMouseButtonReleased && event.mouseButton.button == sfMouseLeft) {
                if (button->type == WIDGET_WINDOW) {
                    coord->position = get_position(components, i);
                    remove_children(components, MOUSE_ENTITY);
                }
                if (button->on_click) {
                    button->on_click(components, i);
                }
            } else if (event.type == sfEvtMouseWheelScrolled) {
                if (button->type == WIDGET_CONTAINER || button->type == WIDGET_SPINBOX) {
                    increment_value(components, i, (int) -event.mouseWheelScroll.delta);
                }
            }
        }
    }
}


void draw_menu(GameData data, sfRenderWindow* window) {
    draw_buttons(data.components, window, data.camera);

    sfVector2f pos = screen_to_world(data.components, data.camera, sfMouse_getPosition((sfWindow*) window));
    draw_circle(window, data.components, data.camera, NULL, pos, 0.1f, sfWhite);
}


void update_buttons(ComponentData* components, sfRenderWindow* window, int camera) {
    int last_selected = -1;
    for (int i = 0; i < components->entities; i++) {
        WidgetComponent* button = WidgetComponent_get(components, i);
        if (!button) continue;
        if (!button->enabled) continue;

        sfVector2f mouse = screen_to_world(components, camera, sfMouse_getPosition((sfWindow*) window));
        button->selected = false;
        if (inside_collider(components, i, mouse)) {
            last_selected = i;
        }
    }
    if (last_selected != -1) {
        WidgetComponent_get(components, last_selected)->selected = true;
    }
}


void draw_buttons(ComponentData* components, sfRenderWindow* window, int camera) {
    for (int i = 0; i < components->entities; i++) {
        WidgetComponent* button = WidgetComponent_get(components, i);
        if (!button) continue;
        if (!button->enabled) continue;

        sfVector2f pos = get_position(components, i);
        ColliderComponent* collider = ColliderComponent_get(components, i);
        sfColor color = get_color(0.2f, 0.2f, 0.2f, 1.0f);
        draw_rectangle(window, components, camera, NULL, pos, collider->width, collider->height, 0.0f, color);

        if (button->strings) {
            draw_text(window, components, camera, button->text, pos, button->strings[button->value], sfWhite);
        } else {
            draw_text(window, components, camera, button->text, pos, button->string, sfWhite);
        }

        if (button->selected) {
            sfVector2f corners[4];
            get_corners(components, i, corners);
            draw_line(window, components, camera, NULL, corners[1], corners[2], 0.1f, sfMagenta);
            draw_line(window, components, camera, NULL, corners[2], corners[3], 0.1f, sfMagenta);
            draw_line(window, components, camera, NULL, corners[3], corners[0], 0.1f, sfMagenta);
            draw_line(window, components, camera, NULL, corners[0], corners[1], 0.1f, sfMagenta);
        }

        switch (button->type) {
        case WIDGET_WINDOW:
            // draw_rectangle(window, components, camera, NULL, pos, collider->width, collider->height);
            break;
        default:
            break;
        }
    }
}
