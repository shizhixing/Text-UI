#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <wchar.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>
#include "def.h"
#include "window_type.h"
#include "window.h"
#include "cursor.h"
#include "input.h"

int usleep(int usec);

int window_t_size()
{
    return sizeof(window_t);
}

static pixel_t *get_pixel_p(window_t *w, int x, int y)
{
    int i;
    if (w) {
        if (x < 0 || x >= w->width || y < 0 || y >= w->height) return 0;
        i = y * w->width + x;
        return &w->pixel[i];
    }
    return 0;
}

typedef struct modal_lock_t {
    pthread_cond_t cond;
    pthread_mutex_t mutex;
} modal_lock_t;

typedef struct thread_context_t {
    int terminal;
} thread_context_t;

thread_context_t *g_tctxt = NULL;
static window_t *desk = NULL;
static window_t *foreground_win = NULL;

window_t *drag = NULL;
int drag_type;
int drag_ox;
int drag_oy;
int drag_owidth;
int drag_oheight;

static void draw_move_win(window_t *w);

void set_win_user_data(window_t *w, void *user_data)
{
    w->user_data = user_data;
}

void *get_win_user_data(window_t *w)
{
    return w->user_data;
}

void draw_desk(window_t *w)
{
    int size = w->width * w->height;
    int i;
    for (i = 0;i < size;i++) {
        w->pixel[i].c = '_';
        w->pixel[i].color = WHITE;
        w->pixel[i].bgcolor = BLACK;
    }
}

void internal_key_desk(window_t *w, unsigned int key)
{
    switch (key) {
        case KEY_F5:
            refresh_screen();
            break;
        default:
            break;
    }
}

void create_desk()
{
    desk = create_window(NULL, NULL, sizeof(window_t));
    strcpy(desk->text, "Desk");
    set_win_size(desk, 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
    desk->pf_internal_key = internal_key_desk;
    draw_desk(desk);
//    desk->can_focus = TRUE;
}

#define UNICODE(x, y) (0xe20000 + (x) + (y))
#define U_TABLE(x) UNICODE(0x6f80, x)
#define U_BLOCK(x) UNICODE(0x7100, x)
void draw_table_rectangle(window_t *w, int left, int top, int right, int bottom, int color, int bgcolor)
{
    draw_table_line(w, left, top, right, top, color, bgcolor);
    draw_table_line(w, left, top, left, bottom, color, bgcolor);
    draw_table_line(w, right, top, right, bottom, color, bgcolor);
    draw_table_line(w, left, bottom, right, bottom, color, bgcolor);
}
void draw_table_line(window_t *w, int ox, int oy, int dx, int dy, int color, int bgcolor)
{
#define IS_U_TABLE8_LINE(x) ((x) == U_TABLE(0x250c) || (x) == U_TABLE(0x2500)\
        || (x) == U_TABLE(0x2510) || (x) == U_TABLE(0x2502)\
        || (x) == U_TABLE(0x2514) || (x) == U_TABLE(0x2518)\
        || (x) == U_TABLE(0x251c) || (x) == U_TABLE(0x252c) || (x) == U_TABLE(0x253c)\
        || (x) == U_TABLE(0x2524) || (x) == U_TABLE(0x2534))
    int i, s, e;
    pixel_t *pixel, *p, *q;
    int t1, t2;
    if (ox == dx) {/* vertical */
        if (dy > oy) {
            s = oy;
            e = dy;
        } else {
            s = dy;
            e = oy;
        }
        if (ox < 0 || ox >= w->width) return;
        if (s < 0) s = 0;
        if (e >= w->height) e = w->height - 1;
        for (i = s;i <= e;i++) {
            pixel = &w->pixel[i * w->width + ox];
            if (!IS_U_TABLE8_LINE(pixel->c)) { 
                pixel->c = U_TABLE(0x2502);
            } else if (pixel->c == U_TABLE(0x2502) || pixel->c == U_TABLE(0x251c) 
                    || pixel->c == U_TABLE(0x2524) || pixel->c == U_TABLE(0x253c)) {
                continue;
            } else if ((i == s) &&
                    (pixel->c == U_TABLE(0x250c) || pixel->c == U_TABLE(0x2510) || pixel->c == U_TABLE(0x252c))) {
                continue;
            } else if ((i == e) &&
                    (pixel->c == U_TABLE(0x2514) || pixel->c == U_TABLE(0x2518) || pixel->c == U_TABLE(0x2534))) {
                continue;
            } else if (pixel->c == U_TABLE(0x250c) || pixel->c == U_TABLE(0x2514)) {
                pixel->c = U_TABLE(0x251c);
            } else if (pixel->c == U_TABLE(0x2510) || pixel->c == U_TABLE(0x2518)) {
                pixel->c = U_TABLE(0x2524);
            } else if (pixel->c == U_TABLE(0x252c) || pixel->c == U_TABLE(0x2534)) {
                pixel->c = U_TABLE(0x253c);
            } else {
                if (ox > 0) p = &w->pixel[i * w->width + ox - 1];
                else p = 0;
                if (ox < w->width - 1) q = &w->pixel[i * w->width + ox + 1];
                else q = 0;
                if (p && IS_U_TABLE8_LINE(p->c)) t1 = 1;else t1 = 0;
                if (q && IS_U_TABLE8_LINE(q->c)) t2 = 1;else t2 = 0;
                if (t1 && t2) {
                    if (i == s) pixel->c = U_TABLE(0x252c);
                    else if (i == e) pixel->c = U_TABLE(0x2534);
                    else pixel->c = U_TABLE(0x253c);
                } else if (t1) {
                    if (i == s) pixel->c = U_TABLE(0x2510);
                    else if (i == e) pixel->c = U_TABLE(0x2518);
                    else pixel->c = U_TABLE(0x2524);
                } else if (t2) {
                    if (i == s) pixel->c = U_TABLE(0x250c);
                    else if (i == e) pixel->c = U_TABLE(0x2514);
                    else  pixel->c = U_TABLE(0x251c);
                } else {
                     pixel->c = U_TABLE(0x2502);
                }
            }
            pixel->color = color;
            pixel->bgcolor = bgcolor;
        }
    } else if (oy == dy) {/* horizontal */
        if (dx > ox) {
            s = ox;
            e = dx;
        } else {
            s = dx;
            e = ox;
        }
        if (oy < 0 || oy >= w->height) return;
        if (s < 0) s = 0;
        if (e >= w->width) e = w->width - 1;
        for (i = s;i <= e;i++) {
            pixel = &w->pixel[oy * w->width + i];
            if (!IS_U_TABLE8_LINE(pixel->c)) {
                pixel->c = U_TABLE(0x2500);
            } else if (pixel->c == U_TABLE(0x2500) || pixel->c == U_TABLE(0x252c)
                    || pixel->c == U_TABLE(0x2534) || pixel->c == U_TABLE(0x253c)) {
            } else if ((i == s) &&
                    (pixel->c == U_TABLE(0x250c) || pixel->c == U_TABLE(0x2514) || pixel->c == U_TABLE(0x251c))) {
                continue;
            } else if ((i == e) &&
                    (pixel->c == U_TABLE(0x2510) || pixel->c == U_TABLE(0x2518) || pixel->c == U_TABLE(0x2524))) {
                continue;
            } else if (pixel->c == U_TABLE(0x2514) || pixel->c == U_TABLE(0x2518)) {
                pixel->c = U_TABLE(0x2534);
            } else if (pixel->c == U_TABLE(0x250c) || pixel->c == U_TABLE(0x2510)) {
                pixel->c = U_TABLE(0x252c);
            } else if (pixel->c == U_TABLE(0x251c) || pixel->c == U_TABLE(0x2524)) {
                pixel->c = U_TABLE(0x253c);
            } else {
                if (oy > 0) p = &w->pixel[(oy - 1) * w->width + i];
                else p = 0;
                if (oy < w->height - 1) q = &w->pixel[(oy + 1) * w->width + i];
                else q = 0;
                if (p && IS_U_TABLE8_LINE(p->c)) t1 = 1;else t1 = 0;
                if (q && IS_U_TABLE8_LINE(q->c)) t2 = 1;else t2 = 0;
                if (t1 && t2) {
                    if (i == s) pixel->c = U_TABLE(0x251c);
                    else if (i == e) pixel->c = U_TABLE(0x2524);
                    else pixel->c = U_TABLE(0x253c);
                } else if (t1) {
                    if (i == s) pixel->c = U_TABLE(0x2514);
                    else if (i == e) pixel->c = U_TABLE(0x2518);
                    else pixel->c = U_TABLE(0x2534);
                } else if (t2) {
                    if (i == s) pixel->c = U_TABLE(0x250c);
                    else if (i == e) pixel->c = U_TABLE(0x2518);
                    else pixel->c = U_TABLE(0x252c);
                } else {
                    pixel->c = U_TABLE(0x2500);
                }
            }
            pixel->color = color;
            pixel->bgcolor = bgcolor;
        }
    }
}

void draw_normal_win(window_t *w)
{
    window_t *p = get_root_win(w);
    pixel_t *pixel;
    int size = w->width * w->height;
    int i;
    for (i = 0;i < size;i++) {
        pixel = &w->pixel[i];
        pixel->c = ' ';
        pixel->color = BLACK;
        pixel->bgcolor = BLACK;
    }
    if (w->style == WIN_STYLE_SIZEABLE) {
        draw_table_rectangle(w, 0, 0, w->width - 1, w->height - 1, WHITE, BLACK);
        for (i = 0;i < w->width;i++) {
            pixel = get_pixel_p(w, i, 0);
            pixel->c = ' ';
            if (foreground_win == p) {
                pixel->color = WHITE;
                pixel->bgcolor = BLUE;
            } else {
                 pixel->color = BLACK;
                 pixel->bgcolor = WHITE;
            }
        }
        pixel = get_pixel_p(w, 0, 0);
        pixel->c = U_BLOCK(0x258C);
        if (foreground_win == w) {
            pixel->color = BLACK;
            pixel->bgcolor = BLUE;
        } else {
            pixel->color = BLACK;
            pixel->bgcolor = WHITE;
        }
        pixel = get_pixel_p(w, w->width - 1, 0);
        pixel->c = U_BLOCK(0x2590);
        if (foreground_win == w) {
            pixel->color = BLACK;
            pixel->bgcolor = BLUE;
        } else {
            pixel->color = BLACK;
            pixel->bgcolor = WHITE;
        }
        for (i = 1;i < w->width - 1 && w->text[i - 1];i++) {
            pixel = get_pixel_p(w, i, 0);
            pixel->c = w->text[i - 1];
        }
    }
}

void init_scr()
{
    printf("\033[?1049h");//allscreen
    printf("\033[?1002h");//enable mouse
    HIDE_CURSOR();
    CLEAR();
    create_desk();
}

void end_scr()
{
    SHOW_CURSOR();
    printf("\033[?1002l");//disable mouse
    printf("\033[?1049l");//exit screen
}

static window_t *get_window_inner(window_t *w, int x, int y)
{
    window_t *p, *q;
    pixel_t *pixel;
    int in_w = 0;
    if (w->visible) {
        if (w->style == WIN_STYLE_SIZEABLE) {
            if (x >= 1 && x < w->width - 1 &&
                y >= 1 && y < w->height - 1) {
                in_w = 1;
            }
        } else {
            if (x >= 0 && x < w->width && y >= 0 && y < w->height) {
                in_w = 1;
            }
        }
    }
    for (p = w->childtop;p;p = p->down) {
        q =  get_window_inner(p, x - p->left, y - p->top);
        if (q) {
            if (q->style != WIN_STYLE_SIZEABLE && !in_w && q->parent == w) continue;
            return q;
        }
    }
    if (!w->visible) return NULL;
    pixel = get_pixel_p(w, x, y);
    if (pixel && pixel->c && x >= 0 && x < w->width && y >= 0 && y < w->height) {
        return w;
    }
    return NULL;
}

window_t *get_window(int x, int y)
{
    window_t *p = get_window_inner(desk, x, y);
    return p;
}

window_t *get_parent(window_t *w)
{
    return w->parent;
}

window_t *create_window(window_t *parent, void *draw_handle, int total_size)
{
    window_t *w;
    if (total_size < sizeof(window_t)) return NULL;
    w = (window_t *)MALLOC(total_size);
    memset(w, 0x00, total_size);
    w->visible = TRUE;
    w->pf_draw_handle = draw_handle;
    if (!parent) parent = desk;
    w->parent = parent;
    if (parent) {
        if (!parent->childtop) {
            parent->childtop = w;
            parent->childbottom = w;
        } else {
            parent->childtop->up = w;
            w->down = parent->childtop;
            parent->childtop = w;
            w->left = w->down->left + 2;
            w->top = w->down->top + 2;
        }
    }
    set_win_size(w, w->left, w->top, 10, 10);

    return w;
}

void internal_mouse_down_normal(window_t *w, int key, int x, int y, int ctrl)
{
    window_t *p = get_root_win(w);
    if (p->modal_child) {
        /* Find the to modal child and flash it.  */
        return;
    }
    if (!ctrl) {
        if (y == 0) {
            drag_type = DRAG_MOVE;
        } else if (x == 0) {
            if (y == w->height - 1) {
                drag_type = DRAG_SIZE_LEFT_DOWN;
            } else {
                drag_type = DRAG_SIZE_LEFT;
            }   
        } else if (x == w->width - 1) {
            if (y == w->height - 1) {
                drag_type = DRAG_SIZE_RIGHT_DOWN;
            } else {
                drag_type = DRAG_SIZE_RIGHT;
            }   
        } else if (y == w->height - 1) {
            drag_type = DRAG_SIZE_DOWN;
        } else {
            drag_type = DRAG_NONE;
        }

        if (drag_type == DRAG_NONE) {
            if (w->pf_on_mouse_down) w->pf_on_mouse_down(w, key, x, y, ctrl);
        } else {
            p = create_normal_window(w);
            p->visible = TRUE;
            memset(p->pixel, 0x00, sizeof(pixel_t) * p->width * p->height);
            p->pf_draw_handle = draw_move_win; 
            set_win_size(p, 0, 0, w->width, w->height);
            p->pf_draw_handle(p);
            drag = p;
            drag_ox = x;
            drag_oy = y;
            drag_owidth =  w->width;
            drag_oheight = w->height;
        }
    }
}

void internal_mouse_up_normal(window_t *w, int key, int x, int y, int ctrl)
{
    if (w->pf_draw_handle != draw_move_win) return;
    set_win_size(w->parent, w->parent->left + w->left, w->parent->top + w->top,
            w->width, w->height);
    destroy_window(drag);
    update_screen();
}

void internal_mouse_drag_normal(window_t *w, int key, int x, int y, int ctrl)
{
    int new_left;
    int new_width;
    int new_height;
    new_left = 0;
    new_width = drag_owidth;
    new_height = drag_oheight;

    if (drag_type == DRAG_MOVE) {
        w->left = w->left + x - drag_ox;
        w->top = w->top + y - drag_oy;
        set_win_size(w, w->left, w->top, w->width, w->height);
    } else {
        if (drag_type == DRAG_SIZE_LEFT || drag_type == DRAG_SIZE_LEFT_DOWN) {
            new_left = w->left + x - drag_ox;
            new_width = w->width - x + drag_ox;
        } else if (drag_type == DRAG_SIZE_RIGHT || drag_type == DRAG_SIZE_RIGHT_DOWN) {
            new_width = drag_owidth + x - drag_ox;
        }   
        if (drag_type == DRAG_SIZE_LEFT_DOWN || drag_type == DRAG_SIZE_RIGHT_DOWN || drag_type == DRAG_SIZE_DOWN) {
            new_height = drag_oheight + y - drag_oy;
        }   
        if (new_width < 10) new_width = 10; 
        if (new_height < 2) new_height = 2;  
        set_win_size(w, new_left, 0, new_width, new_height);
    }   
}
 
window_t *create_normal_window(window_t *parent)
{
    window_t *w = create_window(parent, draw_normal_win, sizeof(window_t));
    w->visible = FALSE;
    set_win_style(w, WIN_STYLE_SIZEABLE);
    w->pf_internal_mouse_down = internal_mouse_down_normal;
    w->pf_internal_mouse_up = internal_mouse_up_normal;
    w->pf_internal_mouse_drag = internal_mouse_drag_normal;
    redraw_window(w);
    return w;
}

void redraw_window(window_t *w)
{
    if (w->pf_draw_handle) w->pf_draw_handle(w);
}

void destroy_window(window_t *w)
{
    window_t *p = w->parent;
    window_t *q;
    if (w->up) {
        w->up->down = w->down;
    } else {
        p->childtop = w->down;
    }
    if (w->down) {
        w->down->up = w->up;
    } else {
        p->childbottom = w->up;
    }
    if (w->pixel) FREE(w->pixel);
    for (p = w->childtop;p;p = q) {
        q = p->down;
        destroy_window(p);
    }
    if (foreground_win == w) foreground_win = NULL;
    FREE(w);
}

void set_win_size(window_t *w, int left, int top, int width, int height)
{
    int size = sizeof(pixel_t) * width * height;
    if (w->left == left && w->top == top && w->width == width && w->height == height) return;
    w->left = left;
    w->top = top;
    w->width = width;
    w->height = height;
    if (w->pixel) FREE(w->pixel);
    w->pixel = (pixel_t *)MALLOC(size);
    memset(w->pixel, 0x00, size);
    redraw_window(w);
    if (w->pf_on_resize) w->pf_on_resize(w, width, height);
}

int get_win_size(window_t *w, int *left, int *top, int *width, int *height)
{
    if (!w) return NO;
    *left = w->left;
    *top = w->top;
    *width = w->width;
    *height = w->height;
    return OK;
}

void bring_win_top(window_t *w)
{
    window_t *p;
    if (!w) w = desk->childbottom;
    if (w->parent && w->parent != desk) bring_win_top(w->parent);
    if (w->up) {
        w->up->down = w->down;
    } else {
        if (foreground_win != w) {
            p = foreground_win;
            foreground_win = w;
            while (p && p != desk) {
                redraw_window(p);
                show_window_inner(p);
                p = p->parent;
            }
            redraw_window(w);
            show_window_inner(w);
        }
        goto Exit;
    }
    if (w->down) {
        w->down->up = w->up;
    } else {
        w->parent->childbottom = w->up;
    }
    w->up = 0;
    w->down = w->parent->childtop;
    w->parent->childtop->up = w;
    w->parent->childtop = w;
    if (foreground_win != w) {
        p = foreground_win;
        foreground_win = w;
        while (p && p != desk) {
            redraw_window(p);
            show_window_inner(p);
            p = p->parent;
        }
    }
    redraw_window(w);
    show_window_inner(w);
Exit:
    return;
}

int set_focus_window(window_t *w)
{
    window_t *p = get_root_win(w);
    window_t *q;
    if (p->focus_child == w) return OK;
    if (w->can_focus) {
        if (p->focus_child && p->focus_child->pf_internal_lose_focus) {
            if (OK != p->focus_child->pf_internal_lose_focus(p->focus_child)) return NO;
        }
        q = p->focus_child;
        p->focus_child = w;
        if (q) {
            redraw_window(q);
            show_window_inner(q);
        }
        if (w->pf_internal_get_focus) {
            w->pf_internal_get_focus(w);
        }
        redraw_window(w);
        show_window_inner(w);
        return OK;
    } else {
        return NO;
    }
}

int is_window_focused(window_t *w)
{
    window_t *p = get_root_win(w);
    return (p->focus_child == w);
}

pixel_t get_pixel(window_t *w, int x, int y)
{
    pixel_t e = {0, };
    int i;
    if (w) {
        if (x < 0 || x >= w->width || y < 0 || y >= w->height) return e;
        i = y * w->width + x;
        return w->pixel[i];
    }
    return e;
}

void get_win_abs_pos(window_t *w, int *left, int *top)
{
    int ox = 0;
    int oy = 0;
    for (;w;w = w->parent) {
        ox += w->left;
        oy += w->top;
    }
    *left = ox;
    *top = oy;
}
//char aa = 32;
void show_window_inner(window_t *w)
{
    window_t *p;
    int i;
    int x, y;
    int cx = 0, cy = 0;
    int ox = 0, oy = 0;
    pixel_t bgpixel;
    pixel_t *pixel;
    pixel_t *desk_pixel;
    unsigned char c;

    if (!w) return;
    HIDE_CURSOR();
    for (p = w->childtop;p;p = p->down) {
        show_window_inner(p);
    }
    if (!w->visible) return;
    get_win_abs_pos(w, &ox, &oy);
    cx = -1;
    cy = -1;
    memset(&bgpixel, 0x00, sizeof(bgpixel));
    bgpixel.c = ' ';
    bgpixel.color = WHITE;
    bgpixel.bgcolor = BLACK;
    if (oy >= 0) y = 0;else y = 0 - oy;
    for (;y < w->height && y + oy < SCREEN_HEIGHT;y++) {
        if (ox >= 0) x = 0; else x = 0 - ox;        
        for (;x < w->width && x + ox < SCREEN_WIDTH;x++) {
            p = get_window(ox + x, oy + y);
            if (p == w) {/* can be seen */
                if (w != desk)  pixel = get_pixel_p(w, x, y);
                else pixel = &bgpixel;
                desk_pixel = get_pixel_p(desk, x + ox, y + oy);
                if (desk_pixel && pixel->c && memcmp(pixel, desk_pixel, sizeof(pixel_t))) {
                    memcpy(desk_pixel, pixel, sizeof(pixel_t));
                    if (cx != x + ox || cy != y + oy) {
                        MOVETO(x + ox, y + oy);
                        cx = x + ox;if (cx < 0) {((char *)0)[0] = 100;}
                        cy = y + oy;
                    }
                    set_color(pixel->color, pixel->bgcolor, pixel->high);
                    if (pixel->c) {
#if 0                        
                        if (pixel == &bgpixel) {
                            printf("%c", aa);
                            aa++;
                            if (aa > 123) aa = 32;
                        } else
#endif
                        for (i = sizeof(pixel->c) - 1;i >= 0;i--) {
                            c = (pixel->c >> (i * 8));
                            if (c) printf("%c", c);
                        }
                        cx++;
                    }
                }
            }
        }
    }
    if (foreground_win && foreground_win->focus_child) {
        if (foreground_win->focus_child->pf_show_cur) foreground_win->focus_child->pf_show_cur(foreground_win->focus_child);
    }
    fflush(stdout);
}

void flash_window(window_t *w)
{
    window_t *f = foreground_win;
    foreground_win = NULL;
    w->pf_draw_handle(w);
    show_window_inner(w);
    usleep(100000);
    foreground_win = w;
    w->pf_draw_handle(w);
    show_window_inner(w);
    usleep(100000);
    foreground_win = NULL;
    w->pf_draw_handle(w);
    show_window_inner(w);
    usleep(100000);
    foreground_win = w;
    w->pf_draw_handle(w);
    show_window_inner(w);
    usleep(100000);
    foreground_win = NULL;
    w->pf_draw_handle(w);
    show_window_inner(w);
    usleep(100000);
    foreground_win = f;
    w->pf_draw_handle(w);
    show_window_inner(w);
}

void update_screen()
{
    if (!desk) return;
//    set_win_size(desk, 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
    show_window_inner(desk);
}

void refresh_screen()
{
    if (!desk) return;
    set_win_size(desk, 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
    draw_desk(desk);
    show_window_inner(desk);
}

window_t *get_root_win(window_t *w)
{
    window_t *p;
    for (p = w;p && p->parent && p->parent != desk;p = p->parent) {
        if (p->style == WIN_STYLE_SIZEABLE) break;
    }
    return p;
}

void set_win_style(window_t *w, int style)
{
    w->style = style;
}

void set_win_text(window_t *w, char *text)
{
    strncpy(w->text, text, sizeof(w->text) - 1);
    redraw_window(w);
}

char *get_win_text(window_t *w)
{
    return w->text;
}

static void draw_move_win(window_t *w)
{
//    pixel_t *pixel;
//    if (drag_type == DRAG_MOVE) {
    draw_table_rectangle(w, 0, 0, w->width - 1, w->height - 1, WHITE, BLACK);
#if 0
        pixel = get_pixel_p(w, 0, 0);
        pixel->c = U_TABLE(0x250c);
        pixel->color = WHITE;
        pixel->bgcolor = BLACK;
        pixel = get_pixel_p(w, w->width - 1, 0);
        pixel->c = U_TABLE(0x2510);
        pixel->color = WHITE;
        pixel->bgcolor = BLACK;
        pixel = get_pixel_p(w, 0, w->height - 1);
        pixel->c = U_TABLE(0x2514);
        pixel->color = WHITE;
        pixel->bgcolor = BLACK;
        pixel = get_pixel_p(w, w->width - 1, w->height - 1);
        pixel->c = U_TABLE(0x2518);
        pixel->color = WHITE;
        pixel->bgcolor = BLACK;
#endif
#if 0
    } else {
        pixel = get_pixel_p(w, 0, w->height - 1);
        pixel->c = U_TABLE(0x2514);
        pixel->color = WHITE;
        pixel->bgcolor = BLACK;

    }
#endif
}
void win_handle_mouse_down(int key, int x, int y, int ctrl)
{
    window_t *w = get_window(x, y);
    window_t *p = NULL;
    window_t *modal;
    pixel_t *pixel;
    int ox, oy;
    if (!w) return;
    get_win_abs_pos(w, &ox, &oy);
    if (ctrl) {
        MOVETO(0, 20);
        pixel = get_pixel_p(desk, x, y);
        printf("%s %d   \r\n", w->text, pixel->c);
        printf("%d %d     ", ox, oy);
        return;
    }
    if (drag) {
        destroy_window(drag);
        drag = NULL;
        update_screen();
    }
    if (key == MOUSE_LEFT) {
        modal = w;
        while (modal->parent && modal->parent != desk) modal = modal->parent;
        p = get_root_win(w);
        if (p->modal_child || (modal->modal_child && modal->modal_child != p)) {
            while (modal->modal_child) modal = modal->modal_child;
            bring_win_top(modal);
            DING();
            flash_window(modal);
        } else {
            bring_win_top(p);
            if (w->can_focus) set_focus_window(w);
            if (w->pf_internal_mouse_down) w->pf_internal_mouse_down(w, key, x - ox, y - oy, ctrl);
        }
    }
}

void win_handle_mouse_up(int key, int x, int y, int ctrl)
{
    window_t *w;
    int ox, oy;
    get_win_abs_pos(drag, &ox, &oy);
    if (drag) {
        if (drag->pf_internal_mouse_up) drag->pf_internal_mouse_up(drag, key, x - ox, y - oy, ctrl);
        drag = NULL;
        update_screen();
    } else {
        int ox, oy;
        w = get_window(x, y);
        if (!w) return;
        get_win_abs_pos(w, &ox, &oy);
        if (w->pf_internal_mouse_up) w->pf_internal_mouse_up(w, key, x - ox, y - oy, ctrl);
    }
}

void win_handle_mouse_drag(int key, int x, int y, int ctrl)
{
    int ox, oy;
    if (drag) {
        get_win_abs_pos(drag, &ox, &oy);
        if (drag->pf_internal_mouse_drag) drag->pf_internal_mouse_drag(drag, key, x - ox, y - oy, ctrl);
        update_screen();
    }
}

void win_handle_mouse_wheel(int updown, int x, int y, int ctrl)
{
    window_t *w = get_window(x, y);
    if (!w) return;
    if (w->pf_internal_mouse_wheel) w->pf_internal_mouse_wheel(w, updown, x, y, ctrl);
}

void win_handl_key(unsigned int key)
{
    window_t *w;
    if (drag) {
        destroy_window(drag);
        drag = NULL;
        update_screen();
    }
    if (!foreground_win) return;
    if (foreground_win->focus_child) w = foreground_win->focus_child; else w = foreground_win;
    switch (key) {
        case KEY_F1:
            {((char *)0)[0] = 100;}
            break;
        case KEY_F2:
            update_screen();
            break;
        case KEY_F3:
            SHOW_CURSOR();
            break;
        case KEY_F4:
            printf("%s\r\n", w->text);
            break;
        case KEY_F11:
            MALLOC(102400);
            break;
        case KEY_F12:
            printf("alloc_size = %d, alloc_block_count = %d\r\n", alloc_size, alloc_block_count);
            break;
        default:
//            printf("0x%x\r\n", key);
            break;
    }   
    if (w->pf_internal_key) w->pf_internal_key(w, key);
}

void *event_loop(void *arg)
{
    modal_lock_t *modal_lock = arg;
    thread_context_t tctxt;
    int key;
    unsigned char m_e;
    unsigned char m_pk = 255;
    unsigned char m_k;
    unsigned char m_act;
    unsigned char m_ctrl;
    int m_x;
    int m_y;
    int m_ox = -1;
    int m_oy = -1;
    memset(&tctxt, 0x00, sizeof(thread_context_t));
    while (1) {
        if (g_tctxt != &tctxt) g_tctxt = &tctxt;
        key = get_key();
        if (IS_MOUSE_EVENT(key)) {
            m_e = (key >> 16) & 0xff;
            m_x = ((key >> 8) & 0xff) - 0x21;
            m_y = (key & 0xff) - 0x21;
            m_k = m_e & MOUSE_KEY_UP;
            m_act = m_e & (MOUSE_DOWN | MOUSE_DRAG);
            m_ctrl = m_e & (MOUSE_SHIFT | MOUSE_ALT | MOUSE_CTRL);
            if (m_act == MOUSE_DOWN) {
                if (m_k == MOUSE_KEY_UP) {/* mouse key up */
                    if (m_pk != 255) {
                        win_handle_mouse_up(m_pk, m_x, m_y, m_ctrl);
                        m_pk = 255;
                        m_ox = -1;
                        m_oy = -1;
                    }
                } else {/* mouse key down */
                    win_handle_mouse_down(m_k, m_x, m_y, m_ctrl);
                    m_pk = m_k;
                    m_ox = m_x;
                    m_oy = m_y;
                }
            } else if (m_act == MOUSE_DRAG) {
                if ((m_ox != -1 && m_oy != -1)
                        && (m_ox != m_x || m_oy != m_y)){
                    win_handle_mouse_drag(m_k, m_x, m_y, m_ctrl);
                    m_ox = m_x;
                    m_oy = m_y;
                }
            } else if (m_act == MOUSE_WHEEL) {
                win_handle_mouse_wheel(m_k, m_x, m_y, m_ctrl);
            } else {
                printf("??\r\n");
            }
        } else {
            win_handl_key(key);
        }
        if (key == 'q') close_window(desk);
        if (tctxt.terminal) break;
    }
    pthread_mutex_lock(&modal_lock->mutex);
    pthread_cond_signal(&modal_lock->cond);
    pthread_mutex_unlock(&modal_lock->mutex);
    return NULL;
}

void modal_finish(window_t *w)
{
    window_t *p = get_root_win(w->parent);
    if (!w->pf_modal_finish) return;
    w->pf_modal_finish = NULL;
    if (p) p->modal_child = NULL;
    g_tctxt->terminal = TRUE;
}

int do_modal(window_t *w)
{
    pthread_t tid;
    modal_lock_t modal_lock;
    window_t *p;
    if (NULL == w) w = desk;
    p = get_root_win(w->parent);
    if (p) p->modal_child = w;
    w->pf_modal_finish = modal_finish;
    w->visible = TRUE;
    bring_win_top(w);
    show_window_inner(w);
    memset(&modal_lock, 0x00, sizeof(modal_lock));
    pthread_cond_init(&modal_lock.cond, NULL);
    pthread_mutex_init(&modal_lock.mutex, NULL);
    pthread_mutex_lock(&modal_lock.mutex);
    pthread_create(&tid, NULL, event_loop, &modal_lock); 
    pthread_cond_wait(&modal_lock.cond, &modal_lock.mutex);
    pthread_mutex_unlock(&modal_lock.mutex);
    pthread_cond_destroy(&modal_lock.cond);
    pthread_mutex_destroy(&modal_lock.mutex);
    return w->modal_result;
}

void show_window(window_t *w)
{
    w->visible = TRUE;
    show_window_inner(w);
}

int close_window(window_t *w)
{
    window_t *p = get_root_win(w->parent);
    int r;
    if (w->pf_close_window) {
        r = w->pf_close_window(w);
        if (r != OK) return r;
    }
    w->visible = FALSE;
    if (foreground_win == w) bring_win_top(p);
//    update_screen();
    if (w->pf_modal_finish) w->pf_modal_finish(w);
    return OK;
}

void win_printf(window_t *w, int x, int y, int c, int b, int hi, char *fmt,...)
{
    unsigned char buf[256];
    unsigned char *p = buf;
    va_list ap;
    int val;
    char *s = 0;
    pixel_t *pixel;

    va_start(ap, fmt);
    while (*fmt) {
        switch(*fmt)
        {
            case '%':
                fmt++;
                switch (*fmt) {
                    case 'd':
                        val = va_arg(ap, int);
                        p += sprintf((char *)p, "%d", val);
                        break;
                    case 'x':
                        val = va_arg(ap, int);
                        p += sprintf((char *)p, "%x", val);
                        break;
                    case 's':
                        s = va_arg(ap, char *);
                        p += sprintf((char *)p, "%s", s);
                        break;
                    case 'c':
                        *p = va_arg(ap, int);
                        p++;
                        break;
                    default:
                        break;
                }
                break;

            default:
                *p = *fmt;
                p++;
                break;
        }

        fmt++;
    }
    *p = 0;
    va_end(ap); 

    p = buf;
    while (*p) {
        if (*p == '\n' || *p == '\r') {
            x =  w->width - 1;
        } else if (*p == 0xe2) {
            pixel = get_pixel_p(w, x, y);
            if (pixel) {
                pixel->c = 0xe20000 | (p[1] * 256) | p[2];
                pixel->high = hi;
                pixel->color = c;
                pixel->bgcolor = b;
                p = p + 2;
            }
        } else {
            pixel = get_pixel_p(w, x, y);
            if (pixel) {
                pixel->c = *p;
                pixel->high = hi;
                pixel->color = c;
                pixel->bgcolor = b;
            }
        }
        p++;
        x++;
        if (x >= w->width) {
            x = 0;
            y++;
            if (y >= w->height) {
                break;
            }
        }
    }
}

void set_win_on_resize(window_t *w, void (*pf_on_resize)(window_t *w, int width, int height))
{
    w->pf_on_resize = pf_on_resize;
}

void set_win_on_click(window_t *w, void (*pf_on_click)(window_t *w))
{
    w->pf_on_click = pf_on_click;
}

void set_win_on_change(window_t *w, void (*pf_on_change)(window_t *w))
{
    w->pf_on_change = pf_on_change;
}
