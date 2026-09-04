#ifndef INPUT_H
#define INPUT_H

enum EditorKey
{
    ARROW_LEFT = 1000,
    ARROW_RIGHT,
    ARROW_UP,
    ARROW_DOWN
};

int readKey(void);

#endif