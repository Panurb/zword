#include <math.h>
#include <ctype.h>
#include <string.h>
#include <stdio.h>

#include "input.h"
#include "util.h"
#include "component.h"
#include "camera.h"
#include "vehicle.h"
#include "item.h"
#include "settings.h"


char* KEY_NAMES[] = {"A", "B", "C", "D", "E", "F", "G", "H", "I", "J", "K", "L", "M", "N", "O", "P", "Q", "R", "S", "T", 
    "U", "V", "W", "X", "Y", "Z", "Num0", "Num1", "Num2", "Num3", "Num4", "Num5", "Num6", "Num7", "Num8", "Num9", 
    "Escape", "LControl", "LShift", "LAlt", "LSystem", "RControl", "RShift", "RAlt", "RSystem", "Menu", "LBracket", 
    "RBracket", "Semicolon", "Comma", "Period", "Quote", "Slash", "Backslash", "Tilde", "Equal", "Hyphen", "Space", 
    "Enter", "Backspace", "Tab", "PageUp", "PageDown", "End", "Home", "Insert", "Delete", "Add", "Subtract", "Multiply", 
    "Divide", "Left", "Right", "Up", "Down", "Numpad0", "Numpad1", "Numpad2", "Numpad3", "Numpad4", "Numpad5", 
    "Numpad6", "Numpad7", "Numpad8", "Numpad9", "F1", "F2", "F3", "F4", "F5", "F6", "F7", "F8", "F9", "F10", "F11", 
    "F12", "F13", "F14", "F15", "Pause"};


char* MOUSE_NAMES[] = {"Mouse left", "Mouse middle", "Mouse right", "Mouse extra 1", "Mouse extra 2"};


char* LETTERS[] = {"a", "b", "c", "d", "e", "f", "g", "h", "i", "j", "k", "l", "m", "n", "o", "p", "q", "r", "s", "t", 
    "u", "v", "w", "x", "y", "z"};


char* BUTTON_NAMES[] = {
    "A",
    "B",
    "X",
    "Y",
    "LB",
    "RB",
    "START",
    "BACK",
    "LT",
    "RT",
    "L",
    "R"
};


char* ACTIONS[] = {
    "MOVE_UP",
    "MOVE_LEFT",
    "MOVE_DOWN",
    "MOVE_RIGHT",
    "ATTACK",
    "ENTER",
    "PICK_UP",
    "RELOAD",
    "ATTACHMENT",
    "INVENTORY",
    "AMMO"
};

int ACTIONS_SIZE = LENGTH(ACTIONS);


char* ACTION_BUTTONS_XBOX[] = {
    "",
    "",
    "",
    "",
    "RT",
    "A",
    "RB",
    "X",
    "Y",
    "LT",
    "LB"
};


const char* key_to_string(SDL_Scancode key) {
    return SDL_GetScancodeName(key);
}

char* mouse_to_string(int button) {
    return (1 <= button && button < LENGTH(MOUSE_NAMES)) ? MOUSE_NAMES[button - 1] : "unknown";
}


char* keybind_to_string(Keybind keybind) {
    if (keybind.device == DEVICE_KEYBOARD) {
        return key_to_string(keybind.key);
    } else if (keybind.device == DEVICE_MOUSE) {
        return mouse_to_string(keybind.key);
    } else {
        return "unknown";
    }
}


Keybind string_to_keybind(String string) {
    Keybind keybind = {DEVICE_UNBOUND, 0};

    for (int i = 0; i < LENGTH(MOUSE_NAMES); i++) {
        if (strcmp(string, MOUSE_NAMES[i]) == 0) {
            keybind.device = DEVICE_MOUSE;
            keybind.key = i + 1;
            return keybind;
        }
    }

    SDL_Scancode scancode = SDL_GetScancodeFromName(string);

    if (scancode != SDL_SCANCODE_UNKNOWN) {
        keybind.device = DEVICE_KEYBOARD;
        keybind.key = scancode;
        return keybind;
    }

    return keybind;
}


char* action_to_keybind(char* action) {
    for (int i = 0; i < LENGTH(ACTIONS); i++) {
        if (strcmp(action, ACTIONS[i]) == 0) {
            return keybind_to_string(game_settings.keybinds[i]);
        }
    }

    if (strcmp(action, "AIM") == 0) {
        return "Mouse";
    }

    return action;
}


bool keybind_pressed(PlayerAction i) {
    Keybind keybind = game_settings.keybinds[i];
    if (keybind.device == DEVICE_KEYBOARD) {
        return sfKeyboard_isKeyPressed(keybind.key);
    } else if (keybind.device == DEVICE_MOUSE) {
        return sfMouse_isButtonPressed(keybind.key);
    } else {
        return false;
    }
}


bool sdl_keybind_pressed(PlayerAction i) {
    Keybind keybind = game_settings.keybinds[i];
    if (keybind.device == DEVICE_KEYBOARD) {
        return SDL_GetKeyboardState(NULL)[keybind.key];
    } else if (keybind.device == DEVICE_MOUSE) {
        return SDL_GetMouseState(NULL, NULL) & SDL_BUTTON(keybind.key);
    } else {
        return false;
    }
}


void replace_actions(String output, String input) {
    output[0] = '\0';
    char* start = strchr(input, '[');
    char* end = input;
    while (start) {
        strncat(output, end, start - end + 1);
        start++;

        end = strchr(start, ']');
        if (end == NULL) {
            strcat(output, start);
            return;
        }
        
        *end = '\0';
        strcat(output, action_to_keybind(start));
        *end = ']';

        start = end + 1;
        start = strchr(start, '[');
    }

    strcat(output, end);
}


Vector2f get_mouse_position(int camera) {
    int x;
    int y;
    SDL_GetMouseState(&x, &y);
    return screen_to_world(camera, (Vector2f) {x, y});
}


void update_controller(int camera, int i) {
    PlayerComponent* player = PlayerComponent_get(i);
    int joystick = player->controller.joystick;

    Vector2f left_stick = zeros();
    Vector2f right_stick = zeros();
    if (player->controller.joystick == -1) {
        if (sdl_keybind_pressed(ACTION_LEFT)) {
            left_stick.x -= 1.0f;
        }
        if (sdl_keybind_pressed(ACTION_RIGHT)) {
            left_stick.x += 1.0f;
        }
        if (sdl_keybind_pressed(ACTION_DOWN)) {
            left_stick.y -= 1.0f;
        }
        if (sdl_keybind_pressed(ACTION_UP)) {
            left_stick.y += 1.0f;
        }
        player->controller.left_stick = normalized(left_stick);

        Vector2f mouse = get_mouse_position(camera);
        right_stick = diff(mouse, get_position(i));
        player->controller.right_stick = normalized(right_stick);

        player->controller.left_trigger = sdl_keybind_pressed(ACTION_ATTACK) ? 1.0f : 0.0f;
        player->controller.right_trigger = sdl_keybind_pressed(ACTION_PICKUP) ? 1.0f : 0.0f;

        for (ControllerButton b = BUTTON_A; b <= BUTTON_R; b++) {
            bool down = false;
            switch (b) {
                case BUTTON_A:
                    down = sdl_keybind_pressed(ACTION_ENTER);
                    break;
                case BUTTON_B:
                    break;
                case BUTTON_X:
                    down = sdl_keybind_pressed(ACTION_RELOAD);
                    break;
                case BUTTON_Y:
                    down = sdl_keybind_pressed(ACTION_ATTACHMENT);
                    break;
                case BUTTON_LB:
                    down = sdl_keybind_pressed(ACTION_AMMO);
                    break;
                case BUTTON_RB:
                    down = sdl_keybind_pressed(ACTION_PICKUP);
                    break;
                case BUTTON_START:
                    break;
                case BUTTON_BACK:
                    break;
                case BUTTON_LT:
                    down = sdl_keybind_pressed(ACTION_INVENTORY);
                    break;
                case BUTTON_RT:
                    down = sdl_keybind_pressed(ACTION_ATTACK);
                    break;
                case BUTTON_L:
                    break;
                case BUTTON_R:
                    break;
            }

            player->controller.buttons_pressed[b] = (down && !player->controller.buttons_down[b]);
            player->controller.buttons_released[b] = (!down && player->controller.buttons_down[b]);
            player->controller.buttons_down[b] = down;
        }
    } else {
        SDL_GameController* controller = app.controllers[joystick];

        left_stick.x = map_to_range(SDL_GameControllerGetAxis(controller, SDL_CONTROLLER_AXIS_LEFTX), 
            SDL_JOYSTICK_AXIS_MIN, SDL_JOYSTICK_AXIS_MAX, -1.0f, 1.0f);
        left_stick.y = -map_to_range(SDL_GameControllerGetAxis(controller, SDL_CONTROLLER_AXIS_LEFTY), 
            SDL_JOYSTICK_AXIS_MIN, SDL_JOYSTICK_AXIS_MAX, -1.0f, 1.0f);
        if (fabsf(left_stick.x) < 0.05f) {
            left_stick.x = 0.0f;
        }
        if (fabsf(left_stick.y) < 0.05f) {
            left_stick.y = 0.0f;
        }
        player->controller.left_stick = left_stick;

        right_stick.x = map_to_range(SDL_GameControllerGetAxis(controller, SDL_CONTROLLER_AXIS_RIGHTX), 
            SDL_JOYSTICK_AXIS_MIN, SDL_JOYSTICK_AXIS_MAX, -1.0f, 1.0f);
        right_stick.y = -map_to_range(SDL_GameControllerGetAxis(controller, SDL_CONTROLLER_AXIS_RIGHTY),
            SDL_JOYSTICK_AXIS_MIN, SDL_JOYSTICK_AXIS_MAX, -1.0f, 1.0f);
        if (norm(right_stick) < 0.25f) {
            right_stick = zeros();
        }
        if (norm(right_stick) > 1.0f) {
            right_stick = normalized(right_stick);
        }
        player->controller.right_stick = right_stick;

        float trigger = 0.01f * SDL_GameControllerGetAxis(controller, SDL_CONTROLLER_AXIS_TRIGGERLEFT);
        if (fabsf(trigger) < 0.1f) {
            trigger = 0.0f;
        }
        player->controller.left_trigger = trigger;

        trigger = 0.01f * SDL_GameControllerGetAxis(controller, SDL_CONTROLLER_AXIS_TRIGGERRIGHT);
        if (fabsf(trigger) < 0.1f) {
            trigger = 0.0f;
        }
        player->controller.right_trigger = trigger;

        // player->controller.dpad.x = 0.01f * SDL_GameControllerGetAxis(controller, SDL_CONTROLLER_BUTTON_DPAD_RIGHT);
        // player->controller.dpad.y = 0.01f * SDL_GameControllerGetAxis(controller, SDL_CONTROLLER_BUTTON_DPAD_UP);

        for (int b = BUTTON_A; b <= BUTTON_R; b++) {
            SDL_GameControllerButton button = player->controller.buttons[b];
            bool down = SDL_GameControllerGetButton(controller, player->controller.buttons[b]);
            if (b == BUTTON_LT) {
                down = (player->controller.left_trigger > 0.5f);
            } else if (b == BUTTON_RT) {
                down = (player->controller.right_trigger > 0.5f);
            }

            player->controller.buttons_pressed[b] = (down && !player->controller.buttons_down[b]);
            player->controller.buttons_released[b] = (!down && player->controller.buttons_down[b]);
            player->controller.buttons_down[b] = down;
        }
    }
}


void input_players(int camera) {
    for (int i = 0; i < game_data->components->entities; i++) {
        PlayerComponent* player = PlayerComponent_get(i);
        if (!player) continue;

        update_controller(camera, i);
        Controller controller = player->controller;

        WeaponComponent* weapon = WeaponComponent_get(player->inventory[player->item]);

        switch (player->state) {
            case PLAYER_ON_FOOT:
                if (controller.buttons_down[BUTTON_LT]) {
                    player->state = PLAYER_MENU;
                }

                if (controller.buttons_pressed[BUTTON_RB]) {
                    player->state = PLAYER_PICK_UP;
                }

                if (controller.buttons_pressed[BUTTON_RT]) {
                    player->state = PLAYER_SHOOT;
                }

                if (weapon) {
                    if (controller.buttons_pressed[BUTTON_X]) {
                        player->state = PLAYER_RELOAD;
                    }

                    if (controller.buttons_pressed[BUTTON_Y]) {
                        ItemComponent* item = ItemComponent_get(player->inventory[player->item]);
                        for (int j = 0; j < item->size; j++) {
                            int k = item->attachments[j];
                            LightComponent* light = LightComponent_get(k);
                            if (light) {
                                light->enabled = !light->enabled;
                            }
                        }
                    }
                }

                if (controller.buttons_down[BUTTON_LB]) {
                    player->state = PLAYER_AMMO_MENU;
                }

                if (controller.buttons_pressed[BUTTON_A]) {
                    player->state = PLAYER_ENTER;
                }

                break;
            case PLAYER_PICK_UP:
                break;
            case PLAYER_SHOOT:
                if (!controller.buttons_down[BUTTON_RT]) {
                    player->state = PLAYER_ON_FOOT;
                }

                break;
            case PLAYER_RELOAD:
                if (controller.buttons_down[BUTTON_LT]) {
                    player->state = PLAYER_MENU;
                }

                break;
            case PLAYER_ENTER:
                break;
            case PLAYER_DRIVE:
                if (controller.buttons_pressed[BUTTON_A]) {
                    exit_vehicle(i);
                    player->state = PLAYER_ON_FOOT;
                }

                break;
            case PLAYER_PASSENGER:
                if (controller.buttons_down[BUTTON_LT]) {
                    player->state = PLAYER_MENU;
                }

                if (controller.buttons_pressed[BUTTON_RT]) {
                    player->state = PLAYER_SHOOT;
                }

                if (controller.buttons_pressed[BUTTON_A]) {
                    exit_vehicle(i);
                    player->state = PLAYER_ON_FOOT;
                }

                break;
            case PLAYER_MENU:
                if (controller.buttons_pressed[BUTTON_RT]) {
                    player->state = PLAYER_MENU_GRAB;
                }

                if (!controller.buttons_down[BUTTON_LT]) {
                    player->state = PLAYER_ON_FOOT;
                }

                if (controller.buttons_pressed[BUTTON_RB]) {
                    drop_item(i);
                }

                break;
            case PLAYER_MENU_GRAB:
                if (controller.buttons_released[BUTTON_RT]) {
                    player->state = PLAYER_MENU_DROP;
                }

                break;
            case PLAYER_MENU_DROP:
                if (controller.buttons_down[BUTTON_LT]) {
                    player->state = PLAYER_MENU;
                } else {
                    player->state = PLAYER_ON_FOOT;
                }

                break;
            case PLAYER_AMMO_MENU:
                if (!controller.buttons_down[BUTTON_LB]) {
                    player->state = PLAYER_ON_FOOT;
                }

                break;
            case PLAYER_DEAD:
                break;
        }
    }
}
