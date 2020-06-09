#ifndef WINDOW_H
#define WINDOW_H
typedef struct pixel_t pixel_t;
typedef struct window_t window_t;
typedef window_t label_t;
typedef window_t text_t;
typedef window_t memo_t;
typedef window_t button_t;
typedef window_t scroll_t;
typedef window_t listview_t;

void init_scr();
void end_scr();
window_t *create_window(window_t *parent, void *draw_handle, int total_size);
void destroy_window(window_t *w);
void set_win_size(window_t *w, int left, int top, int width, int height);
int get_win_size(window_t *w, int *left, int *top, int *width, int *height);
pixel_t get_pixel(window_t *w, int x, int y);
void update_screen();
void refresh_screen();
void bring_win_top(window_t *w);
int set_focus_window(window_t *w);
int is_window_focused(window_t *w);
void show_window(window_t *w);
void flash_window(window_t *w);
int close_window(window_t *w);
int do_modal(window_t *w);
window_t *get_window(int x, int y);
window_t *get_parent(window_t *w);
void set_win_style(window_t *w, int style);
void set_win_text(window_t *w, char *text);
char *get_win_text(window_t *w);
void draw_table_line(window_t *w, int ox, int oy, int dx, int dy, int color, int bgcolor);
void draw_table_rectangle(window_t *w, int left, int top, int right, int bottom, int color, int bgcolor);
int window_t_size();
void redraw_window(window_t *w);

void set_win_user_data(window_t *w, void *user_data);
void *get_win_user_data(window_t *w);


void set_win_on_resize(window_t *w, void (*pf_on_resize)(window_t *w, int width, int height));
void set_win_on_click(window_t *w, void (*pf_on_click)(window_t *w));
void set_win_on_change(window_t *w, void (*pf_on_change)(window_t *w));

window_t *create_normal_window(window_t *parent);
label_t *create_label(window_t *parent, int x, int y, char *text);
text_t *create_text(window_t *parent, int x, int y, int width);
memo_t *create_memo(window_t *parent, int x, int y, int width, int height);
void memo_add_line(window_t *w, char *text);
void memo_insert_line(window_t *w, int line_no, char *text);
void memo_set_line_text(window_t *w, int line_no, char *text);
void memo_del_line(window_t *w, int line_no);
button_t *create_button(window_t *parent, int x, int y, int width);
void set_button_edge(window_t *w, char normal_left, char normal_right, char press_left, char press_right);
scroll_t *create_scroll(window_t *parent, int direction);
int get_scroll_pos(window_t *w);
int get_scroll_count(window_t *w);
void set_scroll_pos(window_t *w, int pos);
void set_scroll_count(window_t *w, int count);

text_t *create_listview(window_t *parent, int x, int y, int width, int height);
void listview_add_column(window_t *w, char *text, int width);


enum {
    WIN_STYLE_NONE  = 0,
    WIN_STYLE_SIZEABLE,
};

#define SCROLL_HORIZONAL 0
#define SCROLL_VERTICAL 1

#endif
