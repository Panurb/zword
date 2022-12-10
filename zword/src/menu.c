#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>

#include "menu.h"
#include "game.h"
#include "component.h"
#include "collider.h"
#include "globals.h"
#include "settings.h"
#include "widget.h"
#include "light.h"


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
    window_id = create_window(components, pos, "NEW GAME", toggle_play);

    int container = create_container(components, vec(0.0f, -3 * BUTTON_HEIGHT), 2, 5);
    add_child(components, window_id, container);

    add_button_to_container(components, container, "START", change_state_start);
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
    window_id = create_window(components, pos, "SETTINGS", toggle_settings);

    int container = create_container(components, vec(0.0f, -3 * BUTTON_HEIGHT), 2, 5);
    add_child(components, window_id, container);

    int label = create_label(components, "Resolution", zeros());
    RESOLUTION_ID = create_dropdown(components, zeros(), RESOLUTIONS, sizeof(RESOLUTIONS) / sizeof(RESOLUTIONS[0]));
    // TODO: show current resolution
    add_row_to_container(components, container, label, RESOLUTION_ID);

    label = create_label(components, "Sound", zeros());
    SOUND_ID = create_slider(components, zeros(), 0, 100, game_settings.volume);
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
    window_id = create_window(components, pos, "CONTROLS", toggle_controls);

    int container = create_container(components, vec(0.0f, -3 * BUTTON_HEIGHT), 2, 5);
    add_child(components, window_id, container);

    for (int i = 0; i < 10; i++) {
        int label = create_label(components, "action", zeros());
        int button = create_button(components, "key", zeros(), NULL);
        add_row_to_container(components, container, label, button);
    }
}


void create_menu(GameData data) {
    int container = create_container(data.components, vec(-20.0f, 0.0f), 1, 4);
    add_button_to_container(data.components, container, "NEW GAME", toggle_play);
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
    update_widgets(data.components, window, data.camera);
}


void input_menu(ComponentData* components, int camera, sfEvent event) {
    static sfVector2f mouse_position = { 0.0f, 0.0f };
    static bool mouse_down = false;
    static int grabbed_window = -1;
    static sfVector2f grab_offset = { 0.0f, 0.0f };
    
    if (event.type == sfEvtMouseMoved) {
        sfVector2i mouse_screen = { event.mouseMove.x, event.mouseMove.y };
        mouse_position = screen_to_world(components, camera, mouse_screen);
    } else if (event.type == sfEvtMouseButtonPressed && event.mouseButton.button == sfMouseLeft) {
        mouse_down = true;
    } else if (event.type == sfEvtMouseButtonReleased && event.mouseButton.button == sfMouseLeft) {
        mouse_down = false;
        grabbed_window = -1;
    }

    for (int i = 0; i < components->entities; i++) {
        WidgetComponent* widget = WidgetComponent_get(components, i);
        if (!widget) continue;
        if (!widget->selected) continue;

        CoordinateComponent* coord = CoordinateComponent_get(components, i);

        if (event.type == sfEvtMouseMoved) {
            if (widget->type == WIDGET_WINDOW) {
                if (i == grabbed_window) {
                    coord->position = sum(mouse_position, grab_offset);
                }
            } else if (widget->type == WIDGET_SLIDER) {
                if (mouse_down) {
                    set_slider(components, i, mouse_position);
                }
            }
        } else if (event.type == sfEvtMouseButtonPressed && event.mouseButton.button == sfMouseLeft) {
            if (widget->type == WIDGET_WINDOW) {
                grabbed_window = i;
                grab_offset = diff(coord->position, mouse_position);
            } else if (widget->type == WIDGET_SLIDER) {
                set_slider(components, i, mouse_position);
            }
            int root = get_root(components, i);
            bring_to_top(components, root);
        } else if (event.type == sfEvtMouseButtonReleased && event.mouseButton.button == sfMouseLeft) {
            if (widget->on_click) {
                widget->on_click(components, i);
            }
        } else if (event.type == sfEvtMouseWheelScrolled) {
            if (widget->type == WIDGET_CONTAINER || widget->type == WIDGET_SPINBOX) {
                increment_value(components, i, (int) -event.mouseWheelScroll.delta);
            } else {
                if (coord->parent) {
                    WidgetComponent* parent = WidgetComponent_get(components, coord->parent);
                    if (parent && parent->type == WIDGET_CONTAINER) {
                        increment_value(components, coord->parent, (int) -event.mouseWheelScroll.delta);
                    }
                }
            }
        }

        break;
    }
}


void draw_menu(GameData data, sfRenderWindow* window) {
    draw_widgets(data.components, window, data.camera);

    sfVector2f pos = screen_to_world(data.components, data.camera, sfMouse_getPosition((sfWindow*) window));
    draw_circle(window, data.components, data.camera, NULL, pos, 0.1f, sfWhite);
}


void update_widgets(ComponentData* components, sfRenderWindow* window, int camera) {
    int last_selected = -1;
    for (ListNode* node = components->widget.order->head; node; node = node->next) {
        int i = node->value;
        WidgetComponent* widget = WidgetComponent_get(components, i);
        if (!widget->enabled) continue;

        sfVector2f mouse = screen_to_world(components, camera, sfMouse_getPosition((sfWindow*) window));
        mouse = diff(mouse, get_position(components, camera));
        widget->selected = false;
        if (inside_collider(components, i, mouse)) {
            last_selected = i;
        }
    }
    if (last_selected != -1) {
        WidgetComponent_get(components, last_selected)->selected = true;
    }
}


void draw_widgets(ComponentData* components, sfRenderWindow* window, int camera) {
    for (ListNode* node = components->widget.order->head; node; node = node->next) {
        int i = node->value;
        WidgetComponent* widget = WidgetComponent_get(components, i);
        if (!widget->enabled) continue;

        sfVector2f pos = sum(get_position(components, camera), get_position(components, i));
        ColliderComponent* collider = ColliderComponent_get(components, i);

        sfColor color = get_color(0.2f, 0.2f, 0.2f, 1.0f);
        switch (widget->type) {
            case WIDGET_CONTAINER:
                color = get_color(0.1f, 0.1f, 0.1f, 1.0f);
                draw_rectangle(window, components, camera, NULL, pos, collider->width, collider->height, 0.0f, color);
                break;
            case WIDGET_LABEL:
                break;
            case WIDGET_BUTTON:
                color = get_color(0.2f, 0.2f, 0.2f, 1.0f);
                draw_rectangle(window, components, camera, NULL, pos, collider->width, collider->height, 0.0f, color);

                color = get_color(0.3f, 0.3f, 0.3f, 1.0f);
                float w = collider->width;
                float h = collider->height;
                draw_rectangle(window, components, camera, NULL, sum(pos, vec(-0.1f, 0.1f)), w - 0.2f, h - 0.2f, 0.0f, color);
                break;
            case WIDGET_DROPDOWN:
                color = get_color(0.2f, 0.2f, 0.2f, 1.0f);
                draw_rectangle(window, components, camera, NULL, pos, collider->width, collider->height, 0.0f, color);

                color = get_color(0.3f, 0.3f, 0.3f, 1.0f);
                w = collider->width;
                h = collider->height;
                sfVector2f r = sum(pos, vec(-0.1f, 0.1f));
                draw_rectangle(window, components, camera, NULL, r, w - 0.2f, h - 0.2f, 0.0f, color);

                r = sum(pos, vec(0.5f * BUTTON_WIDTH - 0.5f * BUTTON_HEIGHT, 0.0f));
                if (CoordinateComponent_get(components, i)->children->size == 0) {
                    draw_text(window, components, camera, NULL, r, "v", sfWhite);
                } else {
                    draw_text(window, components, camera, NULL, r, "^", sfWhite);
                }
                break;
            case WIDGET_SLIDER:
                draw_rectangle(window, components, camera, NULL, pos, collider->width, collider->height, 0.0f, color);
                char buffer[255];
                sprintf(buffer, "%d", widget->value);
                draw_text(window, components, camera, NULL, pos, buffer, sfWhite);
                break;
            default:
                draw_rectangle(window, components, camera, NULL, pos, collider->width, collider->height, 0.0f, color);
        }

        if (widget->strings) {
            draw_text(window, components, camera, widget->text, pos, widget->strings[widget->value], sfWhite);
        } else {
            draw_text(window, components, camera, widget->text, pos, widget->string, sfWhite);
        }
    }
}
