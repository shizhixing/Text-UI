#include <stdio.h>
#include "window.h"

void bclick(window_t *w)
{
    close_window(get_parent(w));
}
window_t *memo, *listview;
void on_click(window_t *w)
{
    window_t *p = get_parent(w);
    int left, top, width, height;
    window_t *b;
    w = create_normal_window(p);
    set_win_text(w, "Test2");
    get_win_size(w, &left, &top, &width, &height);
    set_win_size(w, left, top, 40, 10);
    p = create_text(w, 1, 2, 15);
    set_win_text(p, "Hello World");
    b = create_button(w, 1, 5, 6);
    set_win_text(b, "OK");
    set_win_on_click(b, bclick);
    do_modal(w);
    memo_insert_line(memo, 0, get_win_text(p)); 
    destroy_window(w);
}

void on_click2(window_t *w)
{
    memo_del_line(memo, 0);
}

void user_main()
{
    int left, top, width, height;
    window_t *w, *btn;
    w = create_normal_window(0);
    set_win_text(w, "Test1");
    create_label(w, 1, 2, "Label12345678901234567890");
    show_window(w);
    w = create_normal_window(0);
    get_win_size(w, &left, &top, &width, &height);
    set_win_size(w, left, top, 170, 40);
    set_win_text(w, "Test3");
    show_window(w);
    memo = create_memo(w, 1, 2, 80, 20);
    btn = create_button(w, 1, 26, 6);
    set_win_text(btn, "OK");
    set_win_on_click(btn, on_click);
    show_window(btn);
    btn = create_button(w, 6, 26, 6);
    set_win_text(btn, "DEL");
    set_win_on_click(btn, on_click2);
    show_window(btn);
    listview = create_listview(w, 83, 2, 80, 20);
    show_window(listview);
    listview_add_column(listview, "Head1", 20);
    listview_add_column(listview, "Head2", 20);
    listview_add_column(listview, "Head3", 20);
    listview_add_column(listview, "Head4", 20);
    listview_add_column(listview, "Head5", 20);
    listview_add_column(listview, "Head6", 20);
}
