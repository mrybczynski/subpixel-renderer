#include "renderer.h"
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <stdbool.h>

Buffer* create_buffer(int width, int height) {
    Buffer *buf = (Buffer*)malloc(sizeof(Buffer));
    buf->width = width;
    buf->height = height;
    buf->pixels = (uint32_t*)malloc(width * height * sizeof(uint32_t));
    return buf;
}

void destroy_buffer(Buffer *buf) {
    if (buf) {
        free(buf->pixels);
        free(buf);
    }
}

void clear_buffer(Buffer *buf, Color color) {
    uint32_t c = (color.a << 24) | (color.r << 16) | (color.g << 8) | color.b;
    for (int i = 0; i < buf->width * buf->height; i++) {
        buf->pixels[i] = c;
    }
}

void draw_pixel_aa(Buffer *buf, float x, float y, Color color, float coverage) {
    int ix = (int)x;
    int iy = (int)y;

    if (ix < 0 || ix >= buf->width || iy < 0 || iy >= buf->height) return;

    uint32_t *pixel = &buf->pixels[iy * buf->width + ix];
    
    // Extract current color
    uint8_t cb = (*pixel >> 0) & 0xFF;
    uint8_t cg = (*pixel >> 8) & 0xFF;
    uint8_t cr = (*pixel >> 16) & 0xFF;

    // Blend new color based on coverage
    float alpha = coverage * (color.a / 255.0f);
    uint8_t out_r = (uint8_t)(cr * (1.0f - alpha) + color.r * alpha);
    uint8_t out_g = (uint8_t)(cg * (1.0f - alpha) + color.g * alpha);
    uint8_t out_b = (uint8_t)(cb * (1.0f - alpha) + color.b * alpha);

    *pixel = (color.a << 24) | (out_r << 16) | (out_g << 8) | out_b;
}

// Helper for Wu's line algorithm
static inline float fpart(float x) { return x - floorf(x); }
static inline float rfpart(float x) { return 1.0f - fpart(x); }

void draw_line_aa(Buffer *buf, float x1, float y1, float x2, float y2, Color color) {
    bool steep = fabsf(y2 - y1) > fabsf(x2 - x1);

    if (steep) {
        float temp = x1; x1 = y1; y1 = temp;
        temp = x2; x2 = y2; y2 = temp;
    }
    if (x1 > x2) {
        float temp = x1; x1 = x2; x2 = temp;
        temp = y1; y1 = y2; y2 = temp;
    }

    float dx = x2 - x1;
    float dy = y2 - y1;
    float gradient = (dx == 0.0f) ? 1.0f : dy / dx;

    // Handle first endpoint
    float xend = roundf(x1);
    float yend = y1 + gradient * (xend - x1);
    float xgap = rfpart(x1 + 0.5f);
    float xpxl1 = xend;
    float ypxl1 = floorf(yend);

    if (steep) {
        draw_pixel_aa(buf, ypxl1, xpxl1, color, rfpart(yend) * xgap);
        draw_pixel_aa(buf, ypxl1 + 1, xpxl1, color, fpart(yend) * xgap);
    } else {
        draw_pixel_aa(buf, xpxl1, ypxl1, color, rfpart(yend) * xgap);
        draw_pixel_aa(buf, xpxl1, ypxl1 + 1, color, fpart(yend) * xgap);
    }
    float intery = yend + gradient;

    // Handle second endpoint
    xend = roundf(x2);
    yend = y2 + gradient * (xend - x2);
    xgap = fpart(x2 + 0.5f);
    float xpxl2 = xend;
    float ypxl2 = floorf(yend);

    if (steep) {
        draw_pixel_aa(buf, ypxl2, xpxl2, color, rfpart(yend) * xgap);
        draw_pixel_aa(buf, ypxl2 + 1, xpxl2, color, fpart(yend) * xgap);
    } else {
        draw_pixel_aa(buf, xpxl2, ypxl2, color, rfpart(yend) * xgap);
        draw_pixel_aa(buf, xpxl2, ypxl2 + 1, color, fpart(yend) * xgap);
    }

    // Main loop
    if (steep) {
        for (int x = (int)xpxl1 + 1; x < (int)xpxl2; x++) {
            draw_pixel_aa(buf, floorf(intery), (float)x, color, rfpart(intery));
            draw_pixel_aa(buf, floorf(intery) + 1, (float)x, color, fpart(intery));
            intery += gradient;
        }
    } else {
        for (int x = (int)xpxl1 + 1; x < (int)xpxl2; x++) {
            draw_pixel_aa(buf, (float)x, floorf(intery), color, rfpart(intery));
            draw_pixel_aa(buf, (float)x, floorf(intery) + 1, color, fpart(intery));
            intery += gradient;
        }
    }
}

void draw_circle_aa(Buffer *buf, float cx, float cy, float radius, Color color) {
    // Simple subpixel circle using coverage estimation
    int x_start = (int)(cx - radius - 1);
    int y_start = (int)(cy - radius - 1);
    int x_end = (int)(cx + radius + 1);
    int y_end = (int)(cy + radius + 1);

    for (int y = y_start; y <= y_end; y++) {
        for (int x = x_start; x <= x_end; x++) {
            float dx = (float)x - cx;
            float dy = (float)y - cy;
            float dist = sqrtf(dx * dx + dy * dy);
            
            // Subpixel coverage calculation (linear ramp over 1 pixel edge)
            float diff = dist - radius;
            float coverage = 0.0f;
            if (diff <= -0.5f) coverage = 1.0f;
            else if (diff >= 0.5f) coverage = 0.0f;
            else coverage = 0.5f - diff;

            if (coverage > 0.0f) {
                draw_pixel_aa(buf, (float)x, (float)y, color, coverage);
            }
        }
    }
}

static const unsigned char font5x7[128][5] = {
    ['0']={0x3E, 0x51, 0x49, 0x45, 0x3E}, ['1']={0x00, 0x42, 0x7F, 0x40, 0x00},
    ['2']={0x42, 0x61, 0x51, 0x49, 0x46}, ['3']={0x21, 0x41, 0x45, 0x4B, 0x31},
    ['4']={0x18, 0x14, 0x12, 0x7F, 0x10}, ['5']={0x27, 0x45, 0x45, 0x45, 0x39},
    ['6']={0x3C, 0x4A, 0x49, 0x49, 0x30}, ['7']={0x01, 0x71, 0x09, 0x05, 0x03},
    ['8']={0x36, 0x49, 0x49, 0x49, 0x36}, ['9']={0x06, 0x49, 0x49, 0x29, 0x1E},
    ['F']={0x7F, 0x09, 0x09, 0x09, 0x01}, ['P']={0x7F, 0x09, 0x09, 0x09, 0x06},
    ['S']={0x26, 0x49, 0x49, 0x49, 0x32}, [' ']={0x00, 0x00, 0x00, 0x00, 0x00},
    ['.']={0x00, 0x60, 0x60, 0x00, 0x00}, [':']={0x00, 0x36, 0x36, 0x00, 0x00}
};

void draw_text(Buffer *buf, int x, int y, const char *text, Color color) {
    while (*text) {
        unsigned char c = (unsigned char)*text++;
        if (c < 128) {
            for (int i = 0; i < 5; i++) {
                unsigned char bits = font5x7[c][i];
                for (int j = 0; j < 7; j++) {
                    if (bits & (1 << j)) {
                        draw_pixel_aa(buf, (float)(x + i), (float)(y + j), color, 1.0f);
                    }
                }
            }
        }
        x += 6;
    }
}
