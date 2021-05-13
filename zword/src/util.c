#define _USE_MATH_DEFINES

#include <math.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>

#include <SFML/System/Vector2.h>
#include <SFML/Graphics.h>

#include "util.h"


sfVector2f zeros() {
    return (sfVector2f) { 0.0, 0.0 };
}

sfVector2f ones() {
    return (sfVector2f) { 1.0, 1.0 };
}

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
    if (n == 0.0) {
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

float signed_angle(sfVector2f a, sfVector2f b) {
    // https://stackoverflow.com/questions/2150050/finding-signed-angle-between-vectors
    return atan2f(a.x * b.y - a.y * b.x, a.x * b.x + a.y * b.y);
}

sfVector2f bisector(sfVector2f a, sfVector2f b) {
    return normalized(sum(normalized(a), normalized(b)));
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
    if (x == 0.0) return 0.0;
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

sfVector2f lin_comb(float a, sfVector2f v, float b, sfVector2f u) {
    return sum(mult(a, v), mult(b, u));
}

float randf(float low, float upp) {
    float scale = rand() / (float) RAND_MAX;
    return low + scale * (upp - low);
}

int randi(int low, int upp) {
    return rand() % (upp + 1 - low) + low;
}

float rand_angle() {
    return randf(0.0, 2 * M_PI);
}

sfVector2f rand_vector() {
    return polar_to_cartesian(1.0, rand_angle());
}

int find(int value, int* array, int size) {
    for (int i = 0; i < size; i++) {
        if (array[i] == value) {
            return i;
        }
    }

    return -1;
}

int replace(int old, int new, int* array, int size) {
    int i = find(old, array, size);
    if (i != -1) {
        array[i] = new;
    }
    return i;
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

sfVector2f rotate(sfVector2f v, float angle) {
    float c = cosf(angle);
    float s = sinf(angle);
    return (sfVector2f) { v.x * c - v.y * s, v.x * s + v.y * c };
}

float polar_angle(sfVector2f v) {
    return atan2f(v.y, v.x);
}

Matrix2f rotation_matrix(float angle) {
    float c = cosf(angle);
    float s = sinf(angle);
    return (Matrix2f) { c, -s, s, c };
}

sfVector2f matrix_mult(Matrix2f m, sfVector2f v) {
    return (sfVector2f) { m.a * v.x + m.b * v.y, m.c * v.x + m.d * v.y };
}

Matrix2f transpose(Matrix2f m) {
    return (Matrix2f) { m.a, m.c, m.b, m.d };
}

Matrix2f matrix_inverse(Matrix2f m) {
    float det = m.a * m.d - m.b * m.c;
    return (Matrix2f) { m.d / det, -m.b / det, -m.c / det, m.a / det };
}

sfColor get_color(float r, float g, float b, float a) {
    return sfColor_fromRGBA(r * 255, g * 255, b * 255, a * 255);
}

void permute(int* array, int size) {
    for (int i = 0; i < size; i++) {
        // Warning: skewed distribution
        int j = rand() % (i + 1);
        int t = array[i];
        array[i] = array[j];
        array[j] = t;
    }
}

float lerp(float a, float b, float t) {
    return a + t * (b - a);
}

float smoothstep(float x, float mu, float nu) {
    // https://wernerantweiler.ca/blog.php?item=2018-11-03
    return powf(1.0 + powf(x * (1.0 - mu) / (mu * (1.0 - x)), -nu), -1.0);
}

int binary_search_filename(Filename filename, char** array, int size) {
    int l = 0;
    int r = size - 1;

    while (l <= r) {
        int m = floor((l + r) / 2);

        if (strcmp(array[m], filename) < 0) {
            l = m + 1;
        } else if (strcmp(array[m], filename) > 0) {
            r = m - 1;
        } else {
            return m;
        }
    }

    return -1;
}

bool non_zero(sfVector2f v) {
    return (v.x != 0.0f || v.y != 0.0f);
}

float clamp(float val, float min_val, float max_val) {
    return fmaxf(fminf(val, max_val), min_val);
}
