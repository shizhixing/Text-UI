#include <stdio.h>
#include <string.h>
#include "def.h"
#include "window_type.h"
#include "window.h"
#include "cursor.h"
#include "input.h"

typedef struct inner_scroll_t {
    int mouse_down;
    int pos;
    int posw;
    int posd;
    int count;
    int x, y;
    int slider_pos;
    int old_pos;
    int step;
} inner_scroll_t;

#define WINDOW_TO_SCROLL(x) ((inner_scroll_t *)((x) + 1))

void draw_scroll(window_t *w) 
{
    int width;
    int height;
    int i;
    window_t *parent = w->parent;
    inner_scroll_t *scroll = WINDOW_TO_SCROLL(w);
    if (parent->horz_scroll == w) {
        width = parent->width - 2;
        height = 1;
        if (width != w->width || height != w->height) {
            set_win_size(w, 1, parent->height - 1, width, height);
        }
        if (scroll->mouse_down && scroll->x == 0 && scroll->y == 0) {
            win_printf(w, 0, 0, BLUE, YELLOW, TRUE, "<");
        } else {
            win_printf(w, 0, 0, WHITE, BLACK, FALSE, "<");
        }
        if (scroll->count > 1) {
            scroll->posw = width - 2 - scroll->count + 1;
            if (scroll->posw < 1) scroll->posw = 1;
            scroll->posd = scroll->pos * (width - 2 - scroll->posw + 1) / scroll->count;
            scroll->step = scroll->count * 10;
            scroll->step = (scroll->step / (width - 2 - scroll->posw + 1) + 5) / 10;
            for (i = 0;i < scroll->posd;i++) {
                win_printf(w, i + 1, 0, WHITE, BLACK, FALSE, "\u2500");
            }
            for (i = 0;i < scroll->posw;i++) {
                win_printf(w, i + 1 + scroll->posd, 0, WHITE, BLACK, FALSE, "\u2588");
            }
            for (i = scroll->posd + scroll->posw;i < width - 1;i++) {
                win_printf(w, i + 1, 0, WHITE, BLACK, FALSE, "\u2500");
            }
        } else {
            scroll->posw = 0;
            for (i = 1;i < width - 1;i++) {
                win_printf(w, i, 0, WHITE, BLACK, FALSE, "\u2500");
            }
        }
        if (scroll->mouse_down && (scroll->x == width - 1) && scroll->y == 0) {
            win_printf(w, width - 1, 0, BLUE, YELLOW, FALSE, ">");
        } else {
            win_printf(w, width - 1, 0, WHITE, BLACK, FALSE, ">");
        }
    } else if (parent->vert_scroll == w) {
        width = 1;
        height = parent->height - 2;
        if (width != w->width || height != w->height) {
            set_win_size(w, parent->width - 1, 1, width, height);
        }
        if (scroll->mouse_down && scroll->x == 0 && scroll->y == 0) {
            win_printf(w, 0, 0, BLUE, YELLOW, TRUE, "\u2227");
        } else {
            win_printf(w, 0, 0, WHITE, BLACK, TRUE, "\u2227");
        }
        if (scroll->count > 1) {
            scroll->posw = height - 2 - scroll->count + 1;
            if (scroll->posw < 1) scroll->posw = 1;
            scroll->posd = scroll->pos * (height - 2 - scroll->posw + 1) / scroll->count;
            scroll->step = scroll->count * 10;
            scroll->step = (scroll->step / (height - 2 - scroll->posw + 1) + 5) / 10;
            for (i = 0;i < scroll->posd;i++) {
                win_printf(w, 0, i + 1, WHITE, BLACK, FALSE, "\u2502");
            }
            for (i = 0;i < scroll->posw;i++) {
                win_printf(w, 0, i + 1 + scroll->posd, WHITE, BLACK, FALSE, "\u2588");
            }
            for (i = scroll->posd + scroll->posw;i < height - 1;i++) {
                win_printf(w, 0, i + 1, WHITE, BLACK, FALSE, "\u2502");
            }
        } else {
            scroll->posw = 0;
            for (i = 1;i <  height - 1;i++) {
                win_printf(w, 0, i, WHITE, BLACK, FALSE, "\u2502");
            }
        }
        if (scroll->mouse_down && scroll->x == 0 && (scroll->y == height - 1)) {
            win_printf(w, 0, height - 1, BLUE, YELLOW, TRUE, "\u2228");
        } else {
            win_printf(w, 0, height - 1, WHITE, BLACK, TRUE, "\u2228");
        }
    }
}

void internal_mouse_down_scroll(window_t *w, int key, int x, int y, int ctrl)
{
    inner_scroll_t *b = WINDOW_TO_SCROLL(w);
    if (ctrl == 0 && key == MOUSE_LEFT) {
        drag = w;
        drag_type = DRAG_MOVE;
        b->mouse_down = TRUE;
        b->x = x;
        b->y = y;

        if (w->height == 1) {
            if ((x - 1 >= b->posd) && (x - 1 - b->posd < b->posw)) {
                b->old_pos = b->pos;
                b->slider_pos = x;
            }
        } else if (w->width == 1) {
            if ((y - 1 >= b->posd) && (y - 1 - b->posd < b->posw)) {
                b->old_pos = b->pos;
                b->slider_pos = y;
            }
        }
        draw_scroll(w);
        show_window_inner(w);
    }
}

void internal_mouse_up_scroll(window_t *w, int key, int x, int y, int ctrl)
{
    inner_scroll_t *b = WINDOW_TO_SCROLL(w);
    if (w == drag) {
        drag = NULL;
        if (ctrl == 0 && key == MOUSE_LEFT && x == b->x && y == b->y) {
            if (x == 0 && y == 0) set_scroll_pos(w, b->pos - 1);
            else if ((x == w->width - 1 && w->height == 1) || (y == w->height - 1 && w->width == 1)) set_scroll_pos(w, b->pos + 1);
        }
        if (key == MOUSE_LEFT && ctrl == 0 && x >= 0 && x < w->width && y >= 0 && y <= w->height) {
//            if (w->pf_on_click) w->pf_on_click(w);
        }
    }
    b->mouse_down = FALSE;
    draw_scroll(w);
}

void internal_mouse_drag_scroll(window_t *w, int key, int x, int y, int ctrl)
{
    inner_scroll_t *b = WINDOW_TO_SCROLL(w);
    if (key == MOUSE_LEFT && ctrl == 0) {
        if ((b->x == 0 && b->y == 0) || (b->x == w->width - 1 && b->y == w->height - 1)) {
            if (b->x == x) {
                b->mouse_down = TRUE;
            } else {
                b->mouse_down = TRUE;
            }
        } else if (b->mouse_down) {        
            if (w->height == 1) {
                set_scroll_pos(w, b->old_pos + (x - b->slider_pos) * b->step);
            } else if (w->width == 1) {
                set_scroll_pos(w, b->old_pos + (y - b->slider_pos) * b->step);
            }
        } else {
            b->mouse_down = FALSE;
        }
    } else {
        b->mouse_down = FALSE;
    }
    draw_scroll(w);
}

scroll_t *create_scroll(window_t *parent, int direction)
{
    window_t *w = create_window(parent, draw_scroll, sizeof(window_t) + sizeof(inner_scroll_t));
    set_win_style(w, WIN_STYLE_NONE);
    if (direction == 0) parent->horz_scroll = w; else parent->vert_scroll = w;
    w->can_focus = FALSE;
    w->pf_internal_mouse_down = internal_mouse_down_scroll;
    w->pf_internal_mouse_up = internal_mouse_up_scroll;
    w->pf_internal_mouse_drag = internal_mouse_drag_scroll;
    redraw_window(w);
    return w;
}

int get_scroll_pos(window_t *w)
{
    inner_scroll_t *b = WINDOW_TO_SCROLL(w);
    return b->pos;
}

int get_scroll_count(window_t *w)
{
    inner_scroll_t *b = WINDOW_TO_SCROLL(w);
    return b->count;
}

void set_scroll_pos(window_t *w, int pos)
{
    inner_scroll_t *b = WINDOW_TO_SCROLL(w);
    if (b->count < 1) return;
    if (pos < 0) pos = 0;
    if (pos >= b->count) pos = b->count - 1;
    if (b->pos == pos) return;
    b->pos = pos;
    if (w->pf_on_change) w->pf_on_change(w);
    redraw_window(w);
    show_window_inner(w);
}

void set_scroll_count(window_t *w, int count)
{
    inner_scroll_t *b = WINDOW_TO_SCROLL(w);
    if (count < 1) count = 1;
    if (b->count == count) return;
    b->count = count;
    set_scroll_pos(w, b->pos);
    redraw_window(w);
    show_window_inner(w);
}
