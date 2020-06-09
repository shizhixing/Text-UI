#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "def.h"
#include "window_type.h"
#include "window.h"
#include "cursor.h"
#include "input.h"

#define TEXT_BLOCK_SIZE 1 

typedef struct text_block_t {
    struct text_block_t *up;
    struct text_block_t *down;
    struct text_block_t *prev;
    struct text_block_t *next;
    int row_no;
    unsigned len;
    char s[TEXT_BLOCK_SIZE];
} text_block_t;

typedef struct inner_memo_t {
    text_block_t rows;
    int cur_x;
    text_block_t *cur_row;
    text_block_t *display_start_row;
    int display_start_column;
    int display_row_count;
    int display_column_count;
    int row_count;
    int px, py;
} inner_memo_t;

#define WINDOW_TO_MEMO(x) ((inner_memo_t *)((x) + 1))

static int memo_max_text_width(window_t *w)
{
    inner_memo_t *t = WINDOW_TO_MEMO(w);
    text_block_t *row, *block;
    int row_count;
    int max = 0;
    int i;

    for (row = t->display_start_row, i = 0;row && i < t->display_row_count;row = row->down, i++) {
        row_count = 0;
        for (block = row;block;block = block->next) {
            row_count += block->len;
        }
        if (max < row_count) max = row_count;
    }
    return max;
}

static void memo_draw_text(window_t *w, text_block_t *row)
{
    int i;
    int ox, oy;
    int over = FALSE;
    inner_memo_t *t = WINDOW_TO_MEMO(w);
    text_block_t *block = row;
    int off = t->display_start_column;
    if (row->row_no < t->display_start_row->row_no
            || row->row_no >= t->display_start_row->row_no + t->display_row_count) return;
    
    ox = t->px;
    oy = t->py + row->row_no - t->display_start_row->row_no;
    
    while (block && off >= block->len) {
        off = off - block->len;
        block = block->next;
    }
    for (i = 0;i < t->display_column_count;i++) {
        if (!over) {
            if (!block || (off >= block->len)) over = TRUE;
        }
        if (over) win_printf(w, ox + i, oy, WHITE, BLACK, 0, " ");
        else {
            win_printf(w, ox + i, oy, WHITE, BLACK, 0, "%c", block->s[off]);
            off++;
            if (block && off >= block->len) {
                block = block->next;
                off = 0;
            }
        }
    }
    if ((!row->down) && (row->row_no + 1 < t->display_start_row->row_no + t->display_row_count)) {
        for (i = 0;i < t->display_column_count;i++) {
            win_printf(w, ox + i, oy + 1, WHITE, BLACK, 0, " ");
        }
    }
}

void draw_memo(window_t *w) 
{
    int i, size = w->width * w->height;
    inner_memo_t *t = WINDOW_TO_MEMO(w);
    text_block_t *row;
    for (i = 0;i < size;i++) {
        w->pixel[i].c = ' ';
        w->pixel[i].color = WHITE;
        w->pixel[i].bgcolor = BLACK;
    }
    t->px = 1;
    t->py = 1;
    draw_table_rectangle(w, 0, 0, w->width - 1, w->height - 1, WHITE, BLACK);
    t->display_row_count = w->height - 2;
    t->display_column_count = w->width - 2;

    row = t->display_start_row;
    for (i = 0;row && i < t->display_row_count;i++) {
        memo_draw_text(w, row);
        row = row->down;
    }
}

static void memo_adjust(window_t *w, int cur_x, int redraw, int update_cur)
{
    inner_memo_t *t = WINDOW_TO_MEMO(w);
    int max_width = memo_max_text_width(w);
    text_block_t *t_block;
    int i, m;

    while ((t->display_start_column + t->display_column_count > max_width + 1) && t->display_start_column > 0) t->display_start_column--;
    if (cur_x >= t->display_start_column + t->display_column_count) {
        t->display_start_column = cur_x - t->display_column_count + 1;
        m = 0;
        for (t_block = t->cur_row;t_block;t_block = t_block->next) {
            m += t_block->len;
        }
        for (i = 0;i < 10 && (cur_x - t->display_start_column > 0);i++) {
            if (t->display_start_column + t->display_column_count < m) t->display_start_column++;
        }
        redraw = TRUE;
    } else if (cur_x < t->display_start_column) {
        t->display_start_column = cur_x;
        for (i = 0;i < 10 && (cur_x < t->display_start_column + t->display_column_count - 1);i++) {
            if (t->display_start_column) t->display_start_column--;
        }
        redraw = TRUE;
    }
    if (update_cur) {
        if (t->cur_row->row_no < t->display_start_row->row_no) {
            t->display_start_row = t->cur_row;
            redraw = TRUE;
        }
        while (t->cur_row->row_no >= t->display_start_row->row_no + t->display_row_count) {
            t->display_start_row = t->display_start_row->down;
            redraw = TRUE;
        }
    }
    set_scroll_count(w->horz_scroll, memo_max_text_width(w) - w->width + 4);
    set_scroll_pos(w->horz_scroll, t->display_start_column);
    set_scroll_count(w->vert_scroll, t->row_count - w->height + 3);
    set_scroll_pos(w->vert_scroll, t->display_start_row->row_no);
    if (redraw) draw_memo(w);
    else memo_draw_text(w, t->cur_row);
    show_window_inner(w);
}

void show_cur_memo(window_t *w)
{
    inner_memo_t *t = WINDOW_TO_MEMO(w);
    int x, y;
    int m;
    text_block_t *t_block;
    int rx, ry;
    if (t->cur_row->row_no < t->display_start_row->row_no 
            || t->cur_row->row_no >= t->display_start_row->row_no + t->display_row_count) return;
    get_win_abs_pos(w, &x, &y);
    m = 0;
    for (t_block = t->cur_row;t_block;t_block = t_block->next) {
        m += t_block->len;
    }
    if (m > t->cur_x) m = t->cur_x;
    rx = x + t->px + m - t->display_start_column;
    ry = y + t->py + t->cur_row->row_no - t->display_start_row->row_no;
    if (w == get_window(rx, ry)) {
        MOVETO(rx, ry);
        SHOW_CURSOR();
        set_color(WHITE, BLACK, 0);
    }
}

text_block_t *merge_block(text_block_t *a)
{
    text_block_t *b;
    if (!a) return NULL;
    b = a->next;
    if (!b) return a;
    if (a->len + b->len <= TEXT_BLOCK_SIZE) {
        memcpy(a->s + a->len, b->s, b->len);
        a->len += b->len;
        a->next = b->next;
        if (b->next) {
            b->next->prev = a;
        }
        FREE(b);
        return a;
    }
    return a;
}

text_block_t *split_block(text_block_t *a, int cur)
{
    text_block_t *tb;
    if (!a) return NULL;
    tb = (text_block_t *)MALLOC(sizeof(text_block_t));
    memset(tb, 0x00, sizeof(text_block_t));
    tb->next = a->next;
    tb->prev = a;
    if (a->next) a->next->prev = tb;
    a->next = tb;
    tb->len = a->len - cur;
    if (tb->len) memcpy(tb->s, a->s + cur, tb->len);
    a->len = cur;
    return a;
}

void internal_key_memo(window_t *w, unsigned int key)
{
    int i;
    int m;
    inner_memo_t *t = WINDOW_TO_MEMO(w);
    text_block_t *cur_block = t->cur_row;
    text_block_t *t_block;
    int cur = t->cur_x;
    int cur_x = 0;
    int redraw = FALSE;
    int update_cur = FALSE;
    if (w->pf_on_key) w->pf_on_key(w, &key);
    while (1) {
        if (cur <= cur_block->len) {
            cur_x += cur;
            break;
        }
        cur = cur - cur_block->len;
        cur_x += cur_block->len;
        if (cur_block->next) {
            cur_block = cur_block->next;
        } else {
            cur = cur_block->len;
            break;
        }
    }

    if (key >= 32 && key <= 126) {
        if (cur_block->len == TEXT_BLOCK_SIZE) {
            if (cur == 0) {
                if (cur_block->prev && (cur_block->prev->len < TEXT_BLOCK_SIZE)) {
                    cur_block = cur_block->prev;
                    cur = cur_block->len;
                } else {
                    split_block(cur_block, 0);
                }
            } else if (cur == TEXT_BLOCK_SIZE) {
                if (cur_block->next && (cur_block->next->len < TEXT_BLOCK_SIZE)) {
                    cur_block = cur_block->next;
                    cur = 0;
                } else {
                    cur_block = split_block(cur_block, TEXT_BLOCK_SIZE)->next;
                    cur = 0;
                }
            } else {
                split_block(cur_block, cur);
            }
        }
        if (cur_block->len == TEXT_BLOCK_SIZE) (*(char *)0) = 100;
        for (i = cur_block->len;i > cur;i--) {
            cur_block->s[i] = cur_block->s[i - 1];
        }
        cur_block->s[cur] = key;
        cur_block->len++;
        cur_x++;
        t->cur_x = cur_x;
        update_cur = TRUE;
    } else 
        switch (key) {
        case KEY_ENTER:
            if (cur == 0) {
                if (!cur_block->prev) {
                    cur_block = split_block(cur_block, 0)->next;
                }
            } else if (cur < TEXT_BLOCK_SIZE || !cur_block->next){
                cur_block = split_block(cur_block, cur)->next;
            } else {
                cur_block = cur_block->next;
            }
            cur_block->prev->next = NULL;
            cur_block->prev = NULL;
            cur_block->up = t->cur_row;
            cur_block->down = t->cur_row->down;
            t->cur_row->down = cur_block;
            if (cur_block->down) {
                cur_block->down->up = cur_block;
            }
            m = t->cur_row->row_no + 1;
            memo_draw_text(w, t->cur_row);
            t->cur_row = cur_block;
            t->cur_x = 0;
            cur_x = 0;
            cur_block->row_no = m;
            cur_block = cur_block->down;
            while (cur_block) {
                m++;
                cur_block->row_no = m;
                memo_draw_text(w, cur_block);
                cur_block = cur_block->down;
            }
            update_cur = TRUE;
            t->row_count++;
            break;

        case KEY_RIGHT:
            if (cur < cur_block->len) {
                cur_x++;
            } else {
                cur_block = cur_block->next;
                if (!cur_block) {
                    if (t->cur_row->down) {
                        t->cur_row = t->cur_row->down;
                        cur_block = t->cur_row;
                        cur_x = 0;
                    } else {
                        DING();
                        return;
                    }
                } else {
                    cur_x++;
                }
            }
            t->cur_x = cur_x;
            update_cur = TRUE;
            break;
        case KEY_PGUP:
            m = t->display_row_count;
            while (m) {
                if (t->display_start_row->up) {
                    t->display_start_row = t->display_start_row->up;
                    t->cur_row = t->cur_row->up;
                    m--;
                    redraw = TRUE;
                } else {
                    break;
                }
            }
            break;
        case KEY_PGDN:
            t_block = t->display_start_row;
            while (t_block->row_no - t->display_start_row->row_no < t->display_row_count - 1) {
                if (!t_block->down) break;
                t_block = t_block->down;
            }
            m = t->display_row_count;
            while (m && t_block->down) {
                t->display_start_row = t->display_start_row->down;
                t_block = t_block->down;
                t->cur_row = t->cur_row->down;
                m--;
                redraw = TRUE;
            }
            break;
        case KEY_UP:
            if (t->cur_row->up) {
                t->cur_row = t->cur_row->up;
                update_cur = TRUE;
            }
            break;
        case KEY_DOWN:
            if (t->cur_row->down) {
                t->cur_row = t->cur_row->down;
                update_cur = TRUE;
            }
            break;
        case KEY_BS:
        case KEY_LEFT:
            if (cur_x) {
                cur_x--;
                t->cur_x = cur_x;
                if (key == KEY_BS) {
                    if (cur == 0) {
                        cur_block = cur_block->prev;
                        cur = cur_block->len - 1;
                    } else cur--;
                    goto del;
                }
                update_cur = TRUE;
                break;
            } else {
                if (!t->cur_row->up) {
                    DING();
                    return;
                } else {
                    t->cur_row = t->cur_row->up;
                    update_cur = TRUE;
                }   
            }   
        case KEY_END:
            m = 0;
            cur = 0;
            for (t_block = t->cur_row;t_block;t_block = t_block->next) {
                m += t_block->len;
                if (!t_block->next) {
                    cur = t_block->len;
                    cur_block = t_block;
                    break;
                }
            }   
            t->cur_x = m;
            cur_x = m;
            if(key != KEY_BS) break;
        case KEY_DEL:
del:
            if (cur == cur_block->len) {
                if (cur_block->next) {
                    cur_block = cur_block->next;
                    cur = 0;
                }
            }
            if (cur < cur_block->len) {
                cur_block->len--;
                if (cur_block->len > 0) {
                    for (i = cur;i < cur_block->len;i++) {
                        cur_block->s[i] = cur_block->s[i + 1];
                    }
                } else {
                    if (cur_block->prev) {
                        cur_block = cur_block->prev;
                        merge_block(cur_block);
                        cur = cur_block->len;
                    }
                }
                t->cur_x = cur_x;
            } else if (t->cur_row->down) {
                cur_block->next = t->cur_row->down;
                t->cur_row->down->prev = cur_block;
                t->cur_row->down = t->cur_row->down->down;
                if(t->cur_row->down) t->cur_row->down->up = t->cur_row;
                cur_block->next->up = NULL;
                cur_block->next->down = NULL;
                merge_block(cur_block);
                for (i = t->cur_row->row_no + 1, t_block = t->cur_row->down;t_block;i++, t_block = t_block->down) {
                    t_block->row_no = i;
                    memo_draw_text(w, t_block);
                }
                t->cur_x = cur_x;
                update_cur = TRUE;
                t->row_count--;
            } else {
                DING();
                return;
            }
            break;
        case KEY_HOME:
            t->cur_x = 0;
            cur_x = 0;
            break;
        case KEY_F4:
            MOVETO(0, 20);
            for (t_block = t->cur_row;t_block;t_block = t_block->next) {
                printf("%d, t_block->len = %d\r\n", t_block->row_no, t_block->len);
            }
            printf("End\r\n");
            break;
        default:
            break;
    }

    memo_adjust(w, cur_x, redraw, update_cur);
    return;
}

void scroll_on_change_memo(window_t *w)
{
    inner_memo_t *t = WINDOW_TO_MEMO(w->parent);
    int new_start_column;
    int new_start_row;
    int re_draw = FALSE;
    if (w->parent->horz_scroll == w) {
        new_start_column = get_scroll_pos(w);
        if (t->display_start_column != new_start_column) {
            t->display_start_column = new_start_column;
            re_draw = TRUE;
        }
    } else if (w->parent->vert_scroll == w) {
        new_start_row = get_scroll_pos(w);
        if (t->display_start_row->row_no != new_start_row) {
            while (t->display_start_row->row_no < new_start_row && t->display_start_row->down) {
                t->display_start_row = t->display_start_row->down;
            }
            while (t->display_start_row->row_no > new_start_row && t->display_start_row->up) {
                t->display_start_row = t->display_start_row->up;
            }
            re_draw = TRUE;
        }
    }
    if (re_draw) {
        draw_memo(w->parent);
        show_window_inner(w->parent);
    }
}

void internal_mouse_down_memo(window_t *w, int key, int x, int y, int ctrl)
{
    inner_memo_t *t = WINDOW_TO_MEMO(w);
    int max_width = memo_max_text_width(w);
    int row_no;
    int column_no;
    if (x < t->px || (x >= w->width - t->px) || y < t->py || (y >= w->height - t->py)) return;
    if (key == MOUSE_LEFT) {
        if (ctrl == 0) {
            row_no = y - t->py + t->display_start_row->row_no;
            column_no = x - t->px + t->display_start_column;
            while (t->cur_row->row_no < row_no && t->cur_row->down) {
                t->cur_row = t->cur_row->down;
            }
            while (t->cur_row->row_no > row_no && t->cur_row->up) {
                t->cur_row = t->cur_row->up;
            }
            t->cur_x = column_no;
            while ((t->display_start_column + t->display_column_count > max_width + 1) && t->display_start_column > 0) t->display_start_column--;
            memo_adjust(w, t->cur_x, FALSE, TRUE);
        }
    }
}

void internal_mouse_wheel_memo(window_t *w, int updown, int x, int y, int ctrl)
{
    inner_memo_t *t = WINDOW_TO_MEMO(w);
    if (updown == WHEEL_UP) {
        if (ctrl == 0)
            set_scroll_pos(w->vert_scroll, t->display_start_row->row_no - 1);
        else if (ctrl == MOUSE_CTRL)
            set_scroll_pos(w->horz_scroll, t->display_start_column - 1);
    } else if (updown == WHEEL_DOWN) {
        if (ctrl == 0)
            set_scroll_pos(w->vert_scroll, t->display_start_row->row_no + 1);
        else if (ctrl == MOUSE_CTRL)
            set_scroll_pos(w->horz_scroll, t->display_start_column + 1);
    }
}

text_t *create_memo(window_t *parent, int x, int y, int width, int height)
{
    window_t *w = create_window(parent, draw_memo, sizeof(window_t) + sizeof(inner_memo_t));
    scroll_t *scroll;
    inner_memo_t *t = WINDOW_TO_MEMO(w);
    set_win_style(w, WIN_STYLE_NONE);
    set_win_size(w, x, y, width, height);
    w->can_focus = TRUE;
    w->pf_show_cur = show_cur_memo;
    w->pf_internal_key = internal_key_memo;
    w->pf_internal_mouse_down = internal_mouse_down_memo;
    w->pf_internal_mouse_wheel = internal_mouse_wheel_memo;
    t->display_start_row = &t->rows;
    t->cur_row = &t->rows;
    t->row_count = 1;
    redraw_window(w);
    scroll = create_scroll(w, SCROLL_HORIZONAL);
    set_win_on_change(scroll, scroll_on_change_memo);
    scroll = create_scroll(w, SCROLL_VERTICAL);
    set_win_on_change(scroll, scroll_on_change_memo);

    return w;
}

static int memo_adjust_line(window_t *w, text_block_t *row)
{
    inner_memo_t *t = WINDOW_TO_MEMO(w);
    if (row->row_no >= t->display_start_row->row_no &&
            row->row_no < t->display_start_row->row_no + t->display_row_count) {
        set_scroll_count(w->horz_scroll, memo_max_text_width(w) - w->width + 4);
        memo_draw_text(w, row);
        show_window_inner(w);
        return OK;
    }
    return NO;
}

static void memo_set_row_text(text_block_t *row, char *text)
{
    text_block_t *t_block = row;
    int len = strlen(text);
    int n = len / TEXT_BLOCK_SIZE;
    while (n > 0) {
        memcpy(t_block->s, text, TEXT_BLOCK_SIZE);
        t_block->len = TEXT_BLOCK_SIZE;
        n--;
        text += TEXT_BLOCK_SIZE;
        if (!n) break;
        if (!t_block->next) {
            t_block->next = (text_block_t *)MALLOC(sizeof(text_block_t));
            memset(t_block->next, 0x00, sizeof(text_block_t));
            t_block->next->prev = t_block;
        }
        t_block = t_block->next;
    }
    if (*text) {
        if (!t_block->next) {
            t_block->next = (text_block_t *)MALLOC(sizeof(text_block_t));
            memset(t_block->next, 0x00, sizeof(text_block_t));
        }
        t_block->next->prev = t_block;
        len = strlen(text);
        memcpy(t_block->s, text, len);
        t_block->len = len;
    }
    /* release the rest of the blocks */
    t_block = t_block->next;
    while (t_block) {
        row = t_block->next;
        FREE(t_block);
        t_block = row;
    }
}

void memo_add_line(window_t *w, char *text)
{
    inner_memo_t *t = WINDOW_TO_MEMO(w);
    text_block_t *row;
    for (row = t->cur_row;row->down;row = row->down);
    row->down = (text_block_t *)MALLOC(sizeof(text_block_t));
    memset(row->down, 0x00, sizeof(text_block_t));
    row->down->up = row;
    row = row->down;
    row->row_no = t->row_count;
    t->row_count++;
    
    memo_set_row_text(row, text);
    set_scroll_count(w->vert_scroll, t->row_count - w->height + 3);
    memo_adjust_line(w, row);
}

void memo_insert_line(window_t *w, int line_no, char *text)
{
    inner_memo_t *t = WINDOW_TO_MEMO(w);
    text_block_t *row = &t->rows;
    text_block_t *t_block;

    while (row && row->row_no < line_no) row = row->down;
    if (!row) return;
    split_block(row, 0);
    row->next->prev = NULL;
    row->next->up = row;
    row->next->down = row->down;
    if (row->down) {
        row->down->up = row->next;
    }
    row->down = row->next;
    row->next = NULL;
    memo_set_row_text(row, text);

    t_block = row->down;
    while (t_block) {
        t_block->row_no = ++line_no;
        t_block = t_block->down;
    }
    t->row_count++;

    set_scroll_count(w->vert_scroll, t->row_count - w->height + 3);
    if (row->row_no < t->display_start_row->row_no + t->display_row_count) {
        if (row->row_no < t->display_start_row->row_no) {
            t->display_start_row = t->display_start_row->up;
        }
        set_scroll_count(w->horz_scroll, memo_max_text_width(w) - w->width + 4);
        for (;row && row->row_no < t->display_start_row->row_no + t->display_row_count;row = row->down) {
            memo_draw_text(w, row);
        }
        show_window_inner(w);
        return;
    }
}

void memo_set_line_text(window_t *w, int line_no, char *text)
{
    inner_memo_t *t = WINDOW_TO_MEMO(w);
    text_block_t *row = &t->rows;
    while (row && row->row_no < line_no) row = row->down;
    if (!row) return;
    memo_set_row_text(row, text);
    memo_adjust_line(w, row);
}

void memo_del_line(window_t *w, int line_no)
{
    inner_memo_t *t = WINDOW_TO_MEMO(w);
    text_block_t *row = &t->rows;
    text_block_t *t_block, *p;
    while (row && row->row_no < line_no) row = row->down;
    if (!row) return;
    t_block = row->next;
    while (t_block) {
        p = t_block->next;
        FREE(t_block);
        t_block = p;
    }
    row->len = 0;
    row->next = row->down;
    if (row->down) {
        row->down->prev = row;
        row->down = row->down->down;
        row->next->up = NULL;
        row->next->down = NULL;
        if (row->down) row->down->up = row;

        if (t->cur_row == row->next) {
            if (row->down) t->cur_row = row->down;
            else t->cur_row = row;
        }
        if (t->display_start_row == row->next) {
            if (row->down) t->display_start_row = row->down;
            else t->display_start_row = row;
        }

        merge_block(row);
        t_block = row->down;
        while (t_block) {
            t_block->row_no = ++line_no;
            t_block = t_block->down;
        }
        t->row_count--;
    } else if (row->up) {
        t->row_count--;
        if (t->cur_row == row) t->cur_row = row->up;
        if (t->display_start_row == row) t->display_start_row = row->up;
        row = row->up;
        FREE(row->down);
        row->down = NULL;
    }
    set_scroll_count(w->vert_scroll, t->row_count - w->height + 3);
    if (row->row_no < t->display_start_row->row_no + t->display_row_count) {
        if (row->row_no < t->display_start_row->row_no) {
            t_block = t->display_start_row;
            while (t_block && t_block->row_no - t->display_start_row->row_no + 1 < t->display_row_count) t_block = t_block->down;
            if (t_block->down) t->display_start_row = t->display_start_row->down;
            row = t->display_start_row;
        }
        for (;row && row->row_no < t->display_start_row->row_no + t->display_row_count;row = row->down) {
            memo_draw_text(w, row);
        }
        set_scroll_count(w->horz_scroll, memo_max_text_width(w) - w->width + 4);
        show_window_inner(w);
    }
}
