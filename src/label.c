#include <stdio.h>
#include <string.h>
#include "window_type.h"
#include "cursor.h"

void draw_label(window_t *w) 
{
    win_printf(w, 0, 0, WHITE, BLACK, 0, "%s", w->text);
}

label_t *create_label(window_t *parent, int x, int y, char *text)
{
    window_t *w = create_window(parent, draw_label, sizeof(window_t));
    set_win_style(w, WIN_STYLE_NONE);
    strncpy(w->text, text, sizeof(w->text));
    set_win_size(w, x, y, strlen(w->text), 1); 
    redraw_window(w);
    return w;
}
