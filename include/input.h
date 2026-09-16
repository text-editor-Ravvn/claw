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
    DELETE_KEY
    
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