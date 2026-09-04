#include "cursor.h"
#include "buffer.h"

Cursor cursor = {0, 0};

extern Buffer buffer;

void moveCursorLeft(void)
{
    if (cursor.x > 0)
        cursor.x--;
}

void moveCursorRight(void)
{
    if (cursor.y >= buffer.numRows)
        return;

    Row *row = &buffer.rows[cursor.y];

    if (cursor.x < row->size)
        cursor.x++;
}

void moveCursorUp(void)
{
    if (buffer.numRows == 0 || !buffer.rows)
        return;

    if (cursor.y > 0)
        cursor.y--;

    Row *row = &buffer.rows[cursor.y];

    if (cursor.x > row->size)
        cursor.x = row->size;
}

void moveCursorDown(void)
{
    if (buffer.numRows == 0 || !buffer.rows)
        return;

    if (cursor.y < buffer.numRows - 1)
        cursor.y++;

    Row *row = &buffer.rows[cursor.y];

    if (cursor.x > row->size)
        cursor.x = row->size;
}