#ifndef DUOTONE_FRAME_BUFFER_H
#define DUOTONE_FRAME_BUFFER_H

#include <stdint.h>

typedef struct
{
    uint16_t width;
    uint16_t height;
    uint8_t* buffer;
} duotone_fb_t;

/**
 * The width * height MUST be multiply to 8.
 */
#define duotone_fb_get_buffer_size(width, height) ((width)*(height)/8)

void duotone_fb_clear(duotone_fb_t* fb);

int duotone_fb_set_pixel(duotone_fb_t* fb, uint16_t x, uint16_t y, uint8_t value);

uint8_t duotone_fb_get_pixel(duotone_fb_t* fb, uint16_t x, uint16_t y);

#endif
