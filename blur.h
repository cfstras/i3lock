#ifndef _BLUR_H
#define _BLUR_H

#include <cairo.h>
#include <xcb/xcb.h>
#include <xcb/xproto.h>

cairo_surface_t *screenshot_blur(xcb_connection_t *conn, xcb_screen_t *screen,
                                 int radius);

void blur_image_surface(cairo_surface_t *surface, int radius);

typedef struct blur_t {
    uint8_t *kernel;
    int size, half;
    uint32_t a;
} blur_t;

#pragma pack(push, 1)
typedef struct rgba {
    uint8_t r, g, b, a;
} rgba;
#pragma pack(pop)

blur_t *blur_init(int radius);
void blur_free(blur_t *blur);

void blur_horizontal(blur_t *b, int width, int height, rgba *src, rgba *dst,
                     int radius);
void blur_vertical(blur_t *b, int width, int height, rgba *src, rgba *dst,
                   int radius);

#endif
