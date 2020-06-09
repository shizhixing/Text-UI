#include <stdio.h>
#include <string.h>
#include "def.h"
#include "window_type.h"
#include "window.h"
#include "cursor.h"
#include "input.h"

typedef struct inner_button_t {
    int mouse_down;
    int px, py;
    char nl, nr, pl, pr;
} inner_button_t;

#define WINDOW_TO_BUTTON(x) ((inner_button_t *)((x) + 1))

void draw_button(window_t *w) 
{
    int n;
    int i;
    int c, b, hi;
    inner_button_t *btn = WINDOW_TO_BUTTON(w);
    if (is_window_focused(w)) {
        c = BLUE;
        b = YELLOW;
        hi = TRUE;
    } else {
        c = WHITE;
        b = BLACK;
        hi = FALSE;
    }
    if (btn->mouse_down) win_printf(w, 0, 0, c, b, hi, "%c", btn->pl);
    else win_printf(w, 0, 0, c, b, hi, "%c", btn->nl);
    for (i = 1;i < w->width - 1;i++) {
        win_printf(w, i, 0, c, b, hi, " ");
    }
    n = (w->width - strlen(w->text) - 2) >> 1;
    win_printf(w, n + 1, 0, c, b, hi, "%s", w->text);
    if (btn->mouse_down) win_printf(w, w->width - 1, 0, c, b, hi, "%c", btn->pr);
    else win_printf(w, w->width - 1, 0, c, b, hi, "%c", btn->nr);
}

void internal_key_button(window_t *w, unsigned int key)
{
    if (w->pf_on_key) w->pf_on_key(w, &key);
    switch (key) {
        case KEY_ENTER:
            if (w->pf_on_click) w->pf_on_click(w);
            break;
        default:
            break;
    }
    return;
}

void internal_mouse_down_button(window_t *w, int key, int x, int y, int ctrl)
{
    inner_button_t *b = WINDOW_TO_BUTTON(w);
    if (ctrl == 0 && key == MOUSE_LEFT) {
        drag = w;
        drag_type = DRAG_MOVE;
        b->mouse_down = TRUE;
        draw_button(w);
        show_window_inner(w);
    }
}

void internal_mouse_up_button(window_t *w, int key, int x, int y, int ctrl)
{
    inner_button_t *b = WINDOW_TO_BUTTON(w);
    if (w == drag) {
        drag = NULL;
        if (key == MOUSE_LEFT && ctrl == 0 && x >= 0 && x < w->width && y >= 0 && y <= w->height) {
            if (w->pf_on_click) w->pf_on_click(w);
        }
    }
    b->mouse_down = FALSE;
    draw_button(w);
}

void internal_mouse_drag_button(window_t *w, int key, int x, int y, int ctrl)
{
    inner_button_t *b = WINDOW_TO_BUTTON(w);
    if (key == MOUSE_LEFT && ctrl == 0 && x >= 0 && x < w->width && y >= 0 && y <= w->height) {
        b->mouse_down = TRUE;
    } else {
        b->mouse_down = FALSE;
    }
    draw_button(w);
}

button_t *create_button(window_t *parent, int x, int y, int width)
{
    window_t *w = create_window(parent, draw_button, sizeof(window_t) + sizeof(inner_button_t));
    set_win_style(w, WIN_STYLE_NONE);
    set_win_size(w, x, y, width, 1);
    w->can_focus = TRUE;
    w->pf_internal_key = internal_key_button;
    w->pf_internal_mouse_down = internal_mouse_down_button;
    w->pf_internal_mouse_up = internal_mouse_up_button;
    w->pf_internal_mouse_drag = internal_mouse_drag_button;
    inner_button_t *b = WINDOW_TO_BUTTON(w);
    b->nl = '<';
    b->nr = '>';
    b->pl = '[';
    b->pr = ']';
    redraw_window(w);
    return w;
}

void set_button_edge(window_t *w, char normal_left, char normal_right, char press_left, char press_right)
{
    inner_button_t *b = WINDOW_TO_BUTTON(w);
    b->nl = normal_left;
    b->nr = normal_right;
    b->pl = press_left;
    b->pr = press_right;
}
