#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>

#include <SFML/Audio.h>
#include <SFML/Graphics.h>
#include <SFML/System/Vector2.h>
#include <SFML/Window/Keyboard.h>

#include "sound.h"
#include "game.h"
#include "interface.h"
#include "menu.h"
#include "globals.h"


int main() {
    sfVideoMode mode = { 1920, 1080, 32 };
    sfContext* context = sfContext_create();
    sfContextSettings settings = sfContext_getSettings(context);
    settings.antialiasingLevel = 8;
    sfRenderWindow* window = sfRenderWindow_create(mode, "zword", sfClose, &settings);
    // sfRenderWindow* window = sfRenderWindow_create(mode, "zword", sfFullscreen, &settings);
    sfRenderWindow_setKeyRepeatEnabled(window, sfFalse);
    sfRenderWindow_setMouseCursorVisible(window, false);
    // sfWindow_setVerticalSyncEnabled((sfWindow*) window, true);
    // sfWindow_setFramerateLimit((sfWindow*) window, 60);

    bool focus = true;
    sfJoystick_update();

    sfSound* channels[MAX_SOUNDS];
    for (int i = 0; i < MAX_SOUNDS; i++) {
        channels[i] = sfSound_create();
    }

    FpsCounter* fps = FpsCounter_create();

    sfClock* clock = sfClock_create();
    float time_step = 1.0f / 60.0f;
    float elapsed_time = 0.0f;

    GameData data = create_game(mode);
    start_game(data);

    create_menu(data);

    while (sfRenderWindow_isOpen(window)) {
        float delta_time = sfTime_asSeconds(sfClock_restart(clock));

        sfEvent event;
        while (sfRenderWindow_pollEvent(window, &event)) {
            switch (event.type) {
                case sfEvtLostFocus:
                    focus = false;
                    break;
                case sfEvtGainedFocus:
                    focus = true;
                    sfClock_restart(clock);
                    break;
                case sfEvtClosed:
                    sfRenderWindow_close(window);
                    break;
                case sfEvtKeyPressed:
                    if (event.key.code == sfKeyEscape) {
                        if (game_state == STATE_GAME) {
                            game_state = STATE_PAUSE;
                        } else if (game_state == STATE_PAUSE) {
                            game_state = STATE_GAME;
                        }
                    } else if (event.key.code == sfKeyF5 || event.key.code == sfKeyF6) {
                        if (event.key.code == sfKeyF6) {
                            data.seed = time(NULL);
                        }

                        clear_sounds(channels);
                        sfClock_restart(clock);
                        reset_game(data);
                    }
                    break;
                case sfEvtMouseWheelScrolled:;
                    // CameraComponent* cam = CameraComponent_get(components, camera);
                    // cam->zoom_target = fmaxf(1.0f, cam->zoom_target + event.mouseWheelScroll.delta);
                    break;
                default:
                    break;
            }
        }

        if (focus) {
            while (elapsed_time > time_step) {
                elapsed_time -= time_step;
                switch (game_state) {
                    case STATE_MENU:
                        update_menu(data, window, time_step);
                        break;
                    case STATE_GAME:
                        update_game(data, window, time_step);
                        break;
                    case STATE_PAUSE:
                        break;
                    case STATE_QUIT:
                        sfRenderWindow_close(window);
                        break;
                }
            }

            elapsed_time += delta_time;
        }

        sfRenderWindow_clear(window, sfBlack);

        draw_game(data, window);
        if (game_state == STATE_MENU) {
            draw_menu(data, window);
        }
        draw_fps(window, fps, delta_time);

        sfRenderWindow_display(window);

        play_sounds(data.components, data.camera, data.sounds, channels);
    }

    sfRenderWindow_destroy(window);

    return 0;
}
