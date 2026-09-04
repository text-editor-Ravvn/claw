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

int readKey(void);

#endif