#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "text_renderer.h"
#include "libschrift/schrift.h"

typedef struct
{
    SFT sft;
} text_renderer_t;

typedef struct
{
    int16_t x_offset;
    int16_t y_offset;
    int16_t advance;
    uint16_t width;
    uint16_t height;
    uint8_t data[];
} tr_char_t;

text_renderer_handle text_renderer_new(uint8_t* font, size_t font_length)
{
    text_renderer_t* tr = malloc(sizeof(text_renderer_t));
    if(!tr)
        return NULL;
    memset(tr, 0, sizeof(text_renderer_t));
    tr->sft.font = sft_loadmem(font, font_length);
    if(!tr->sft.font)
    {
        free(tr);
        return NULL;
    }
    return tr;
}

void text_renderer_free(text_renderer_handle handle)
{
    text_renderer_t* tr = (text_renderer_t*)handle;
    if(tr)
    {
        if(tr->sft.font)
            sft_freefont(tr->sft.font);
        free(tr);
    }
}

static tr_char_t* tr_render_one(SFT* sft, uint32_t cp)
{
    SFT_Glyph gid;
    if(sft_lookup(sft, cp, &gid) < 0)
        return NULL;
    SFT_GMetrics mtx;
    if(sft_gmetrics(sft, gid, &mtx) < 0)
        return NULL;
    SFT_Image img = {
        .width = mtx.minWidth,
        .height = mtx.minHeight
    };
    uint8_t pixels[img.width * img.height];
    img.pixels = pixels;
    if(sft_render(sft, gid, img) < 0)
        return NULL;
    /** quantize the img */
    tr_char_t* c = malloc(sizeof(tr_char_t) + (img.width * img.height + 7)/8);
    if(!c)
        return NULL;
    c->x_offset = mtx.leftSideBearing;
    c->y_offset = mtx.yOffset;
    c->advance = mtx.advanceWidth;
    c->width = img.width;
    c->height = img.height;
    memset(c->data, 0, (img.width * img.height + 7)/8);
    for(int i=0;i<img.width*img.height;i++)
    {
        /** Keep only the MSB. And reverse the output. */
        c->data[i/8] |= (!(pixels[i] >> 7)) << (i%8);
    }
    return c;
}

static void tr_char_free(tr_char_t* c)
{
    if(c)
        free(c);
}

static void tr_draw_one(duotone_fb_t* fb, tr_char_t* c, int x, int y)
{
    for(int i=0;i<c->height;i++)
    {
        if(y + i + c->y_offset < 0 || y + i + c->y_offset >= fb->height)
            continue;
        for(int j=0;j<c->width;j++)
        {
            if(x + j + c->x_offset < 0 || x + j + c->x_offset >= fb->width)
                continue;
            int pos = i*c->width+j;
            if(!(c->data[pos/8] & (1 << (pos%8))))
            {
                /** The backgroud is white. So only draw the black dots. */
                duotone_fb_set_pixel(fb, x + j + c->x_offset, y + i + c->y_offset, 0);
            }
        }
    }
}

static int utf8_to_utf32_get_next(const uint8_t *utf8, uint32_t* utf32)
{
	uint32_t c;
    if (!(*utf8 & 0x80U)) 
    {
        *utf32 = *utf8;
        return 1;
    } 
    else if ((*utf8 & 0xe0U) == 0xc0U) 
    {
        c = (*utf8++ & 0x1fU) << 6;
        if ((*utf8 & 0xc0U) != 0x80U) 
            return -1;
        *utf32 = c + (*utf8 & 0x3fU);
        return 2;
    } 
    else if ((*utf8 & 0xf0U) == 0xe0U) 
    {
        c = (*utf8++ & 0x0fU) << 12;
        if ((*utf8 & 0xc0U) != 0x80U) 
            return -1;
        c += (*utf8++ & 0x3fU) << 6;
        if ((*utf8 & 0xc0U) != 0x80U) 
            return -1;
        *utf32 = c + (*utf8++ & 0x3fU);
        return 3;
    } 
    else if ((*utf8 & 0xf8U) == 0xf0U) 
    {
        c = (*utf8++ & 0x07U) << 18;
        if ((*utf8 & 0xc0U) != 0x80U) 
            return -1;
        c += (*utf8++ & 0x3fU) << 12;
        if ((*utf8 & 0xc0U) != 0x80U) 
            return -1;
        c += (*utf8++ & 0x3fU) << 6;
        if ((*utf8 & 0xc0U) != 0x80U) 
            return -1;
        c += (*utf8 & 0x3fU);
        if ((c & 0xFFFFF800U) == 0xD800U) 
            return -1;
        *utf32 = c;
        return 4;
    }
    else 
    {
        return -1;
    }   
}

char* render_text_in_box(text_renderer_handle handle, const char* text, int font_size, float line_space, int x, int y, int width, int height, duotone_fb_t* fb)
{
    text_renderer_t* tr = (text_renderer_t*)handle;
    if(!tr)
        return NULL;
    if(!text)
        return NULL;
    if(font_size <= 0)
        return NULL;
    if(width <= 0 || height <= 0)
        return NULL;
    if(!fb)
        return NULL;
    if(x<0 || y<0 || x+width > fb->width || y+height > fb->height)
        return NULL;
    
    tr->sft.xScale = font_size;
    tr->sft.yScale = font_size;
    tr->sft.flags = SFT_DOWNWARD_Y;
    tr->sft.xOffset = 0;
    tr->sft.yOffset = 0;

    SFT_LMetrics lmtx;
    if(sft_lmetrics(&tr->sft, &lmtx) != 0)
        return NULL;

    int x_offset = x;
    /** First line's offset */
    int y_offset = y + lmtx.lineGap + lmtx.ascender;
    char* pos = (char*)text;
    while((*pos) && (y_offset < y + height))
    {
        uint32_t utf32;
        int n = utf8_to_utf32_get_next((const uint8_t*)pos, &utf32);
        if(n < 0)
        {
            /** something went wrong with the text, skip this one. */
            pos++;
            continue;
        }
        if(*pos == '\n')
        {
            /** Advance to the next line */
            pos += n;
            x_offset = x;
            y_offset += (lmtx.descender + lmtx.lineGap + lmtx.ascender) * line_space;
            continue;
        }
        tr_char_t* c = tr_render_one(&tr->sft, utf32);
        if(!c)
        {
            /** Maybe this font can not render this char. Render a space instead */
            c = tr_render_one(&tr->sft, ' ');
            if(!c)
                return NULL;
        }
        if(x_offset + c->advance > x + width)
        {
            /** Advance to the next line without consuming this char except for space. */
            if(utf32 == ' ')
                pos += n;
            x_offset = x;
            y_offset += (lmtx.descender + lmtx.lineGap + lmtx.ascender) * line_space;
            tr_char_free(c);
            continue;
        }
        tr_draw_one(fb, c, x_offset, y_offset);
        x_offset += c->advance;
        tr_char_free(c);
        pos += n;
    }
    return pos;
}   
