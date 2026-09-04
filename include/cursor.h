#ifndef CURSOR_H
#define CURSOR_H

typedef struct
{
    int x;
    int y;
} Cursor;

extern Cursor cursor;

void moveCursorLeft(void);
void moveCursorRight(void);
void moveCursorUp(void);
void moveCursorDown(void);

#endif