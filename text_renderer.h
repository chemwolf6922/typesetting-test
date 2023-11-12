#ifndef TEXT_RENDERER_H
#define TEXT_RENDERER_H

#include <stdint.h>
#include <stddef.h>
#include "duotone_frame_buffer.h"

typedef void* text_renderer_handle;

text_renderer_handle text_renderer_new(uint8_t* font, size_t font_length);

void text_renderer_free(text_renderer_handle tr);

char* render_text_in_box(text_renderer_handle tr, const char* text, int font_size, float line_space, int x, int y, int width, int height, duotone_fb_t* fb);

#endif

