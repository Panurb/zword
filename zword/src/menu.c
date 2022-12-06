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


static ButtonText RESOLUTIONS[] = {"1280x720", "1360x768", "1600x900", "1920x1080", "2560x1440", "2160x3840"};
int RESOLUTION_ID = -1;
int SOUND_ID = -1;

static int MOUSE_ENTITY;


void play(ComponentData* components, int entity) {
    UNUSED(entity);
    game_state = STATE_START;
}


void quit(ComponentData* components, int entity) {
    UNUSED(entity);
    game_state = STATE_QUIT;
}


void apply(ComponentData* components, int entity) {
    UNUSED(entity);
    WidgetComponent* widget = WidgetComponent_get(components, RESOLUTION_ID);
    ButtonText text;
    strcpy(text, RESOLUTIONS[widget->value]);
    char* width = strtok(text, "x");
    char* height = strtok(NULL, "x");

    printf("%d\n", widget->value);
    printf("%sx%s\n", width, height);

    game_settings.width = strtol(width, NULL, 10);
    game_settings.height = strtol(height, NULL, 10);
    // game_settings.antialiasing;
    save_settings();
    game_state = STATE_APPLY;
}


void change_resolution(ComponentData* components, int entity, int dir) {
    UNUSED(entity);
    WidgetComponent* widget = WidgetComponent_get(components, RESOLUTION_ID);
    char* string = widget->string;
    int len = sizeof(RESOLUTIONS) / sizeof(RESOLUTIONS[0]);
    for (int i = 0; i < len; i++) {
        if (strcmp(string, RESOLUTIONS[i]) == 0) {
            strcpy(widget->string, RESOLUTIONS[(int) mod(i + dir, len)]);
            return;
        }
    }
    strcpy(widget->string, RESOLUTIONS[0]);
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


void toggle_settings(ComponentData* components, int entity) {
    UNUSED(entity);
    static int settings_window = -1;
    if (settings_window != -1) {
        destroy_entity_recursive(components, settings_window);
        settings_window = -1;
        return;
    }

    settings_window = create_window(components, zeros(), "SETTINGS");

    int i = create_button_small(components, "X", vec(BUTTON_WIDTH - 0.5 * BUTTON_HEIGHT, 0.0f), toggle_settings);
    add_child(components, settings_window, i);

    int container = create_container(components, vec(0.0f, -3 * BUTTON_HEIGHT), 2, 5);
    add_child(components, settings_window, container);

    int label = create_label(components, "Resolution", zeros());
    RESOLUTION_ID = create_dropdown(components, zeros(), RESOLUTIONS, sizeof(RESOLUTIONS) / sizeof(RESOLUTIONS[0]));
    add_row_to_container(components, container, label, RESOLUTION_ID);

    label = create_label(components, "Sound", zeros());
    SOUND_ID = create_slider(components, zeros(), 0, 100);
    add_row_to_container(components, container, label, SOUND_ID);

    add_button_to_container(components, container, "Apply", apply);
}


void create_menu(GameData data) {
    MOUSE_ENTITY = create_entity(data.components);
    CoordinateComponent_add(data.components, MOUSE_ENTITY, zeros(), 0.0f);

    int container = create_container(data.components, vec(-20.0f, 0.0f), 1, 3);
    add_button_to_container(data.components, container, "PLAY", play);
    add_button_to_container(data.components, container, "SETTINGS", toggle_settings);
    add_button_to_container(data.components, container, "QUIT", quit);
}


void update_menu(GameData data, sfRenderWindow* window) {
    update_widgets(data.components, window, data.camera);
}


void input_menu(ComponentData* components, int camera, sfEvent event) {
    static bool mouse_down = false;
    if (event.type == sfEvtMouseButtonPressed && event.mouseButton.button == sfMouseLeft) {
        mouse_down = true;
    } else if (event.type == sfEvtMouseButtonReleased && event.mouseButton.button == sfMouseLeft) {
        mouse_down = false;
    }

    for (int i = 0; i < components->entities; i++) {
        WidgetComponent* widget = WidgetComponent_get(components, i);
        if (!widget) continue;

        CoordinateComponent* mouse_coord = CoordinateComponent_get(components, MOUSE_ENTITY);
        CoordinateComponent* coord = CoordinateComponent_get(components, i);

        if (widget->selected) {
            if (event.type == sfEvtMouseMoved) {
                sfVector2f v = screen_to_world(components, camera, (sfVector2i) { event.mouseMove.x, event.mouseMove.y });
                mouse_coord->position = v;
                if (widget->type == WIDGET_SLIDER) {
                    if (mouse_down) {
                        set_slider(components, i, mouse_coord->position);
                    }
                }
            }
            if (event.type == sfEvtMouseButtonPressed && event.mouseButton.button == sfMouseLeft) {
                if (widget->type == WIDGET_WINDOW) {
                    add_child(components, MOUSE_ENTITY, i);
                    coord->position = diff(coord->position, mouse_coord->position);
                } else if (widget->type == WIDGET_SLIDER) {
                    set_slider(components, i, mouse_coord->position);
                }
            } else if (event.type == sfEvtMouseButtonReleased && event.mouseButton.button == sfMouseLeft) {
                if (widget->type == WIDGET_WINDOW) {
                    coord->position = get_position(components, i);
                    remove_children(components, MOUSE_ENTITY);
                }
                if (widget->on_click) {
                    widget->on_click(components, i);
                }
            } else if (event.type == sfEvtMouseWheelScrolled) {
                if (widget->type == WIDGET_CONTAINER || widget->type == WIDGET_SPINBOX) {
                    increment_value(components, i, (int) -event.mouseWheelScroll.delta);
                }
            }
        }
    }
}


void draw_menu(GameData data, sfRenderWindow* window) {
    draw_widgets(data.components, window, data.camera);

    sfVector2f pos = screen_to_world(data.components, data.camera, sfMouse_getPosition((sfWindow*) window));
    draw_circle(window, data.components, data.camera, NULL, pos, 0.1f, sfWhite);
}


void update_widgets(ComponentData* components, sfRenderWindow* window, int camera) {
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


void draw_widgets(ComponentData* components, sfRenderWindow* window, int camera) {
    for (int i = 0; i < components->entities; i++) {
        WidgetComponent* widget = WidgetComponent_get(components, i);
        if (!widget) continue;
        if (!widget->enabled) continue;

        sfVector2f pos = get_position(components, i);
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

        if (widget->selected) {
            sfVector2f corners[4];
            get_corners(components, i, corners);
            draw_line(window, components, camera, NULL, corners[1], corners[2], 0.1f, sfMagenta);
            draw_line(window, components, camera, NULL, corners[2], corners[3], 0.1f, sfMagenta);
            draw_line(window, components, camera, NULL, corners[3], corners[0], 0.1f, sfMagenta);
            draw_line(window, components, camera, NULL, corners[0], corners[1], 0.1f, sfMagenta);
        }
    }
}
