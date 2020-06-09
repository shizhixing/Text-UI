#ifndef CURSOR_H
#define CURSOR_H
#define CLEAR() printf("\033[2J")
#define MOVEUP(x) printf("\033[%dA", (x))
#define MOVEDOWN(x) printf("\033[%dB", (x))
#define MOVELEFT(y) printf("\033[%dD", (y))
#define MOVERIGHT(y) printf("\033[%dC",(y))
#define MOVETO(x,y) printf("\033[%d;%dH", (y + 1), (x + 1))
#define RESET_CURSOR() printf("\033[H")
#define HIDE_CURSOR() printf("\033[?25l")
#define SHOW_CURSOR() printf("\033[?25h")
#define DING() putchar(7)

#define BLACK 30
#define RED 31
#define GREEN 32
#define YELLOW 33
#define BLUE 34
#define PURPLE 35
#define DARK_BREEN 36
#define WHITE 37

#define UNDER_LINE 4
#define FLASH 5
#define REVERSE 7

extern int screen_width;
extern int screen_height;
#define SCREEN_WIDTH screen_width
#define SCREEN_HEIGHT screen_height

void set_color(int color, int bgcolor, int high);
#endif
