#include <stdio.h>

#include "game.h"
#include "menu.h"
#include "component.h"
#include "collider.h"
#include "globals.h"


void play() {
    game_state = STATE_GAME;
}


void quit() {
    game_state = STATE_QUIT;
}


int create_button(ComponentData* components, int camera, ButtonText text, sfVector2f position, void (*on_click)()) {
    int i = create_entity(components);
    CoordinateComponent_add(components, i, position, 0.0f)->parent = camera;
    ColliderComponent_add_rectangle(components, i, 5.0f, 2.0f, GROUP_WALLS);
    ButtonComponent* button = ButtonComponent_add(components, i, text);
    button->on_click = on_click;

    return i;
}


void create_menu(GameData data) {
    create_button(data.components, data.camera, "PLAY", (sfVector2f) { 0.0f, 5.0f }, play);
    create_button(data.components, data.camera, "OPTIONS", (sfVector2f) { 0.0f, 0.0f }, NULL);
    create_button(data.components, data.camera, "QUIT", (sfVector2f) { 0.0f, -5.0f }, quit);
}


void update_menu(GameData data, sfRenderWindow* window, float time_step) {
    update_buttons(data.components, window, data.camera);
}


void draw_menu(GameData data, sfRenderWindow* window) {
    draw_buttons(data.components, window, data.camera);

    sfVector2f pos = screen_to_world(data.components, data.camera, sfMouse_getPosition((sfWindow*) window));
    draw_circle(window, data.components, data.camera, NULL, pos, 0.1f, sfWhite);
}


void update_buttons(ComponentData* components, sfRenderWindow* window, int camera) {
    for (int i = 0; i < components->entities; i++) {
        ButtonComponent* button = ButtonComponent_get(components, i);
        if (!button) continue;

        sfVector2f mouse = screen_to_world(components, camera, sfMouse_getPosition((sfWindow*) window));

        button->selected = inside_collider(components, i, mouse);

        if (button->selected && sfMouse_isButtonPressed(sfMouseLeft)) {
            if (button->on_click) {
                button->on_click();
            }
        }
    }
}


void draw_buttons(ComponentData* components, sfRenderWindow* window, int camera) {
    for (int i = 0; i < components->entities; i++) {
        ButtonComponent* button = ButtonComponent_get(components, i);
        if (!button) continue;

        sfVector2f pos = get_position(components, i);

        sfVector2f corners[4];
        get_corners(components, i, corners);

        draw_text(window, components, camera, NULL, pos, button->text, sfWhite);
        if (button->selected) {
            draw_line(window, components, camera, NULL, corners[1], corners[2], 0.1f, sfWhite);
        }
    }
}
