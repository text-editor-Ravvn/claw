#ifndef INPUT_H
#define INPUT_H
/* Convert an alphabetic key into its terminal control-code value. */
#define CTRL_KEY(k) ((k) & 0x1f)
enum EditorKey
{
    ARROW_LEFT = 1000,
    ARROW_RIGHT,
    ARROW_UP,
    ARROW_DOWN,
    DELETE_KEY,
    HOME_KEY,
    END_KEY,
    PAGE_UP,
    PAGE_DOWN,
    BACKTAB,
    KEY_F1,
    KEY_F2,
    KEY_F3,
    KEY_F4,
    KEY_F5,
    KEY_F6,
    KEY_F7,
    KEY_F8,
    KEY_F9,
    KEY_F10,
    KEY_F11,
    KEY_F12
};

typedef struct
{
    int key;
    int modifiers;
} KeyEvent;

/* MOD_CTRL, MOD_ALT, MOD_SHIFT are defined in keymap.h */
#include "keymap.h"

KeyEvent decodeKeyChar(char c);
KeyEvent readKey(void);

#endif