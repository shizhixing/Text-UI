#ifndef INPUT_H
#define INPUT_H
#define KEY_ESC 0x1b 
#define KEY_F1 0x5b31317e
#define KEY_F2 0x5b31327e
#define KEY_F3 0x5b31337e
#define KEY_F4 0x5b31347e
#define KEY_F5 0x5b31357e
#define KEY_F6 0x5b31377e 
#define KEY_F7 0x5b31387e 
#define KEY_F8 0x5b31397e 
#define KEY_F9 0x5b32307e 
#define KEY_F10 0x5b32317e 
#define KEY_F11 0x5b32337e 
#define KEY_F12 0x5b32347e 
#define KEY_HOME 0x5b317e 
#define KEY_END 0x5b347e 
#define KEY_PGUP 0x5b357e 
#define KEY_PGDN 0x5b367e 
#define KEY_INSERT 0x5b327e 
#define KEY_DEL 0x7f 
#define KEY_UP 0x5b41 
#define KEY_DOWN 0x5b42 
#define KEY_LEFT 0x5b44 
#define KEY_RIGHT 0x5b43
#define KEY_BS 0x8
#define KEY_ENTER 0xa

#define IS_MOUSE_EVENT(key) (((key) >> 24) == 0x4d)
#define MOUSE_LEFT 0
#define MOUSE_MID 1
#define MOUSE_RIGHT 2
#define MOUSE_KEY_UP 3

#define MOUSE_DOWN 0x20
#define MOUSE_DRAG 0x40
#define MOUSE_WHEEL 0x60

#define MOUSE_SHIFT 0x4
#define MOUSE_ALT 0x08
#define MOUSE_CTRL 0x10

#define WHEEL_UP 0x0
#define WHEEL_DOWN 0x1

unsigned int get_key();
void setup_key(void);
void restore_key(void);
#endif
