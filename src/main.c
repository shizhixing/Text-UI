#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include "window.h"
#include "input.h"
#include "cursor.h"

int screen_width;
int screen_height;
void sig_resize( int signo ) 
{
    struct winsize size;
    if (ioctl(0, TIOCGWINSZ, &size) < 0) {
        printf("Error\n");
    } else {
        screen_width = size.ws_col;
        screen_height = size.ws_row;
        refresh_screen();
    }
}
void user_main();
int main()
{
    sig_resize(0);
    signal(SIGWINCH, sig_resize);
    init_scr();
    setup_key();
    user_main();
    update_screen();
    do_modal(NULL);
    restore_key();
    end_scr();

    return 0;
}
