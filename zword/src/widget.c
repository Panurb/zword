#include <stdio.h>
#include <string.h>
#define NOMINMAX
#include <windows.h>
#include <stdlib.h>

#include "widget.h"
#include "component.h"
#include "camera.h"
#include "collider.h"
#include "input.h"


void bring_to_top(ComponentData* components, int entity) {
    // Breadth-first search
    List* queue = List_create();
    List_add(queue, entity);
    while (queue->size > 0) {
        int entity = List_pop(queue);
        List_remove(components->widget.order, entity);
        List_append(components->widget.order, entity);
        CoordinateComponent* coord = CoordinateComponent_get(components, entity);
        for (ListNode* node = coord->children->head; node; node = node->next) {
            int child = node->value;
            List_append(queue, child);
        }
    }
    List_delete(queue);
}


int create_window(ComponentData* components, sfVector2f position, ButtonText text, int width, OnClick on_close) {
    int i = create_entity(components);
    CoordinateComponent_add(components, i, position, 0.0f);
    ColliderComponent_add_rectangle(components, i, width * BUTTON_WIDTH, BUTTON_HEIGHT, GROUP_WALLS)->enabled = false;
    WidgetComponent_add(components, i, text, WIDGET_WINDOW);

    int j = create_button_small(components, "X", vec(0.5 * (width * BUTTON_WIDTH - BUTTON_HEIGHT), 0.0f), on_close);
    add_child(components, i, j);

    return i;
}


int create_label(ComponentData* components, ButtonText text, sfVector2f position) {
    int i = create_entity(components);
    CoordinateComponent_add(components, i, position, 0.0f);
    ColliderComponent_add_rectangle(components, i, BUTTON_WIDTH, BUTTON_HEIGHT, GROUP_WALLS)->enabled = false;
    WidgetComponent_add(components, i, text, WIDGET_LABEL);

    return i;
}


int create_button(ComponentData* components, ButtonText text, sfVector2f position, OnClick on_click) {
    int i = create_entity(components);
    CoordinateComponent_add(components, i, position, 0.0f);
    ColliderComponent_add_rectangle(components, i, BUTTON_WIDTH, BUTTON_HEIGHT, GROUP_WALLS)->enabled = false;
    WidgetComponent_add(components, i, text, WIDGET_BUTTON)->on_click = on_click;

    return i;
}


int create_button_small(ComponentData* components, ButtonText text, sfVector2f position, OnClick on_click) {
    int i = create_entity(components);
    CoordinateComponent_add(components, i, sum(position, vec(0.0f, 0.25f * BORDER_WIDTH)), 0.0f);
    float height = BUTTON_HEIGHT - 0.5f * BORDER_WIDTH;
    ColliderComponent_add_rectangle(components, i, BUTTON_HEIGHT, height, GROUP_WALLS)->enabled = false;
    WidgetComponent_add(components, i, text, WIDGET_BUTTON)->on_click = on_click;

    return i;
}


int create_container(ComponentData* components, sfVector2f position, int width, int height) {
    int i = create_entity(components);
    CoordinateComponent_add(components, i, position, 0.0f);
    ColliderComponent_add_rectangle(components, i, width * BUTTON_WIDTH, height * BUTTON_HEIGHT, GROUP_WALLS)->enabled = false;
    WidgetComponent* widget = WidgetComponent_add(components, i, "", WIDGET_CONTAINER);
    widget->type = WIDGET_CONTAINER;

    return i;
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

    if (button->on_change) {
        button->on_change(components, entity, direction);
    }
}


void scroll_container(ComponentData* components, int entity, int delta) {
    CoordinateComponent* coord = CoordinateComponent_get(components, entity);
    coord = CoordinateComponent_get(components, coord->parent);
    for (ListNode* node = coord->children->head; node; node = node->next) {
        int i = node->value;
        if (WidgetComponent_get(components, i)->type == WIDGET_CONTAINER) {
            increment_value(components, i, delta);
        }
    }
}


void add_widget_to_container(ComponentData* components, int container, int entity) {
    CoordinateComponent* coord = CoordinateComponent_get(components, container);
    WidgetComponent* widget = WidgetComponent_get(components, container);
    ColliderComponent* collider = ColliderComponent_get(components, container);

    int columns = collider->width / BUTTON_WIDTH;
    int rows = coord->children->size / columns;
    sfVector2f pos = vec(0.0f, 0.5f * collider->height - rows * BUTTON_HEIGHT - 0.5f * BUTTON_HEIGHT);
    CoordinateComponent* coord_child = CoordinateComponent_get(components, entity);
    coord_child->position = pos;
    add_child(components, container, entity);

    int n = columns * collider->height / BUTTON_HEIGHT;
    if (coord->children->size > n) {
        widget->max_value += 1;
        WidgetComponent_get(components, entity)->enabled = false;
    }
}


int add_button_to_container(ComponentData* components, int container, ButtonText string, OnClick on_click) {
    int i = create_button(components, string, zeros(), on_click);
    add_widget_to_container(components, container, i);
    return i;
}


void add_row_to_container(ComponentData* components, int container, int left, int right) {
    add_widget_to_container(components, container, left);
    CoordinateComponent* coord_left = CoordinateComponent_get(components, left);
    coord_left->position.x -= 0.5f * BUTTON_WIDTH;

    add_child(components, container, right);
    CoordinateComponent* coord_right = CoordinateComponent_get(components, right);
    sfVector2f pos = sum(coord_left->position, vec(BUTTON_WIDTH, 0.0f));
    float height = ColliderComponent_get(components, container)->height;
    coord_right->position = pos;
    WidgetComponent_get(components, right)->enabled = (pos.y > -0.5f * height && pos.y < 0.5f * height);
}


void add_files_to_container(ComponentData* components, int container, Filename directory, OnClick on_click) {
    Filename path;
    snprintf(path, 128, "%s/%s/*.*", "data", directory);
    
    WIN32_FIND_DATA file;
    HANDLE handle = FindFirstFile(path, &file);

    if (handle == INVALID_HANDLE_VALUE) {
        printf("Path not found: %s\n", path);
        return;
    }

    do {
        if (strcmp(file.cFileName, ".") == 0 || strcmp(file.cFileName, "..") == 0) {
            continue;
        }
        char* dot = strchr(file.cFileName, '.');
        if (dot) {
            *dot = '\0';
        }
        add_button_to_container(components, container, file.cFileName, on_click);
    } while (FindNextFile(handle, &file));
}


void close_dropdown(ComponentData* components, int entity) {
    CoordinateComponent* coord = CoordinateComponent_get(components, entity);
    destroy_entity_recursive(components, coord->children->head->value);
    destroy_entity_recursive(components, coord->children->head->next->value);
    List_clear(coord->children);
}


void set_dropdown(ComponentData* components, int entity) {
    CoordinateComponent* coord = CoordinateComponent_get(components, entity);
    int dropdown = CoordinateComponent_get(components, coord->parent)->parent;
    WidgetComponent* widget = WidgetComponent_get(components, entity);
    WidgetComponent* widget_dropdown = WidgetComponent_get(components, dropdown);
    widget_dropdown->value = widget->value;
    close_dropdown(components, dropdown);
}


void update_scrollbar(ComponentData* components, int entity, int delta) {
    UNUSED(delta);
    CoordinateComponent* coord = CoordinateComponent_get(components, entity);
    coord = CoordinateComponent_get(components, coord->parent);
    int value = WidgetComponent_get(components, entity)->value;
    for (ListNode* node = coord->children->head; node; node = node->next) {
        int i = node->value;
        WidgetComponent* widget = WidgetComponent_get(components, i);
        if (widget->type == WIDGET_SCROLLBAR) {
            widget->value = value;
        }
    }
}


void add_scrollbar_to_container(ComponentData* components, int container) {
    CoordinateComponent* coord = CoordinateComponent_get(components, container);
    WidgetComponent* widget = WidgetComponent_get(components, container);
    ColliderComponent* collider = ColliderComponent_get(components, container);

    widget->on_change = update_scrollbar;

    for (ListNode* node = coord->children->head; node; node = node->next) {
        int i = node->value;
        ColliderComponent_get(components, i)->width = BUTTON_WIDTH - SCROLLBAR_WIDTH;
        CoordinateComponent* coord_child = CoordinateComponent_get(components, i);
        coord_child->position = diff(coord_child->position, vec(0.5f * SCROLLBAR_WIDTH, 0.0f));
    }

    int parent = coord->parent;
    int height = collider->height / BUTTON_HEIGHT;
    float w = ColliderComponent_get(components, parent)->width;
    sfVector2f pos = vec(0.5f * w - 0.5f * SCROLLBAR_WIDTH, -0.5f * (height + 1) * BUTTON_HEIGHT);
    int scrollbar = create_scrollbar(components, pos, height, widget->max_value, scroll_container);
    add_child(components, parent, scrollbar);
}


void toggle_dropdown(ComponentData* components, int entity) {
    CoordinateComponent* coord = CoordinateComponent_get(components, entity);
    WidgetComponent* widget = WidgetComponent_get(components, entity);
    if (coord->children->size > 0) {
        close_dropdown(components, entity);
    } else {
        int height = min(3, widget->max_value);

        int container = create_container(components, vec(0.0f, -2.0f * BUTTON_HEIGHT), 1, height);
        add_child(components, entity, container);
        for (int i = widget->min_value; i <= widget->max_value; i++) {
            int j = add_button_to_container(components, container, "", set_dropdown);
            WidgetComponent* widget_child = WidgetComponent_get(components, j);
            widget_child->value = i;
            widget_child->strings = widget->strings;
        }

        if (widget->max_value > 3) {
            add_scrollbar_to_container(components, container);
        }
    }
}


int create_dropdown(ComponentData* components, sfVector2f position, ButtonText* strings, int size) {
    int i = create_button(components, strings[0], position, toggle_dropdown);
    WidgetComponent* widget = WidgetComponent_get(components, i);
    widget->strings = strings;
    widget->max_value = size - 1;
    widget->type = WIDGET_DROPDOWN;

    return i;
}


void set_slider(ComponentData* components, int entity, sfVector2f mouse_position) {
    WidgetComponent* widget = WidgetComponent_get(components, entity);
    ColliderComponent* collider = ColliderComponent_get(components, entity);
    float x = mouse_position.x - get_position(components, entity).x + 0.5f * collider->width;
    int n = widget->max_value - widget->min_value;
    int value = clamp(x  / collider->width, 0.0f, 1.0f) * n;
    int delta = value - widget->value;
    widget->value = value;
    if (widget->on_change) {
        widget->on_change(components, entity, delta);
    }
}


int create_slider(ComponentData* components, sfVector2f position, int min_value, int max_value, int value, 
        OnChange on_change) {
    int i = create_entity(components);
    CoordinateComponent_add(components, i, position, 0.0f);
    ColliderComponent_add_rectangle(components, i, BUTTON_WIDTH, BUTTON_HEIGHT, GROUP_WALLS)->enabled = false;
    WidgetComponent* widget = WidgetComponent_add(components, i, "", WIDGET_SLIDER);
    widget->min_value = min_value;
    widget->max_value = max_value;
    widget->value = value;
    widget->on_change = on_change;

    return i;
}


void set_scrollbar(ComponentData* components, int entity, sfVector2f mouse_position) {
    WidgetComponent* widget = WidgetComponent_get(components, entity);
    ColliderComponent* collider = ColliderComponent_get(components, entity);
    float y = mouse_position.y - get_position(components, entity).y + 0.5f * collider->height;
    int n = widget->max_value - widget->min_value;
    int value = clamp(1.0f - y  / collider->height, 0.0f, 0.99f) * (n + 1);
    int delta = value - widget->value;
    widget->value = value;
    if (widget->on_change) {
        widget->on_change(components, entity, delta);
    }
}


int create_scrollbar(ComponentData* components, sfVector2f position, int height, int max_value, OnChange on_change) {
    int i = create_entity(components);
    CoordinateComponent_add(components, i, position, 0.0f);
    ColliderComponent* collider = ColliderComponent_add_rectangle(components, i, SCROLLBAR_WIDTH, 
        BUTTON_HEIGHT * height, GROUP_WALLS);
    collider->enabled = false;
    WidgetComponent* widget = WidgetComponent_add(components, i, "", WIDGET_SCROLLBAR);
    widget->max_value = max_value;
    widget->on_change = on_change;

    return i;
}


int create_textbox(ComponentData* components, sfVector2f position, int width) {
    int i = create_entity(components);
    CoordinateComponent_add(components, i, position, 0.0f);
    ColliderComponent* collider = ColliderComponent_add_rectangle(components, i, BUTTON_WIDTH * width, 
        BUTTON_HEIGHT, GROUP_WALLS);
    collider->enabled = false;
    WidgetComponent_add(components, i, "", WIDGET_TEXTBOX);

    return i;
}


void update_widgets(ComponentData* components, sfRenderWindow* window, int camera) {
    int last_selected = -1;
    for (ListNode* node = components->widget.order->head; node; node = node->next) {
        int i = node->value;
        WidgetComponent* widget = WidgetComponent_get(components, i);
        if (!widget->enabled) continue;

        sfVector2f mouse = screen_to_world(components, camera, sfMouse_getPosition((sfWindow*) window));
        widget->selected = false;
        if (point_inside_collider(components, i, mouse)) {
            last_selected = i;
        }
    }
    if (last_selected != -1) {
        WidgetComponent_get(components, last_selected)->selected = true;
    }
}


void draw_button(sfRenderWindow* window, ComponentData* components, int camera, sfVector2f position, float width, 
        float height, bool selected) {
    draw_rectangle(window, components, camera, NULL, position, width, height, 0.0f, COLOR_SHADOW);
    sfVector2f r = sum(position, vec(-0.5f * BORDER_WIDTH, 0.5f * BORDER_WIDTH));
    draw_rectangle(window, components, camera, NULL, r, width - BORDER_WIDTH, height - BORDER_WIDTH, 0.0f, 
        COLOR_HIGHLIGHT);
    sfColor color = selected ? COLOR_HIGHLIGHT : COLOR_BUTTON;
    draw_rectangle(window, components, camera, NULL, position, width - 2.0f * BORDER_WIDTH, 
        height - 2.0f * BORDER_WIDTH, 0.0f, color);
}


void draw_widgets(ComponentData* components, sfRenderWindow* window, int camera) {
    for (ListNode* node = components->widget.order->head; node; node = node->next) {
        int i = node->value;
        WidgetComponent* widget = WidgetComponent_get(components, i);
        if (!widget->enabled) continue;

        sfVector2f pos = sum(get_position(components, camera), get_position(components, i));
        CoordinateComponent* coord = CoordinateComponent_get(components, i);
        ColliderComponent* collider = ColliderComponent_get(components, i);

        float w = collider->width;
        float h = collider->height;
        switch (widget->type) {
            case WIDGET_WINDOW:
                draw_rectangle(window, components, camera, NULL, pos, w + BORDER_WIDTH, h + BORDER_WIDTH, 0.0f, 
                    COLOR_BORDER);
                draw_rectangle(window, components, camera, NULL, pos, w, h, 0.0f, COLOR_SHADOW);
            case WIDGET_CONTAINER:
                draw_rectangle(window, components, camera, NULL, pos, w + BORDER_WIDTH, h + BORDER_WIDTH, 0.0f, 
                    COLOR_BORDER);
                draw_rectangle(window, components, camera, NULL, pos, w, h, 0.0f, COLOR_CONTAINER);
                if (coord->children->size > h / BUTTON_HEIGHT) {
                    sfVector2f r = sum(pos, vec(0.5f * w - 0.5f * BUTTON_HEIGHT, 0.0f));
                    draw_rectangle(window, components, camera, NULL, r, BUTTON_HEIGHT, h, 0.0f, COLOR_CONTAINER);
                }
                break;
            case WIDGET_LABEL:
                break;
            case WIDGET_BUTTON:
                draw_button(window, components, camera, pos, w, h, widget->selected);
                break;
            case WIDGET_DROPDOWN:
                draw_rectangle(window, components, camera, NULL, pos, w, h, 0.0f, COLOR_SHADOW);

                sfVector2f r = sum(pos, vec(-0.1f, 0.1f));
                draw_rectangle(window, components, camera, NULL, r, w - 0.2f, h - 0.2f, 0.0f, COLOR_BUTTON);

                r = sum(pos, vec(0.5f * BUTTON_WIDTH - 0.5f * BUTTON_HEIGHT, 0.0f));
                if (CoordinateComponent_get(components, i)->children->size == 0) {
                    draw_text(window, components, camera, NULL, r, "v", sfWhite);
                } else {
                    draw_text(window, components, camera, NULL, r, "^", sfWhite);
                }
                break;
            case WIDGET_SLIDER:
                draw_rectangle(window, components, camera, NULL, pos, w, h, 0.0f, COLOR_SHADOW);
                float x = widget->value / (float) (widget->max_value - widget->min_value);
                draw_rectangle(window, components, camera, NULL, sum(pos, vec(0.5f * (x - 1.0f) * w, 0.0f)), x * w, h, 
                    0.0f, COLOR_BUTTON);
                char buffer[256];
                sprintf(buffer, "%d", widget->value);
                draw_text(window, components, camera, NULL, pos, buffer, COLOR_TEXT);
                break;
            case WIDGET_SCROLLBAR:
                draw_rectangle(window, components, camera, NULL, pos, w, h, 0.0f, COLOR_CONTAINER);
                float n = widget->max_value - widget->min_value + 1;
                float y = widget->value / n;
                r = sum(pos, vec(0.0f, ((h / BUTTON_HEIGHT - 1.5f) / n - y) * h));
                draw_button(window, components, camera, r, w, h / n, widget->selected);
                break;
            case WIDGET_TEXTBOX:
                draw_rectangle(window, components, camera, NULL, pos, w, h, 0.0f, COLOR_SHADOW);
                draw_text(window, components, camera, widget->text, pos, widget->string, sfWhite);
                break;
            default:
                draw_rectangle(window, components, camera, NULL, pos, w, h, 0.0f, COLOR_SHADOW);
        }

        if (widget->strings) {
            draw_text(window, components, camera, widget->text, pos, widget->strings[widget->value], COLOR_TEXT);
        } else {
            draw_text(window, components, camera, widget->text, pos, widget->string, COLOR_TEXT);
        }
    }
}


bool input_widgets(ComponentData* components, int camera, sfEvent event) {
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

    bool input_detected = false;
    int top_textbox = -1;
    for (int i = 0; i < components->entities; i++) {
        WidgetComponent* widget = WidgetComponent_get(components, i);
        if (!widget) continue;
        if (widget->type == WIDGET_TEXTBOX) {
            top_textbox = i;
        }
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
            } else if (widget->type == WIDGET_SCROLLBAR){
                if (mouse_down) {
                    set_scrollbar(components, i, mouse_position);
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
            int delta = -event.mouseWheelScroll.delta;
            if (widget->type == WIDGET_CONTAINER || widget->type == WIDGET_SPINBOX 
                    || widget->type == WIDGET_SCROLLBAR) {
                increment_value(components, i, delta);
            } else {
                if (coord->parent) {
                    WidgetComponent* parent = WidgetComponent_get(components, coord->parent);
                    if (parent && parent->type == WIDGET_CONTAINER) {
                        increment_value(components, coord->parent, delta);
                    }
                }
            }
        }

        input_detected = true;
    }

    if (top_textbox != -1) {
        WidgetComponent* widget = WidgetComponent_get(components, top_textbox);
        if (event.type == sfEvtKeyPressed) {
            int len = strlen(widget->string);
            if (event.key.code == sfKeyBackspace) {
                if (len > 0) {
                    widget->string[len - 1] = '\0';
                }
            } else {
                if (len < 18) {
                    strcat(widget->string, key_to_letter(event.key.code));
                }
            }
            input_detected = true;
        }
    }

    return input_detected;
}


void destroy_widgets(ComponentData* components) {
    for (int i = 0; i < components->entities; i++) {
        if (WidgetComponent_get(components, i)) {
            destroy_entity(components, i);
        }
    }
}
