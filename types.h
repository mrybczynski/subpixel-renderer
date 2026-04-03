#ifndef TYPES_H
#define TYPES_H

#include <stdint.h>

typedef struct {
    uint8_t b, g, r, a;
} Color;

typedef struct {
    int width;
    int height;
    uint32_t *pixels;
} Buffer;

typedef struct {
    float x, y;
} Point;

#endif // TYPES_H
