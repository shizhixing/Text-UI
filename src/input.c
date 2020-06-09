#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <termios.h>
#include "input.h"

void cfmakeraw(struct termios *termios_p);

unsigned int get_key()
{
    unsigned int r = 0;
    int i;
    int c;
    int alt = 0;
    for (;;) {
        c = getchar();
//        printf("{%x    %c}\r\n", c, c);
//        if (c == 'q') return c;
//        continue;
        if (c == -1) continue;
        if (c == '\033') {/* ESC */
            for (;;) {
                c = getchar();
                if (c != '\033') break;
                alt = 0x80000000;
            }
            if (c == '[' || c == 'O') {
                r = c;
                c = getchar();
                if (c != 0x4d) {/* key */
                    while (c != -1 && c != 0x41 && c != 0x42 && c != 0x43 && c != 0x44 && c != 0x45 && c != 'Z' && c != 0x7e) {
                        r = (r << 8) | c;
                        c  = getchar();
                    }
                    if (c == -1) r = 0;
                    else r = (r << 8) | c;
                } else {/* mouse */
                    r = c;
                    for (i = 0;i < 3 && c != -1;i++) {
                        c = getchar();
                        r = (r << 8) | c;
                    }
                    if (c == -1) r = 0;
                }
            } else {
                r = 0x80000000 + c;
            }
            switch (r) {
                case 0x5b5b41:
                    r = KEY_F1;
                    break;
                case 0x5b5b42:
                    r = KEY_F2;
                    break;
                case 0x5b5b43:
                    r = KEY_F3;
                    break;
                case 0x5b5b44:
                    r = KEY_F4;
                    break;
                case 0x5b5b45:
                    r = KEY_F5;
                    break;
                case 0x5b337e:
                    r = KEY_DEL;
                    break;
                default:
                    break;
            }
            r = r + alt;
            if (r) return r;
            return '\033';
            break;
        } else if (c == 0x7f) {
             return 8;
        } else if (c > 0 && c <= 127 ){
            if (c == 0xd) c = 0xa;
            break;
        }
    }
    return c;
}

static struct termios stored_settings;
void setup_key(void)
{
    struct termios new_settings;
    tcgetattr (0, &stored_settings);
    new_settings = stored_settings;
    cfmakeraw(&new_settings);
    tcsetattr(0, TCSANOW, &new_settings);
}
void restore_key(void)
{
    tcsetattr (0, TCSANOW, &stored_settings);
}
