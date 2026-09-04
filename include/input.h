#ifndef INPUT_H
#define INPUT_H
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