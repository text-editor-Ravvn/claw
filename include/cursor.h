#ifndef CURSOR_H
#define CURSOR_H

typedef struct
{
    /* Zero-based column and row within the text buffer. */
    int x;
    int y;
} Cursor;

extern Cursor cursor;
extern int bracketMatchX;
extern int bracketMatchY;

/* Move without allowing the cursor to leave the current document. */
void moveCursorLeft(void);
void moveCursorRight(void);
void moveCursorUp(void);
void moveCursorDown(void);

#endif