#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "game.h"
#include "menu.h"
#include "component.h"
#include "collider.h"
#include "globals.h"
#include "settings.h"


static const char* RESOLUTIONS[] = {"1280x720", "1920x1080", "2560x1440"};
static ButtonComponent* BUTTON_RESOLUTION = NULL;


void play() {
    game_state = STATE_START;
}


void settings() {
    game_state = STATE_SETTINGS;
}


void quit() {
    game_state = STATE_QUIT;
}


void back() {
    game_state = STATE_MENU;
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


void change_resolution(int dir) {
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


void resolution_down() {
    change_resolution(-1);
}


void resolution_up() {
    change_resolution(+1);
}


int create_button(ComponentData* components, ButtonText text, sfVector2f position, ButtonMenu menu, OnClick on_click) {
    int i = create_entity(components);
    CoordinateComponent_add(components, i, position, 0.0f);
    ColliderComponent_add_rectangle(components, i, 5.0f, 2.0f, GROUP_WALLS)->enabled = false;
    ButtonComponent_add(components, i, text, menu, on_click);

    return i;
}


void create_menu(GameData data) {
    create_button(data.components, "PLAY", (sfVector2f) { 0.0f, 5.0f }, MENU_MAIN, play);
    create_button(data.components, "SETTINGS", (sfVector2f) { 0.0f, 0.0f }, MENU_MAIN, settings);
    create_button(data.components, "QUIT", (sfVector2f) { 0.0f, -5.0f }, MENU_MAIN, quit);

    create_button(data.components, "RESOLUTION", (sfVector2f) { 0.0f, 5.0f }, MENU_SETTINGS, NULL);
    ButtonText buffer;
    sprintf(buffer, "%ix%i", game_settings.width, game_settings.height);
    int i = create_button(data.components, buffer, (sfVector2f) { 0.0f, 4.0f }, MENU_SETTINGS, NULL);
    BUTTON_RESOLUTION = ButtonComponent_get(data.components, i);
    create_button(data.components, "<", (sfVector2f) { -4.0f, 4.0f }, MENU_SETTINGS, resolution_down);
    create_button(data.components, ">", (sfVector2f) { 4.0f, 4.0f }, MENU_SETTINGS, resolution_up);

    create_button(data.components, "VOLUME", (sfVector2f) { 0.0f, 0.0f }, MENU_SETTINGS, NULL);
    create_button(data.components, "APPLY", (sfVector2f) { 5.0f, -5.0f }, MENU_SETTINGS, apply);
    create_button(data.components, "BACK", (sfVector2f) { -5.0f, -5.0f }, MENU_SETTINGS, back);
}


void update_menu(GameData data, sfRenderWindow* window, ButtonMenu menu) {
    update_buttons(data.components, window, data.camera, menu);
}


void input_menu(ComponentData* components, sfEvent event) {
    if (event.type == sfEvtMouseButtonReleased && event.mouseButton.button == sfMouseLeft) {
        for (int i = 0; i < components->entities; i++) {
            ButtonComponent* button = ButtonComponent_get(components, i);
            if (!button) continue;

            if (button->selected) {
                if (button->on_click) {
                    button->on_click();
                    // button->selected = false;
                }
            }
        }
    }
}


void draw_menu(GameData data, sfRenderWindow* window, ButtonMenu menu) {
    draw_buttons(data.components, window, data.camera, menu);

    sfVector2f pos = screen_to_world(data.components, data.camera, sfMouse_getPosition((sfWindow*) window));
    draw_circle(window, data.components, data.camera, NULL, pos, 0.1f, sfWhite);
}


void update_buttons(ComponentData* components, sfRenderWindow* window, int camera, ButtonMenu menu) {
    for (int i = 0; i < components->entities; i++) {
        ButtonComponent* button = ButtonComponent_get(components, i);
        if (!button) continue;

        if (button->menu == menu) {
            sfVector2f mouse = screen_to_world(components, camera, sfMouse_getPosition((sfWindow*) window));

            button->selected = inside_collider(components, i, mouse);
        } else {
            button->selected = false;
        }
    }
}


void draw_buttons(ComponentData* components, sfRenderWindow* window, int camera, ButtonMenu menu) {
    for (int i = 0; i < components->entities; i++) {
        ButtonComponent* button = ButtonComponent_get(components, i);
        if (!button) continue;
        if (button->menu != menu) continue;

        sfVector2f pos = get_position(components, i);

        sfVector2f corners[4];
        get_corners(components, i, corners);

        draw_text(window, components, camera, button->text, pos, button->string, sfWhite);
        if (button->selected && button->on_click) {
            draw_line(window, components, camera, NULL, corners[1], corners[2], 0.1f, sfWhite);
        }
    }
}
