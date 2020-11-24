#define _USE_MATH_DEFINES

#include <math.h>
#include <stdlib.h>

#include <SFML/System/Vector2.h>


float norm(sfVector2f v) {
    return sqrtf(v.x * v.x + v.y * v.y);
}

float norm2(sfVector2f v) {
    return v.x * v.x + v.y * v.y;
}

float dist(sfVector2f a, sfVector2f b) {
    float x = a.x - b.x;
    float y = a.y - b.y;
    return sqrtf(x * x + y * y);
}

sfVector2f normalized(sfVector2f v) {
    float n = norm(v);
    if (fabs(n) < 1e-6) {
        return v;
    }
    return (sfVector2f) { v.x / n, v.y / n };
}

double to_degrees(double radians) {
    return radians * (180.0 / M_PI);
}

float dot(sfVector2f a, sfVector2f b) {
    return a.x * b.x + a.y * b.y;
}

sfVector2f polar_to_cartesian(float length, float angle) {
    sfVector2f v = { length * cosf(angle), length * sinf(angle) };
    return v;
}

int abs_argmin(float* a, int n) {
    int i = 0;

    for (int j = 1; j < n; j++) {
        if (fabs(a[j]) < fabs(a[i])) {
            i = j;
        }
    }

    return i;
}

float mean(float* array, int size) {
    float tot = 0.0;
    for (int i = 0; i < size; i++) {
        tot += array[i];
    }
    return tot / size;
}

sfVector2f perp(sfVector2f v) {
    return (sfVector2f) { -v.y, v.x };
}

float sign(float x) {
    return copysignf(1.0, x);
}

sfVector2f sum(sfVector2f v, sfVector2f u) {
    return (sfVector2f) { v.x + u.x, v.y + u.y };
}

sfVector2f diff(sfVector2f v, sfVector2f u) {
    return (sfVector2f) { v.x - u.x, v.y - u.y };
}

sfVector2f mult(float c, sfVector2f v) {
    return (sfVector2f) { c * v.x, c * v.y };
}

sfVector2f proj(sfVector2f a, sfVector2f b) {
    sfVector2f b_norm = normalized(b);
    return mult(dot(a, b_norm), b_norm);
}

float float_rand(float min, float max) {
    float scale = rand() / (float) RAND_MAX;
    return min + scale * (max - min);
}

int find(int value, int* array, int size) {
    for (int i; i < size; i++) {
        if (array[i] == value) {
            return i;
        }
    }

    return -1;
}

int min(int a, int b) {
    return (a < b) ? a : b;
}

int max(int a, int b) {
    return (a > b) ? a : b;
}

float mod(float x, float y) {
    return fmod(fmod(x, y) + y, y);
}

float cross(sfVector2f v, sfVector2f u) {
    return v.x * u.y - v.y * u.x;
}
