#pragma once


typedef struct {
    int width;
    int height;
    int antialiasing;
    bool fullscreen;
} Settings;

typedef struct {
    char* key;
    int value;
} KeyValue;

extern Settings game_settings;

void load_settings();

void save_settings();
