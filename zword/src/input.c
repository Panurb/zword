#include <math.h>

#include <SFML/Window/Keyboard.h>

#include "input.h"
#include "util.h"
#include "component.h"
#include "camera.h"


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
