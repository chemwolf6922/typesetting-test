#include <string.h>
#include "duotone_frame_buffer.h"

void duotone_fb_clear(duotone_fb_t* fb)
{
    /** clear means fill with white */
    memset(fb->buffer, 0xFF, duotone_fb_get_buffer_size(fb->width, fb->height));
}

/**
 * @param fb The frame buffer
 * @param x The x coordinate
 * @param y The y coordinate
 * @param value The value of the pixel, 0 for black, 1 for white
 */
int duotone_fb_set_pixel(duotone_fb_t* fb, uint16_t x, uint16_t y, uint8_t value)
{
    if(x >= fb->width || y >= fb->height)
        return -1;
    int byte_index = (y * fb->width + x) / 8;
    int bit_index = (y * fb->width + x) % 8;
    if(value == 0)
        fb->buffer[byte_index] &= ~(1 << bit_index);
    else
        fb->buffer[byte_index] |= (1 << bit_index);
    return 0;
}

uint8_t duotone_fb_get_pixel(duotone_fb_t* fb, uint16_t x, uint16_t y)
{
    if(x >= fb->width || y >= fb->height)
        return -1;
    int byte_index = (y * fb->width + x) / 8;
    int bit_index = (y * fb->width + x) % 8;
    return (fb->buffer[byte_index] >> bit_index) & 0x01;
}