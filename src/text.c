#include <stdio.h>
#include <string.h>
#include "def.h"
#include "window_type.h"
#include "window.h"
#include "cursor.h"
#include "input.h"

typedef struct inner_text_t {
    int cur;
    int display_start;
    int display_len;
    int px, py;
} inner_text_t;

#define WINDOW_TO_TEXT(x) ((inner_text_t *)((x) + 1))

static void text_draw_text(window_t *w)
{
    int i;
    int over = FALSE;
    inner_text_t *t = WINDOW_TO_TEXT(w);
    for (i = 0;i < t->display_len;i++) {
        if (!over && !w->text[i + t->display_start]) over = TRUE;
        if (over) win_printf(w, t->px + i, t->py, WHITE, BLACK, 0, " ");
        else win_printf(w, t->px + i, t->py, WHITE, BLACK, 0, "%c", w->text[i + t->display_start]);
    }
}

void draw_text(window_t *w) 
{
    int i, size = w->width * w->height;
    inner_text_t *t = WINDOW_TO_TEXT(w);
    for (i = 0;i < size;i++) {
        w->pixel[i].color = WHITE;
        w->pixel[i].bgcolor = BLACK;
    }
    if (w->height > 3) w->height = 3;
    if (w->height == 3) {
        t->px = 1;
        t->py = 1;
        draw_table_rectangle(w, 0, 0, w->width - 1, w->height - 1, WHITE, BLACK);
        t->display_len = w->width - 2;
    } else {
        t->px = 0;
        t->py = 0;
        t->display_len = w->width;
    }
    text_draw_text(w);
}

void show_cur_text(window_t *w)
{
    inner_text_t *t = WINDOW_TO_TEXT(w);
    int x, y;
    get_win_abs_pos(w, &x, &y);
    if (w == get_window(x + t->px + t->cur - t->display_start, y + t->py)) {
        MOVETO(x + t->px + t->cur - t->display_start, y + t->py);
        SHOW_CURSOR();
        set_color(BLACK, WHITE, 0);
    }
}

void internal_key_text(window_t *w, unsigned int key)
{
    int i;
//    int x, y;
    inner_text_t *t = WINDOW_TO_TEXT(w);
    if (w->pf_on_key) w->pf_on_key(w, &key);
    if (key >= 32 && key <= 126) {
        if (strlen(w->text) >= sizeof(w->text) - 1) {
            DING();
            return;
        }
        for (i = strlen(w->text);i > t->cur;i--) {
            w->text[i] = w->text[i - 1];
        }
        w->text[t->cur] = key;
        t->cur++;
    } else switch (key) {
        case KEY_LEFT:
            if (t->cur) {
                t->cur--;
            } else {
                DING();
                return;
            }
            break;
        case KEY_RIGHT:
            if (w->text[t->cur]) {
                t->cur++;
            } else {
                DING();
                return;
            }
            break;
        case KEY_BS:
            if (t->cur == 0) {
                DING();
                return;
            }
            t->cur--;
        case KEY_DEL:
            if (w->text[t->cur]) {
                i = t->cur;
                while (w->text[i]) {
                    w->text[i] = w->text[i + 1];
                    i++;
                }
                if (strlen(w->text) <= t->display_len) t->display_start = 0;
            } else {
                DING();
                return;
            }
            break;
        case KEY_HOME:
            t->cur = 0;
            break;
        case KEY_END:
            t->cur = strlen(w->text);
            break;
        case KEY_F4:
            update_screen();
            break;
        default:
            break;
    }
    
    if (t->cur >= t->display_start + t->display_len) {
        t->display_start = t->cur - t->display_len + 1;
        for (i = 0;i < 10 && (t->cur - t->display_start > 0);i++) {
            if (w->text[t->display_start + t->display_len]) t->display_start++;
        }
    } else if (t->cur - t->display_start < 0) {
        t->display_start = t->cur;
        for (i = 0;i < 10 && (t->cur < t->display_start + t->display_len - 1);i++) {
            if (t->display_start) t->display_start--; 
        }
    }
    text_draw_text(w);
    show_window_inner(w);
    return;
}

void internal_mouse_down_text(window_t *w, int key, int x, int y, int ctrl)
{
    inner_text_t *t = WINDOW_TO_TEXT(w);
    int len;
    if (x < t->px || (x >= w->width - t->px) || y < t->py || (y >= w->height - t->py)) return;
    if (key == MOUSE_LEFT && ctrl == 0) {
        len = strlen(w->text);
        if (t->display_start + x - t->px < len)
            t->cur = t->display_start + x - t->px;
        else 
            t->cur = len;

        show_cur_text(w);
    }
}

text_t *create_text(window_t *parent, int x, int y, int width)
{
    window_t *w = create_window(parent, draw_text, sizeof(window_t) + sizeof(inner_text_t));
    set_win_style(w, WIN_STYLE_NONE);
    set_win_size(w, x, y, width, 3);
    w->can_focus = TRUE;
    w->pf_show_cur = show_cur_text;
    w->pf_internal_key = internal_key_text;
    w->pf_internal_mouse_down = internal_mouse_down_text;
    redraw_window(w);
    return w;
}
