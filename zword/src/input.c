#include <math.h>

#include <SFML/Window/Keyboard.h>

#include "input.h"
#include "util.h"
#include "component.h"
#include "camera.h"
#include "vehicle.h"
#include "item.h"


void update_controller(ComponentData* components, sfRenderWindow* window, int camera, int i) {
    PlayerComponent* player = PlayerComponent_get(components, i);
    int joystick = player->controller.joystick;

    sfVector2f left_stick = zeros();
    sfVector2f right_stick = zeros();
    if (player->controller.joystick == -1) {
        if (sfKeyboard_isKeyPressed(sfKeyA)) {
            left_stick.x -= 1.0f;
        }
        if (sfKeyboard_isKeyPressed(sfKeyD)) {
            left_stick.x += 1.0f;
        }
        if (sfKeyboard_isKeyPressed(sfKeyW)) {
            left_stick.y += 1.0f;
        }
        if (sfKeyboard_isKeyPressed(sfKeyS)) {
            left_stick.y -= 1.0f;
        }
        player->controller.left_stick = normalized(left_stick);

        sfVector2f mouse = screen_to_world(components, camera, sfMouse_getPosition((sfWindow*) window));
        right_stick = diff(mouse, get_position(components, i));
        player->controller.right_stick = normalized(right_stick);

        player->controller.left_trigger = sfKeyboard_isKeyPressed(player->controller.buttons[BUTTON_LT]) ? 1.0f : 0.0f;
        player->controller.right_trigger = sfMouse_isButtonPressed(sfMouseLeft) ? 1.0f : 0.0f;

        for (int b = BUTTON_A; b <= BUTTON_RT; b++) {
            bool down = sfKeyboard_isKeyPressed(player->controller.buttons[b]);
            if (b == BUTTON_LT) {
                down = (player->controller.left_trigger > 0.5f);
            } else if (b == BUTTON_RT) {
                down = (player->controller.right_trigger > 0.5f);
            } else if (b == BUTTON_RB) {
                down = sfMouse_isButtonPressed(sfMouseRight);
            }

            player->controller.buttons_pressed[b] = (down && !player->controller.buttons_down[b]);
            player->controller.buttons_released[b] = (!down && player->controller.buttons_down[b]);
            player->controller.buttons_down[b] = down;
        }
    } else {
        int* axes = player->controller.axes;

        left_stick.x = 0.01f * sfJoystick_getAxisPosition(joystick, axes[sfJoystickX]);
        left_stick.y = -0.01f * sfJoystick_getAxisPosition(joystick, axes[sfJoystickY]);
        if (fabsf(left_stick.x) < 0.05f) {
            left_stick.x = 0.0f;
        }
        if (fabsf(left_stick.y) < 0.05f) {
            left_stick.y = 0.0f;
        }
        player->controller.left_stick = left_stick;

        right_stick.x = 0.01f * sfJoystick_getAxisPosition(joystick, axes[sfJoystickZ]);
        right_stick.y = -0.01f * sfJoystick_getAxisPosition(joystick, axes[sfJoystickR]);
        if (norm(right_stick) < 0.25f) {
            right_stick = zeros();
        }
        if (norm(right_stick) > 1.0f) {
            right_stick = normalized(right_stick);
        }
        player->controller.right_stick = right_stick;

        if (axes[sfJoystickU] == axes[sfJoystickV]) {
            float trigger = 0.01f * sfJoystick_getAxisPosition(joystick, axes[sfJoystickU]);
            if (fabsf(trigger) < 0.1f) {
                trigger = 0.0f;
            }

            if (trigger > 0) {
                player->controller.left_trigger = trigger;
            } else if (trigger < 0) {
                player->controller.right_trigger = -trigger;
            } else {
                player->controller.left_trigger = 0.0f;
                player->controller.right_trigger = 0.0f;
            }
        } else {
            player->controller.left_trigger = 0.005f * sfJoystick_getAxisPosition(joystick, axes[sfJoystickU]) + 0.5f;
            player->controller.right_trigger = 0.005f * sfJoystick_getAxisPosition(joystick, axes[sfJoystickV]) + 0.5f;
        }

        player->controller.dpad.x = 0.01f * sfJoystick_getAxisPosition(joystick, axes[sfJoystickPovX]);
        player->controller.dpad.y = 0.01f * sfJoystick_getAxisPosition(joystick, axes[sfJoystickPovY]);

        for (int b = BUTTON_A; b <= BUTTON_RT; b++) {
            bool down = sfJoystick_isButtonPressed(joystick, player->controller.buttons[b]);
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


void input(ComponentData* components, sfRenderWindow* window, int camera) {
    sfJoystick_update();

    for (int i = 0; i < components->entities; i++) {
        PlayerComponent* player = PlayerComponent_get(components, i);
        if (!player) continue;

        update_controller(components, window, camera, i);
        Controller controller = player->controller;

        WeaponComponent* weapon = WeaponComponent_get(components, player->inventory[player->item]);

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
                        ItemComponent* item = ItemComponent_get(components, player->inventory[player->item]);
                        for (int j = 0; j < item->size; j++) {
                            int k = item->attachments[j];
                            LightComponent* light = LightComponent_get(components, k);
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
                    enter_vehicle(components, i);
                }

                VehicleComponent* vehicle = VehicleComponent_get(components, player->vehicle);
                if (vehicle) {
                    if (vehicle->riders[0] == i) {
                        player->state = PLAYER_DRIVE;
                    } else {
                        player->state = PLAYER_PASSENGER;
                    }
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
            case PLAYER_DRIVE:
                if (controller.buttons_pressed[BUTTON_A]) {
                    exit_vehicle(components, i);
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
                    exit_vehicle(components, i);
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
                    drop_item(components, i);
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
