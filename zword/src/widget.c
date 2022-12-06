#include "widget.h"
#include "component.h"


float BUTTON_WIDTH = 8.0f;
float BUTTON_HEIGHT = 2.0f;
float BUTTON_MARGIN = 0.2f;


int create_window(ComponentData* components, sfVector2f position, ButtonText text) {
    int i = create_entity(components);
    CoordinateComponent_add(components, i, position, 0.0f);
    ColliderComponent_add_rectangle(components, i, 2 * BUTTON_WIDTH, BUTTON_HEIGHT, GROUP_WALLS)->enabled = false;
    WidgetComponent_add(components, i, text, WIDGET_WINDOW);

    return i;
}


int create_label(ComponentData* components, ButtonText text, sfVector2f position) {
    int i = create_entity(components);
    CoordinateComponent_add(components, i, position, 0.0f);
    ColliderComponent_add_rectangle(components, i, BUTTON_WIDTH, BUTTON_HEIGHT, GROUP_WALLS)->enabled = false;
    WidgetComponent_add(components, i, text, WIDGET_BUTTON);

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


int create_container(ComponentData* components, sfVector2f position, int width, int height) {
    int i = create_entity(components);
    CoordinateComponent_add(components, i, position, 0.0f);
    ColliderComponent_add_rectangle(components, i, width * BUTTON_WIDTH, height * BUTTON_HEIGHT, GROUP_WALLS)->enabled = false;
    WidgetComponent* button = WidgetComponent_add(components, i, "", WIDGET_CONTAINER);
    button->type = WIDGET_CONTAINER;

    return i;
}


void add_widget_to_container(ComponentData* components, int container, int entity) {
    CoordinateComponent* coord = CoordinateComponent_get(components, container);
    WidgetComponent* widget = WidgetComponent_get(components, container);
    ColliderComponent* collider = ColliderComponent_get(components, container);
    float height = collider->height;

    int columns = collider->width / BUTTON_WIDTH;
    int rows = coord->children->size / columns;
    sfVector2f pos = vec(0.0f, 0.5f * height - rows * BUTTON_HEIGHT - 0.5f * BUTTON_HEIGHT);
    CoordinateComponent_get(components, entity)->position = pos;
    add_child(components, container, entity);

    if (coord->children->size * BUTTON_HEIGHT > height) {
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

    CoordinateComponent* coord = CoordinateComponent_get(components, container);
    float height = ColliderComponent_get(components, container)->height;

    CoordinateComponent_get(components, right)->position = sum(coord_left->position, vec(BUTTON_WIDTH, 0.0f));
    add_child(components, container, right);

    if (coord->children->size * BUTTON_HEIGHT > height) {
        WidgetComponent_get(components, right)->enabled = false;
    }
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
        int container = create_container(components, vec(0.0f, -2.0f * BUTTON_HEIGHT), 1, height);
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
    WidgetComponent* widget = WidgetComponent_get(components, i);
    widget->strings = strings;
    widget->max_value = max_value;
    widget->type = WIDGET_DROPDOWN;

    return i;
}


void set_slider(ComponentData* components, int entity, sfVector2f mouse_position) {
    WidgetComponent* widget = WidgetComponent_get(components, entity);
    ColliderComponent* collider = ColliderComponent_get(components, entity);
    float x = mouse_position.x - get_position(components, entity).x + 0.5f * collider->width;
    float value = clamp(x  / collider->width, 0.0f, 1.0f) * (widget->max_value - widget->min_value);
    widget->value = (int) value;
}


int create_slider(ComponentData* components, sfVector2f position, int min_value, int max_value) {
    int i = create_entity(components);
    CoordinateComponent_add(components, i, position, 0.0f);
    ColliderComponent_add_rectangle(components, i, BUTTON_WIDTH, BUTTON_HEIGHT, GROUP_WALLS);
    WidgetComponent* widget = WidgetComponent_add(components, i, "", WIDGET_SLIDER);
    widget->min_value = min_value;
    widget->max_value = max_value;

    return i;
}
