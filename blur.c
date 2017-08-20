/*
 * vim:ts=4:sw=4:expandtab
 *
 * See LICENSE for licensing information
 *
 */
#include "blur.h"
#include <stdio.h>
#include <stdlib.h>
#include "math.h"

cairo_surface_t *screenshot_blur(xcb_connection_t *conn, xcb_screen_t *screen, int radius) {
    int width = screen->width_in_pixels,
        height = screen->height_in_pixels;
    xcb_get_image_cookie_t cookie = xcb_get_image(conn, XCB_IMAGE_FORMAT_Z_PIXMAP,
                                                  screen->root, 0, 0, width, height, (uint32_t)-1);
    xcb_generic_error_t *err;
    xcb_get_image_reply_t *reply = xcb_get_image_reply(conn, cookie, &err);
    if (!reply) {
        fprintf(stderr, "Could not get screenshot to blur\n");
        return NULL;
    }
    uint8_t *data_1 = xcb_get_image_data(reply);
    int image_length = xcb_get_image_data_length(reply);

    if (image_length != width * height * 4) {
        fprintf(stderr, "Screenshot image has unexpected dimensions\n");
        return NULL;
    }
    blur_t *blur = blur_init(radius);
    if (!blur) {
        fprintf(stderr, "Could not allocate memory to blur\n");
        return NULL;
    }
    uint8_t *data_2 = malloc(image_length);
    if (!data_2) {
        fprintf(stderr, "Could not allocate memory to blur\n");
        blur_free(blur);
        return NULL;
    }

    blur_horizontal(blur, width, height, (rgba *)data_1, (rgba *)data_2, radius);
    blur_vertical(blur, width, height, (rgba *)data_2, (rgba *)data_1, radius);
    free(data_2);
    blur_free(blur);
    blur = NULL;

    int stride = cairo_format_stride_for_width(CAIRO_FORMAT_RGB24,
                                               width);
    cairo_surface_t *surface = cairo_image_surface_create_for_data(data_1,
                                                                   CAIRO_FORMAT_RGB24, width, height, stride);
    return surface;
}

/* The following was amended from https://www.cairographics.org/cookbook/blur.c/
 *
 * Copyright © 2008 Kristian Høgsberg
 * Copyright © 2009 Chris Wilson
 *
 * Permission to use, copy, modify, distribute, and sell this software and its
 * documentation for any purpose is hereby granted without fee, provided that
 * the above copyright notice appear in all copies and that both that copyright
 * notice and this permission notice appear in supporting documentation, and
 * that the name of the copyright holders not be used in advertising or
 * publicity pertaining to distribution of the software without specific,
 * written prior permission.  The copyright holders make no representations
 * about the suitability of this software for any purpose.  It is provided "as
 * is" without express or implied warranty.
 *
 * THE COPYRIGHT HOLDERS DISCLAIM ALL WARRANTIES WITH REGARD TO THIS SOFTWARE,
 * INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO
 * EVENT SHALL THE COPYRIGHT HOLDERS BE LIABLE FOR ANY SPECIAL, INDIRECT OR
 * CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE,
 * DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER
 * TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE
 * OF THIS SOFTWARE.
 */

blur_t *blur_init(int radius) {
    blur_t *b = calloc(1, sizeof(blur_t));
    if (!b) {
        return NULL;
    }
    b->size = radius * 2;
    b->half = radius;
    b->kernel = malloc(b->size);

    b->a = 0;
    for (int i = 0; i < b->size; i++) {
        double f = i - b->half,
               x = exp(-f * f / (8 * radius)) * 2.5 * radius;
        b->a += x;
        b->kernel[i] = x;
    }
    return b;
}

void blur_free(blur_t *b) {
    free(b->kernel);
    free(b);
}

void blur_horizontal(blur_t *b, int width, int height, rgba *src, rgba *dst,
                     int radius) {
    for (int y = 0; y < height; y++) {
        rgba *src_row = src + y * width,
             *dst_row = dst + y * width;
        for (int x = 0; x < width; x++) {
            int c_r = 0, c_g = 0, c_b = 0, c_a = 0;
            for (int k = 0; k < b->size; k++) {
                if (x - b->half + k < 0 || x - b->half + k >= width)
                    continue;
                rgba p = src_row[x - b->half + k];
                c_r += p.r * b->kernel[k];
                c_g += p.g * b->kernel[k];
                c_b += p.b * b->kernel[k];
                c_a += p.a * b->kernel[k];
            }
            dst_row[x].r = c_r / b->a;
            dst_row[x].g = c_g / b->a;
            dst_row[x].b = c_b / b->a;
            dst_row[x].a = c_a / b->a;
        }
    }
}

void blur_vertical(blur_t *b, int width, int height, rgba *src, rgba *dst,
                   int radius) {
    for (int y = 0; y < height; y++) {
        rgba *dst_row = dst + y * width;
        for (int x = 0; x < width; x++) {
            int c_r = 0, c_g = 0, c_b = 0, c_a = 0;
            for (int k = 0; k < b->size; k++) {
                if (y - b->half + k < 0 || y - b->half + k >= height)
                    continue;

                rgba *src_row = src + (y - b->half + k) * width;
                rgba p = src_row[x];

                c_r += p.r * b->kernel[k];
                c_g += p.g * b->kernel[k];
                c_b += p.b * b->kernel[k];
                c_a += p.a * b->kernel[k];
            }
            dst_row[x].r = c_r / b->a;
            dst_row[x].g = c_g / b->a;
            dst_row[x].b = c_b / b->a;
            dst_row[x].a = c_a / b->a;
        }
    }
}
