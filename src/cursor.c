#include <stdio.h>

#define SET_COLOR_FONT(c) printf("\033[%dm", (c))
#define BG_COLOR(c) (c + 10)
#define HIGHT_LIGHT() printf("\033[1m")
#define UN_HIGHT_LIGHT() printf("\033[0m")

static int cur_color = 0;
static int cur_bgcolor = 0;
static int cur_high = 0;

void set_color(int color, int bgcolor, int high)
{
    if (high != cur_high) {
        if (high) HIGHT_LIGHT(); else UN_HIGHT_LIGHT();
    }
    if (color != cur_color || high != cur_high) {
        SET_COLOR_FONT(color);
        cur_color = color;
    }
    if (bgcolor != cur_bgcolor || high != cur_high) {
        SET_COLOR_FONT(BG_COLOR(bgcolor));
        cur_bgcolor = bgcolor;
    }
    if (high != cur_high) {
        cur_high = high;
    }
}
