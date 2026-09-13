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

#define MOD_CTRL  1
#define MOD_ALT   2
#define MOD_SHIFT 4

KeyEvent readKey(void);

#endif