#include <png.h>
#include <stdlib.h>
#include "duotone_frame_buffer.h"

int print_duotone_fb_to_png(const char* filename, duotone_fb_t* fb)
{
    png_bytep row;
    int x, y;
    png_structp png_ptr;
    png_infop info_ptr;
    png_byte color_type = PNG_COLOR_TYPE_GRAY;
    png_byte bit_depth = 8;

    FILE *fp = fopen(filename, "wb");
    if (!fp) {
        printf("[print_duotone_fb_to_png] File %s could not be opened for writing", filename);
        goto error;
    }

    png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);

    if (!png_ptr) {
        printf("[print_duotone_fb_to_png] png_create_write_struct failed");
        goto error;
    }

    info_ptr = png_create_info_struct(png_ptr);
    if (!info_ptr) {
        printf("[print_duotone_fb_to_png] png_create_info_struct failed");
        goto error;
    }

    if (setjmp(png_jmpbuf(png_ptr))) {
        printf("[print_duotone_fb_to_png] Error during init_io");
        goto error;
    }

    png_init_io(png_ptr, fp);

    if (setjmp(png_jmpbuf(png_ptr))) {
        printf("[print_duotone_fb_to_png] Error during writing header");
        goto error;
    }

    png_set_IHDR(png_ptr, info_ptr, fb->width, fb->height,
                 bit_depth, color_type, PNG_INTERLACE_NONE,
                 PNG_COMPRESSION_TYPE_BASE, PNG_FILTER_TYPE_BASE);

    png_write_info(png_ptr, info_ptr);

    if (setjmp(png_jmpbuf(png_ptr))) {
        printf("[print_duotone_fb_to_png] Error during writing bytes");
        goto error;
    }

    row = (png_bytep) malloc(1 * fb->width * sizeof(png_byte));

    for (y = 0; y < fb->height; y++) {
        for (x = 0; x < fb->width; x++) {
            row[x] = duotone_fb_get_pixel(fb, x, y) * 255;
        }
        png_write_row(png_ptr, row);
    }

    png_write_end(png_ptr, NULL);

    png_destroy_write_struct(&png_ptr, &info_ptr);
    fclose(fp);
    if(row)
        free(row);
    return 0;
error:
    if(row)
        free(row);
    if(png_ptr)
        png_destroy_write_struct(&png_ptr, &info_ptr);
    if(fp)
        fclose(fp);
    return -1;
}
