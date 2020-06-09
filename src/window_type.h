#ifndef WINDOW_TYPE_H
#define WINDOW_TYPE_H
#include "window.h"

struct pixel_t {
    unsigned int c;
    unsigned char color;
    unsigned char bgcolor;
    unsigned char high;
};
struct window_t {
    struct window_t *up;
    struct window_t *down;
    struct window_t *parent;
    struct window_t *childtop;
    struct window_t *childbottom;
    struct window_t *focus_child;
    struct window_t *modal_child;

    void *user_data;

    int top;
    int left;
    int width;
    int height;

    scroll_t *vert_scroll;
    scroll_t *horz_scroll;

    int style;
    int can_focus;
    int visible;
    char text[256];
    int modal_result;
    void (*pf_modal_finish)(window_t *w);

    void (*pf_draw_handle)(window_t *w);

    pixel_t *pixel;

    void (*pf_show_cur)(window_t *w);

    void (*pf_internal_mouse_down)(window_t *w, int key, int x, int y, int ctrl);
    void (*pf_internal_mouse_up)(window_t *w, int key, int x, int y, int ctrl);
    void (*pf_internal_mouse_drag)(window_t *w, int key, int x, int y, int ctrl);
    void (*pf_internal_mouse_wheel)(window_t *w, int updown, int x, int y, int ctrl);
    void (*pf_internal_key)(window_t *w, unsigned int key);

    void (*pf_internal_get_focus)(window_t *w);
    int (*pf_internal_lose_focus)(window_t *w);

    void (*pf_on_resize)(window_t *w, int width, int height);
    void (*pf_on_click)(window_t *w);
    void (*pf_on_mouse_down)(window_t *w, int key, int x, int y, int ctrl);
    void (*pf_on_mouse_up)(window_t *w, int key, int x, int y, int ctrl);
    void (*pf_on_mouse_drag)(window_t *w, int key, int x, int y, int ctrl);
    void (*pf_on_mouse_wheel)(window_t *w, int updown, int x, int y, int ctrl);
    void (*pf_on_key)(window_t *w, unsigned int *key);
    
    void (*pf_on_get_focus)(window_t *w);
    int (*pf_on_lose_focus)(window_t *w);
    void (*pf_on_change)(window_t *w);

    int (*pf_close_window)(window_t *w);
};

#define DRAG_NONE 0
#define DRAG_MOVE 1
#define DRAG_SIZE_LEFT 2
#define DRAG_SIZE_RIGHT 3
#define DRAG_SIZE_DOWN 4
#define DRAG_SIZE_LEFT_DOWN 5
#define DRAG_SIZE_RIGHT_DOWN 6

extern window_t *drag;
extern int drag_type;
extern int drag_ox;
extern int drag_oy;
extern int drag_owidth;
extern int drag_oheight;

void get_win_abs_pos(window_t *w, int *left, int *top);
window_t *get_root_win(window_t *w);
void win_printf(window_t *w, int x, int y, int c, int b, int hi, char *fmt,...);
void show_window_inner(window_t *w);
#endif
