#ifndef RENDERER_H
#define RENDERER_H

#include "types.h"

// Buffer management
Buffer* create_buffer(int width, int height);
void destroy_buffer(Buffer *buf);
void clear_buffer(Buffer *buf, Color color);

// Primitive drawing (Subpixel/AA)
void draw_pixel_aa(Buffer *buf, float x, float y, Color color, float coverage);
void draw_line_aa(Buffer *buf, float x1, float y1, float x2, float y2, Color color);
void draw_circle_aa(Buffer *buf, float x, float y, float radius, Color color);
void draw_text(Buffer *buf, int x, int y, const char *text, Color color);

#endif // RENDERER_H
