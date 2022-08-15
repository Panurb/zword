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
#include "settings.h"


sfRenderWindow* create_game_window(sfVideoMode* mode) {
    mode->width = game_settings.width;
    mode->height = game_settings.height;
    sfContext* context = sfContext_create();
    sfContextSettings settings = sfContext_getSettings(context);
    sfContext_destroy(context);
    settings.antialiasingLevel = game_settings.antialiasing;
    sfUint32 style = game_settings.fullscreen ? sfFullscreen : sfClose;
    sfRenderWindow* window = sfRenderWindow_create(*mode, "zword", style, &settings);
    sfRenderWindow_setKeyRepeatEnabled(window, false);
    sfRenderWindow_setMouseCursorVisible(window, false);
    sfWindow_setVerticalSyncEnabled((sfWindow*) window, game_settings.vsync);
    if (game_settings.max_fps) {
        sfWindow_setFramerateLimit((sfWindow*) window, 60);
    }
    return window;
}


int main() {
    load_settings();

    sfVideoMode mode = { game_settings.width, game_settings.height, 32 };
    sfRenderWindow* window = create_game_window(&mode);

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
                    input_menu(data.components, event);
                    break;
            }
        }

        if (focus) {
            while (elapsed_time > time_step) {
                elapsed_time -= time_step;
                switch (game_state) {
                    case STATE_MENU:
                        update_menu(data, window, MENU_MAIN);
                        break;
                    case STATE_START:
                        start_game(data);
                        game_state = STATE_GAME;
                        break;
                    case STATE_GAME:
                        update_game(data, window, time_step);
                        break;
                    case STATE_PAUSE:
                        update_menu(data, window, MENU_PAUSE);
                        break;
                    case STATE_SETTINGS:
                        update_menu(data, window, MENU_SETTINGS);
                        break;
                    case STATE_APPLY:
                        sfRenderWindow_destroy(window);
                        window = create_game_window(&mode);
                        resize_game(&data, mode);
                        game_state = STATE_SETTINGS;
                        break;
                    case STATE_QUIT:
                        sfRenderWindow_close(window);
                        break;
                }
            }

            elapsed_time += delta_time;
        }

        sfRenderWindow_clear(window, sfBlack);

        switch (game_state) {
            case STATE_MENU:
                draw_menu(data, window, MENU_MAIN);
                break;
            case STATE_START:
                draw_text(window, data.components, data.camera, NULL, zeros(), "LOADING", sfWhite);
                break;
            case STATE_GAME:
                draw_game(data, window);
                break;
            case STATE_PAUSE:
                draw_game(data, window);
                draw_menu(data, window, MENU_PAUSE);
                draw_text(window, data.components, data.camera, NULL, zeros(), "PAUSED", sfWhite);
                break;
            case STATE_SETTINGS:
                draw_menu(data, window, MENU_SETTINGS);
                break;
            case STATE_APPLY:
                break;
            case STATE_QUIT:
                sfRenderWindow_close(window);
                break;
        }

        draw_fps(window, fps, delta_time);

        sfRenderWindow_display(window);

        play_sounds(data.components, data.camera, data.sounds, channels);
    }

    sfRenderWindow_destroy(window);

    return 0;
}
