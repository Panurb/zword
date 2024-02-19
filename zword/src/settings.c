#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>

#include <SFML/Graphics.h>

#include "settings.h"
#include "input.h"


Settings game_settings = { 1920, 1080, 8, 0, 0, 0, 100, 100, 
    {
        sfKeyW, sfKeyA, sfKeyS, sfKeyD, 
        sfMouseLeft, sfKeyE, sfMouseRight, sfKeyR,
        sfKeyF, sfKeySpace, sfKeyLShift
    },
    {
        INPUT_KEYBOARD, INPUT_KEYBOARD, INPUT_KEYBOARD, INPUT_KEYBOARD,
        INPUT_MOUSE, INPUT_KEYBOARD, INPUT_MOUSE, INPUT_KEYBOARD,
        INPUT_KEYBOARD, INPUT_KEYBOARD, INPUT_KEYBOARD
    } 
};


KeyValue parse_line(Line string, char* delim) {
    char* token = strtok(string, delim);
    char* key = token;
    token = strtok(NULL, delim);
    int value = strtol(token, NULL, 10);

    return (KeyValue) { key, value };
}


void load_settings() {
    FILE* file = fopen("config.cfg", "r");

    if (!file) {
        return;
    }

    Line buffer;
    while (fgets(buffer, 255, file)) {
        KeyValue line = parse_line(buffer, "=");

        printf("%s = %i\n", line.key, line.value);

        if (strcmp(line.key, "WIDTH") == 0) {
            game_settings.width = line.value;
        } else if (strcmp(line.key, "HEIGHT") == 0) {
            game_settings.height = line.value;
        } else if (strcmp(line.key, "ANTIALIASING") == 0) {
            game_settings.antialiasing = line.value;
        } else if (strcmp(line.key, "FULLSCREEN") == 0) {
            game_settings.fullscreen = line.value;
        } else if (strcmp(line.key, "VSYNC") == 0) {
            game_settings.vsync = line.value;
        } else if (strcmp(line.key, "MAX_FPS") == 0) {
            game_settings.max_fps = line.value;
        } else if (strcmp(line.key, "VOLUME") == 0) {
            game_settings.volume = line.value;
        } else if (strcmp(line.key, "MUSIC") == 0) {
            game_settings.music = line.value;
        } else {
            for (int i = 0; i < ACTIONS_SIZE; i++) {
                if (strcmp(line.key, ACTIONS[i]) == 0) {
                    if (line.value == 0) {
                        game_settings.keybinds_device[i] = INPUT_UNBOUND;
                        game_settings.keybinds[i] = 0;
                    } else if (line.value < 0) {
                        game_settings.keybinds_device[i] = INPUT_MOUSE;
                        game_settings.keybinds[i] = -line.value - 1;
                    } else {
                        game_settings.keybinds_device[i] = INPUT_KEYBOARD;
                        game_settings.keybinds[i] = line.value - 1;
                    }
                }
            }
        }
    }

    fclose(file);
}


void save_settings() {
    FILE* file = fopen("config.cfg", "w");

    fprintf(file, "WIDTH=%i\n", game_settings.width);
    fprintf(file, "HEIGHT=%i\n", game_settings.height);
    fprintf(file, "ANTIALIASING=%i\n", game_settings.antialiasing);
    fprintf(file, "FULLSCREEN=%i\n", game_settings.fullscreen);
    fprintf(file, "VSYNC=%i\n", game_settings.vsync);
    fprintf(file, "MAX_FPS=%i\n", game_settings.max_fps);
    fprintf(file, "VOLUME=%i\n", game_settings.volume);
    fprintf(file, "MUSIC=%i\n", game_settings.music);
    for (int i = 0; i < ACTIONS_SIZE; i++) {
        int value = 0;
        if (game_settings.keybinds_device[i] == INPUT_KEYBOARD) {
            value = game_settings.keybinds[i] + 1;
        } else if (game_settings.keybinds_device[i] == INPUT_MOUSE) {
            value = -game_settings.keybinds[i] - 1;
        }
        fprintf(file, "%s=%i\n", ACTIONS[i], value);
    }

    fclose(file);
}
