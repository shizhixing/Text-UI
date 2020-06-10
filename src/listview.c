#include <stdio.h>
#include <string.h>
#include "def.h"
#include "window_type.h"
#include "window.h"
#include "cursor.h"
#include "input.h"

typedef struct list_head_t {
    struct list_head_t *prev;
    struct list_head_t *next;
    button_t *btn;
} list_head_t;

typedef struct list_row_t {
    struct list_row_t *up;
    struct list_row_t *down;
    struct list_row_t *prev;
    struct list_row_t *next;
    char text[256];
} list_row_t;

typedef struct inner_listview_t {
    list_head_t *head, *tail;
    list_row_t *row_head;
    list_row_t *row_tail;
    list_row_t *display_start_row;
    int display_row_count;
    int total_row_count;
} inner_listview_t;

#define WINDOW_TO_LISTVIEW(x) ((inner_listview_t *)((x) + 1))

void draw_listview(window_t *w) 
{
    inner_listview_t *t = WINDOW_TO_LISTVIEW(w);
    list_head_t *head;
    list_row_t *row, *cell;
    int left, top, width, height;
    int i, y = 2;
    draw_table_rectangle(w, 0, 0, w->width - 1, w->height - 1, WHITE, BLACK);
    for (y = 1;y < w->height - 1;y++) {
        for (i = 1;i < w->width - 1;i++) {
            win_printf(w, i, y, WHITE, BLACK, FALSE, " ");
        }
    }
    for (y = 2, row = t->row_head;y < w->height - 1 && row;y++, row = row->next) {
        for (head = t->head, cell = row;head && cell;head = head->next, cell = cell->next) {
            get_win_size(head->btn, &left, &top, &width, &height);
            for (i = 0;i < left + width - 1;i++) {
                if (cell->text[i] == 0) break;
                if ((i + left > 0) && (i + left < w->width - 1)) {
                    win_printf(w, i + left, y, WHITE, BLACK, FALSE, "%c", cell->text[i]);
                }
            }
        }
    }
}

void internal_key_listview(window_t *w, unsigned int key)
{
}

void internal_mouse_down_listview(window_t *w, int key, int x, int y, int ctrl)
{
}

void scroll_on_change_listview(window_t *w)
{
    window_t *p = w->parent;
    inner_listview_t *t = WINDOW_TO_LISTVIEW(p);
    list_head_t *h;
    int left, top, width, height;
    int off;
    if (w == p->horz_scroll) {
        off = 1 - get_scroll_pos(w);
        for (h = t->head;h;h = h->next) {
            get_win_size(h->btn, &left, &top, &width, &height);
            left = off;
            set_win_size(h->btn, left, top, width, height);
            off = left + width;
        }
    }
    draw_listview(p);
    show_window_inner(p);
}

text_t *create_listview(window_t *parent, int x, int y, int width, int height)
{
    window_t *w = create_window(parent, draw_listview, sizeof(window_t) + sizeof(inner_listview_t));
    scroll_t *scroll;
    set_win_style(w, WIN_STYLE_NONE);
    set_win_size(w, x, y, width, height);
    w->can_focus = TRUE;
    w->pf_internal_key = internal_key_listview;
    w->pf_internal_mouse_down = internal_mouse_down_listview;
    redraw_window(w);
    scroll = create_scroll(w, SCROLL_HORIZONAL);
    set_win_on_change(scroll, scroll_on_change_listview);
    scroll = create_scroll(w, SCROLL_VERTICAL);
    set_win_on_change(scroll, scroll_on_change_listview);

    return w;
}

void listview_add_column(window_t *w, char *text, int width)
{
    inner_listview_t *t = WINDOW_TO_LISTVIEW(w);
    list_head_t *head = (list_head_t *)MALLOC(sizeof(list_head_t));
    int left, top, width2, height;

    memset(head, 0x00, sizeof(list_head_t));
    if (t->head && t->tail) {
        get_win_size(t->tail->btn, &left, &top, &width2, &height);
        left += width2;
        head->btn = create_button(w, left, 1, width);
        set_button_edge(head->btn, ' ', '|', ' ', '!');
        t->tail->next = head;
        head->prev = t->tail;
        t->tail = head;
    } else {
        left = 1;
        head->btn = create_button(w, 1, 1, width);
        set_button_edge(head->btn, ' ', '|', ' ', '!');
        t->head = head;
        t->tail = head;
    }
    set_win_text(head->btn, text);
    set_scroll_count(w->horz_scroll, left + width - w->width + 2);
    bring_win_top(w->vert_scroll);
}

void listview_add_row(window_t *w, char *text)
{
    inner_listview_t *t = WINDOW_TO_LISTVIEW(w);
    list_row_t *row = (list_row_t *)MALLOC(sizeof(list_row_t));
    if (t->row_head && t->row_tail) {
        t->row_tail->next = row;
        row->prev = t->row_tail;
        t->row_tail = row;
    } else {
        t->row_head = row;
        t->row_tail = row;
    }
    strncpy(row->text, text, sizeof(row->text));
    t->total_row_count++;
    set_scroll_count(w->vert_scroll, t->total_row_count - w->height + 3);
}
