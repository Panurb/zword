#include "widget.h"
#include "component.h"


float BUTTON_WIDTH = 5.0f;
float BUTTON_HEIGHT = 2.0f;


int create_window(ComponentData* components, sfVector2f position) {
    int i = create_entity(components);
    CoordinateComponent_add(components, i, position, 0.0f);
    ColliderComponent_add_rectangle(components, i, BUTTON_WIDTH, BUTTON_HEIGHT, GROUP_WALLS)->enabled = false;
    WidgetComponent_add(components, i, "window", WIDGET_WINDOW);

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
    CoordinateComponent_add(components, i, position, 0.0f);
    ColliderComponent_add_rectangle(components, i, BUTTON_HEIGHT, BUTTON_HEIGHT, GROUP_WALLS)->enabled = false;
    WidgetComponent_add(components, i, text, WIDGET_BUTTON)->on_click = on_click;

    return i;
}


int create_container(ComponentData* components, sfVector2f position, int height) {
    int i = create_entity(components);
    CoordinateComponent_add(components, i, position, 0.0f);
    ColliderComponent_add_rectangle(components, i, BUTTON_WIDTH, height * BUTTON_HEIGHT, GROUP_WALLS)->enabled = false;
    WidgetComponent* button = WidgetComponent_add(components, i, "", WIDGET_CONTAINER);
    button->type = WIDGET_CONTAINER;

    return i;
}


int add_button_to_container(ComponentData* components, int container, ButtonText string, OnClick on_click) {
    CoordinateComponent* coord = CoordinateComponent_get(components, container);
    WidgetComponent* button = WidgetComponent_get(components, container);
    float height = ColliderComponent_get(components, container)->height;

    sfVector2f pos = vec(0.0f, 0.5f * height - coord->children->size * BUTTON_HEIGHT - 0.5f * BUTTON_HEIGHT);
    int i = create_button(components, string, pos, on_click);
    add_child(components, container, i);

    if (coord->children->size * BUTTON_HEIGHT > height) {
        button->max_value += 1;
        WidgetComponent_get(components, i)->enabled = false;
    }

    return i;
}


void close_dropdown(ComponentData* components, int entity) {
    CoordinateComponent* coord = CoordinateComponent_get(components, entity);
    destroy_entity_recursive(components, coord->children->head->value);
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


void toggle_dropdown(ComponentData* components, int entity) {
    CoordinateComponent* coord = CoordinateComponent_get(components, entity);
    WidgetComponent* widget = WidgetComponent_get(components, entity);
    if (coord->children->size > 0) {
        close_dropdown(components, entity);
    } else {
        int height = min(3, widget->max_value);
        int container = create_container(components, vec(0.0f, -2.0f * BUTTON_HEIGHT), height);
        add_child(components, entity, container);
        for (int i = widget->min_value; i <= widget->max_value; i++) {
            int j = add_button_to_container(components, container, "", set_dropdown);
            WidgetComponent* widget_child = WidgetComponent_get(components, j);
            widget_child->value = i;
            widget_child->strings = widget->strings;
        }
    }
}


int create_dropdown(ComponentData* components, sfVector2f position, ButtonText* strings, int max_value) {
    int i = create_button(components, strings[0], position, toggle_dropdown);
    WidgetComponent* button = WidgetComponent_get(components, i);
    button->strings = strings;
    button->max_value = max_value;
    button->type = WIDGET_BUTTON;

    return i;
}
