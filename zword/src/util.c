#define _USE_MATH_DEFINES

#include <math.h>

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
    sfVector2f vn = { v.x / n, v.y / n };
    return vn;
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
