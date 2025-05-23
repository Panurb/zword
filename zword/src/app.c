#include <stdio.h>

#include <SDL.h>
#include <SDL_image.h>

#include "app.h"
#include "game.h"
#include "settings.h"
#include "interface.h"
#include "player.h"
#include "menu.h"
#include "editor.h"
#include "hud.h"
#include "input.h"
#include "serialize.h"


App app;

static String version = "0.3";

static float title_scale = 2.0f;

static int debug_level = 0;

typedef struct {
    int page;
    int panel;
    Vector2f position;
    float scale;
} Intro;

static Intro intro = {1, 0, {0.0f, 0.0f}, 3.0f};


void create_screen_textures() {
    app.shadow_texture = SDL_CreateTexture(app.renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, 
        game_settings.width, game_settings.height);
    SDL_SetTextureBlendMode(app.shadow_texture, SDL_BLENDMODE_BLEND);

    app.light_texture = SDL_CreateTexture(app.renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, 
        game_settings.width, game_settings.height);
    SDL_SetTextureBlendMode(app.light_texture, SDL_BLENDMODE_MUL);

    app.blood_texture = SDL_CreateTexture(app.renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET,
        game_settings.width, game_settings.height);
    SDL_SetTextureBlendMode(app.blood_texture, SDL_BLENDMODE_BLEND);

    int threshold = 32;
    app.blood_threshold_texture = SDL_CreateTexture(app.renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET,
        game_settings.width, game_settings.height);
    SDL_SetRenderTarget(app.renderer, app.blood_threshold_texture);
    SDL_SetRenderDrawColor(app.renderer, 0, 0, 0, 255 - threshold);
    SDL_RenderClear(app.renderer);
    SDL_SetRenderTarget(app.renderer, NULL);

    SDL_BlendMode bm = SDL_ComposeCustomBlendMode(
        SDL_BLENDFACTOR_ZERO, SDL_BLENDFACTOR_ONE, SDL_BLENDOPERATION_ADD, 
        SDL_BLENDFACTOR_ONE, SDL_BLENDFACTOR_ONE, SDL_BLENDOPERATION_ADD
    );
    SDL_SetTextureBlendMode(app.blood_threshold_texture, bm);

    app.blood_multiply_texture = SDL_CreateTexture(app.renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET,
        game_settings.width, game_settings.height);

    bm = SDL_ComposeCustomBlendMode(
        SDL_BLENDFACTOR_ZERO, SDL_BLENDFACTOR_ONE, SDL_BLENDOPERATION_ADD, 
        SDL_BLENDFACTOR_ZERO, SDL_BLENDFACTOR_DST_ALPHA, SDL_BLENDOPERATION_ADD
    );
    SDL_SetTextureBlendMode(app.blood_multiply_texture, bm);
}


void create_game_window() {
    app.window = SDL_CreateWindow("NotK", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 
        game_settings.width, game_settings.height, SDL_WINDOW_SHOWN);
    SDL_SetWindowFullscreen(app.window, game_settings.fullscreen ? SDL_WINDOW_FULLSCREEN : 0);
    SDL_SetHint(SDL_HINT_RENDER_VSYNC, game_settings.vsync ? "1" : "0");
    app.renderer = SDL_CreateRenderer(app.window, -1, SDL_RENDERER_ACCELERATED);

    create_screen_textures();
}


void destroy_game_window() {
    SDL_DestroyWindow(app.window);
    app.window = NULL;

    // This will also destroy all textures
    SDL_DestroyRenderer(app.renderer);
    app.renderer = NULL;
    app.shadow_texture = NULL;
    app.light_texture = NULL;
    app.blood_texture = NULL;
    app.blood_threshold_texture = NULL;
    app.blood_multiply_texture = NULL;
}


void resize_game_window() {
    int w;
    int h;
    SDL_GetWindowSize(app.window, &w, &h);
    if (game_settings.width != w || game_settings.height != h) {
        SDL_SetWindowSize(app.window, game_settings.width, game_settings.height);
        
        SDL_DestroyTexture(app.shadow_texture);
        SDL_DestroyTexture(app.light_texture);
        SDL_DestroyTexture(app.blood_texture);
        SDL_DestroyTexture(app.blood_threshold_texture);
        SDL_DestroyTexture(app.blood_multiply_texture);

        create_screen_textures();
    }

    SDL_SetWindowFullscreen(app.window, game_settings.fullscreen ? SDL_WINDOW_FULLSCREEN : 0);
}


void init() {
    setbuf(stdout, NULL);

    SDL_Init(SDL_INIT_VIDEO | SDL_INIT_GAMECONTROLLER | SDL_INIT_AUDIO);
    SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "1");
    SDL_ShowCursor(SDL_DISABLE);
    
    IMG_Init(IMG_INIT_PNG);
    TTF_Init();
    Mix_OpenAudio(MIX_DEFAULT_FREQUENCY, MIX_DEFAULT_FORMAT, MIX_DEFAULT_CHANNELS, 2048);
    Mix_AllocateChannels(32);

    memset(app.controllers, 0, sizeof(app.controllers));

    int controllers = SDL_NumJoysticks();
    for (int i = 0; i < controllers; i++) {
        if (SDL_IsGameController(i)) {
            if (app.controllers[i] == NULL) {
                app.controllers[i] = SDL_GameControllerOpen(i);
            }
        }
    }
    for (int i = 0; i < 4; i++) {
        app.player_controllers[i] = CONTROLLER_NONE;
    }
    app.player_controllers[0] = CONTROLLER_MKB;

    create_game_window();

    app.fps = FpsCounter_create();

    app.quit = false;
    app.focus = true;
    app.time_step = 1.0f / 60.0f;
    app.delta = 0.0f;
}


void quit() {
    free(app.fps);
    destroy_game_window();

    Mix_CloseAudio();
    TTF_Quit();
    IMG_Quit();
    SDL_Quit();
}


void input_game(SDL_Event sdl_event) {
    switch (game_state) {
        case STATE_GAME:
            if (sdl_event.type == SDL_KEYDOWN && sdl_event.key.repeat == 0) {
                if (sdl_event.key.keysym.sym == SDLK_ESCAPE) {
                    if (game_data->testing) {
                        game_state = STATE_LOAD_EDITOR;
                    } else {
                        game_state = STATE_PAUSE;
                    }
                } else if (sdl_event.key.keysym.sym == SDLK_F1) {
                    if (game_settings.debug) {
                        debug_level = (debug_level + 1) % 4;
                    }
                }
            }

            if (game_settings.debug) {
                if (sdl_event.type == SDL_MOUSEWHEEL) {
                    CameraComponent* camera = CameraComponent_get(game_data->camera);
                    if (sdl_event.wheel.y > 0) {
                        camera->zoom_target = fminf(camera->zoom_target * 1.1f, 100.0f);
                    } else if (sdl_event.wheel.y < 0) {
                        camera->zoom_target = fmaxf(camera->zoom_target / 1.1f, 10.0f);
                    }
                }
            }
            break;
        case STATE_PAUSE:
            if (sdl_event.type == SDL_KEYDOWN && sdl_event.key.keysym.sym == SDLK_ESCAPE) {
                game_state = STATE_GAME;
            }
            input_menu(game_data->menu_camera, sdl_event);
            break;
        case STATE_EDITOR:
            input_editor(sdl_event);
            break;
        case STATE_MENU:
        case STATE_GAME_OVER:
            input_menu(game_data->menu_camera, sdl_event);
            break;
        case STATE_INTRO:
            if (sdl_event.type == SDL_KEYDOWN && sdl_event.key.keysym.sym == SDLK_ESCAPE) {
                game_state = STATE_START;
            } else if (sdl_event.type == SDL_MOUSEBUTTONDOWN) {
                intro.panel++;
                if (intro.panel >= 6) {
                    intro.panel = 0;
                    intro.page++;
                    if (intro.page > 4) {
                        game_state = STATE_START;
                    }
                }
            }
            break;
    default:
        break;
    }

}


void input() {
    SDL_Event sdl_event;
    while (SDL_PollEvent(&sdl_event))
    {
        switch (sdl_event.type) {
            case SDL_WINDOWEVENT:
                if (sdl_event.window.event == SDL_WINDOWEVENT_FOCUS_LOST) {
                    app.focus = false;
                } else if (sdl_event.window.event == SDL_WINDOWEVENT_FOCUS_GAINED) {
                    app.focus = true;
                }
                break;
            case SDL_QUIT:
                app.quit = true;
                return;
            case SDL_JOYDEVICEADDED:
                if (app.controllers[sdl_event.jdevice.which] == NULL) {
                    app.controllers[sdl_event.jdevice.which] = SDL_GameControllerOpen(sdl_event.jdevice.which);
                    LOG_INFO("Joystick added: %d", sdl_event.jdevice.which)
                }
                break;
            case SDL_JOYDEVICEREMOVED:
                for (int i = 0; i < 8; i++) {
                    if (app.controllers[i] == NULL) continue;

                    if (SDL_JoystickInstanceID(SDL_GameControllerGetJoystick(app.controllers[i])) == sdl_event.jdevice.which) {
                        SDL_GameControllerClose(app.controllers[i]);
                        app.controllers[i] = NULL;
                        LOG_INFO("Joystick removed: %d", sdl_event.jdevice.which)
                    }
                }
                break;
            default:
                input_game(sdl_event);
                break;
        }
    }
}


void update(float time_step) {
    static GameState previous_state = STATE_MENU;

    GameState state = game_state;

    switch (game_state) {
        case STATE_MENU:
            intro.page = 1;
            intro.panel = 0;
            update_menu();
            break;
        case STATE_START:
            start_game(game_data->map_name, false);
            game_state = STATE_GAME;
            break;
        case STATE_END:
            end_game();
            clear_all_sounds();
            game_state = STATE_MENU;
            break;
        case STATE_RESET:
            end_game();
            clear_all_sounds();
            game_state = STATE_START;
            break;
        case STATE_GAME:
            input_players(game_data->camera);
            update_game(time_step);
            update_game_mode(time_step);
            break;
        case STATE_PAUSE:
            update_menu();
            break;
        case STATE_SAVE:
            save_state(game_data->map_name);
            game_state = previous_state;
            break;
        case STATE_LOAD:
            end_game();
            clear_all_sounds();
            start_game(game_data->map_name, true);
            game_state = STATE_GAME;
            break;
        case STATE_APPLY:
            resize_game();
            resize_game_window();
            save_settings();
            game_state = STATE_MENU;
            break;
        case STATE_CREATE:
            destroy_menu();
            create_editor_menu();
            game_state = STATE_EDITOR;
            break;
        case STATE_LOAD_EDITOR:
            game_data->testing = false;
            end_game();
            CoordinateComponent_get(game_data->camera)->position = game_data->start_position;
            create_editor_menu();
            load_map(game_data->map_name);
            game_state = STATE_EDITOR;
            break;
        case STATE_EDITOR:
            update_editor(time_step);
            break;
        case STATE_GAME_OVER:
            update_game_over(time_step);
            break;
        case STATE_QUIT:
            app.quit = true;
            return;
        case STATE_INTRO:
            ;
            Vector2f target = mult(intro.scale * 9.0f, vec(0.5f - intro.panel % 2, intro.panel / 2 - 1));
            intro.position = sum(intro.position, mult(0.1f, diff(target, intro.position)));
            break;
    }

    if (state != game_state) {
        previous_state = state;
    }

    float time = SDL_GetTicks() / 1000.0f;
    title_scale = 2.5f + 0.1f * sinf(2.0f * time);
}


void draw() {
    SDL_SetRenderDrawBlendMode(app.renderer, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(app.renderer, 0, 0, 0, 255);
    SDL_RenderClear(app.renderer);

    switch (game_state) {
        case STATE_MENU:
            draw_sprite(game_data->menu_camera, get_texture_index("menu"), 0, 0, 0, zeros(), 0.0f, mult(3.5f, ones()), 1.0f);
            draw_sprite(game_data->menu_camera, get_texture_index("title"), 0, 0, 0, vec(0.0f, 9.0f), 0.0f, vec(title_scale, title_scale), 1.0f);
            draw_menu();

            String buffer;
            snprintf(buffer, STRING_SIZE, "v%s", version);
            Color color = get_color(1.0f, 1.0f, 0.0f, 0.5f);
            draw_text(game_data->menu_camera, vec(24.5f, -13.5f), buffer, 20, color);
            break;
        case STATE_START:
        case STATE_END:
        case STATE_RESET:
            draw_text(game_data->menu_camera, zeros(), "LOADING", 20, COLOR_WHITE);
            break;
        case STATE_GAME:
            draw_game();
            draw_hud(game_data->camera);
            draw_game_mode();
            break;
        case STATE_PAUSE:
            app.delta = 0.0f;
            draw_game();
            draw_overlay(game_data->camera, 0.4f);
            draw_menu();
            break;
        case STATE_LOAD_EDITOR:
            draw_text(game_data->menu_camera, zeros(), "LOADING", 20, COLOR_WHITE);
            break;
        case STATE_EDITOR:
            draw_editor();
            draw_hud(game_data->camera);
            break;
        case STATE_GAME_OVER:
            draw_game_over();
            break;
        case STATE_INTRO:
            ;
            String filename;
            snprintf(filename, STRING_SIZE, "intro_%d", intro.page);
            draw_sprite(game_data->menu_camera, get_texture_index(filename), 0, 0, 0, intro.position, 0.0f, mult(intro.scale, ones()), 1.0f);

            Vector2f pos = get_mouse_position(game_data->menu_camera);
            draw_circle(game_data->menu_camera, pos, 0.1f, COLOR_WHITE);
            break;
        default:
            break;
    }

    if (debug_level) {
        draw_debug(debug_level);
    }

    FPSCounter_draw(app.fps);

    SDL_RenderPresent(app.renderer);
}


void play_audio() {
    static bool music_playing = false;
    static float music_fade = 0.0f;
    static int current_music = -1;

    switch(game_state) {
        case STATE_MENU:
            game_data->music = 0;
            break;
        case STATE_GAME:
        case STATE_INTRO:
        case STATE_PAUSE:
            game_data->music = 1;
            break;
        default:
            game_data->music = -1;
            break;
    }

    if (game_data->music != current_music) {
        music_fade = fmaxf(0.0f, music_fade - 0.01f);

        if (music_fade == 0.0f) {
            current_music = game_data->music;

            Mix_HaltMusic();
            if (game_data->music != -1) {
                Mix_PlayMusic(resources.music[game_data->music], -1);
            }
        }
    } else {
        music_fade = fminf(1.0f, music_fade + 0.01f);
    }

    play_sounds(game_data->camera);

    Mix_VolumeMusic(0.5f * game_settings.music * music_fade);

    if (!music_playing) {
        Mix_VolumeMusic(0);
        Mix_PlayMusic(resources.music[0], -1);
        music_playing = true;
    }
}
