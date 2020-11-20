#pragma once

#include <stdlib.h>

#include <SFML/System/Vector2.h>


float norm(sfVector2f v);

float norm2(sfVector2f v);

float dist(sfVector2f a, sfVector2f b);

sfVector2f normalized(sfVector2f v);

double to_degrees(double radians);

float dot(sfVector2f a, sfVector2f b);

sfVector2f polar_to_cartesian(float length, float angle);

int abs_argmin(float* a, int n);

float mean(float* array, int size);

sfVector2f perp(sfVector2f v);

float sign(float x);

sfVector2f sum(sfVector2f v, sfVector2f u);

sfVector2f diff(sfVector2f v, sfVector2f u);

sfVector2f mult(float c, sfVector2f v);

sfVector2f proj(sfVector2f a, sfVector2f b);

static inline float float_rand(float min, float max) {
    float scale = rand() / (float) RAND_MAX;
    return min + scale * (max - min);
}
