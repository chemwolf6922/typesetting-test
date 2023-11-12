#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <png.h>
#include <assert.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include "duotone_frame_buffer.h"
#include "print_duotone_fb.h"
#include "text_renderer.h"

#define WIDTH 480
#define HEIGHT 800

static uint8_t buffer[duotone_fb_get_buffer_size(WIDTH, HEIGHT)] = {0};

static duotone_fb_t fb = {
    .width = WIDTH,
    .height = HEIGHT,
    .buffer = buffer
};

static uint8_t* map_font(const char* font_filename, size_t* font_length)
{
    /** map font to memory with mmap */
    int fd = open(font_filename, O_RDONLY);
    if(fd < 0)
        return NULL;
    struct stat st;
    if(fstat(fd, &st) < 0)
    {
        close(fd);
        return NULL;
    }
    *font_length = st.st_size;
    uint8_t* font = mmap(NULL, st.st_size, PROT_READ, MAP_PRIVATE, fd, 0);
    if(font == MAP_FAILED)
    {
        close(fd);
        return NULL;
    }
    close(fd);
    return font;
}

static void unmap_font(uint8_t* font)
{
    munmap(font, 0);
}

const char* test_text = "先帝创业未半而中道崩殂，今天下三分，益州疲弊，此诚危急存亡之秋也。"\
"然侍卫之臣不懈于内，忠志之士忘身于外者，盖追先帝之殊遇，欲报之于陛下也。诚宜开张圣听，以光先帝"\
"遗德，恢弘志士之气，不宜妄自菲薄，引喻失义，以塞忠谏之路也。\n宫中府中，俱为一体；陟罚臧否，不"\
"宜异同：若有作奸犯科及为忠善者，宜付有司论其刑赏，以昭陛下平明之理；不宜偏私，使内外异法也。\n侍"\
"中、侍郎郭攸之、费祎、董允等，此皆良实，志虑忠纯，是以先帝简拔以遗陛下：愚以为宫中之事，事无大小"\
"，悉以咨之，然后施行，必能裨补阙漏，有所广益。\n将军向宠，性行淑均，晓畅军事，试用于昔日，先帝称之"\
"曰“能”，是以众议举宠为督：愚以为营中之事，悉以咨之，必能使行阵和睦，优劣得所。\n亲贤臣，远小人，此"\
"先汉所以兴隆也；亲小人，远贤臣，此后汉所以倾颓也。先帝在时，每与臣论此事，未尝不叹息痛恨于桓、灵"\
"也。侍中、尚书、长史、参军，此悉贞良死节之臣，愿陛下亲之信之，则汉室之隆，可计日而待也。\n臣本布衣"\
"，躬耕于南阳，苟全性命于乱世，不求闻达于诸侯。先帝不以臣卑鄙，猥自枉屈，三顾臣于草庐之中，咨臣以"\
"当世之事，由是感激，遂许先帝以驱驰。后值倾覆，受任于败军之际，奉命于危难之间：尔来二十有一年矣。"\
"先帝知臣谨慎，故临崩寄臣以大事也。受命以来，夙夜忧叹，恐托付不效，以伤先帝之明；故五月渡泸，深入"\
"不毛。今南方已定，兵甲已足，当奖率三军，北定中原，庶竭驽钝，攘除奸凶，兴复汉室，还于旧都。此臣所"\
"以报先帝而忠陛下之职分也。至于斟酌损益，进尽忠言，则攸之、祎、允之任也。\n愿陛下托臣以讨贼兴复之效"\
"，不效，则治臣之罪，以告先帝之灵。若无兴德之言，则责攸之、祎、允等之慢，以彰其咎；陛下亦宜自谋，"\
"以咨诹善道，察纳雅言，深追先帝遗诏。臣不胜受恩感激。今当远离，临表涕零，不知所言。";

int main(int argc, char const *argv[])
{
    (void)argc;
    (void)argv;
    duotone_fb_clear(&fb);
    size_t font_length;
    uint8_t* font = map_font("./fonts/test_font.ttf", &font_length);
    assert(font);

    text_renderer_handle tr = text_renderer_new(font, font_length);
    assert(tr);

    /** draw line over the edges */
    for(int i=0; i<WIDTH; i++)
    {
        duotone_fb_set_pixel(&fb, i, 0, 0);
        duotone_fb_set_pixel(&fb, i, HEIGHT-1, 0);
    }
    for(int i=0; i<HEIGHT; i++)
    {
        duotone_fb_set_pixel(&fb, 0, i, 0);
        duotone_fb_set_pixel(&fb, WIDTH-1, i, 0);
    }

    char* end = render_text_in_box(tr, test_text, 24, 2, 20, 20, WIDTH-40, HEIGHT-40, &fb);
    assert(end);

    assert(print_duotone_fb_to_png("test.png", &fb)==0);

    text_renderer_free(tr);
    unmap_font(font);
    
    return 0;
}

