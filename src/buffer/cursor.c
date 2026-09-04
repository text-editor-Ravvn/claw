#include "cursor.h"
#include "buffer.h"

Cursor cursor = {0, 0};

extern Buffer buffer;

void moveCursorLeft(void)
{
    /* Horizontal movement is limited to the beginning of the current row. */
    if (cursor.x > 0)
        cursor.x--;
}

void moveCursorRight(void)
{
    /* The cursor may sit immediately after the final character. */
    if (cursor.y >= buffer.numRows)
        return;

    Row *row = &buffer.rows[cursor.y];

    if (cursor.x < row->size)
        cursor.x++;
}

void moveCursorUp(void)
{
    /* Preserve the column where possible, clamping on shorter rows. */
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
    /* Preserve the column where possible, clamping on shorter rows. */
    if (buffer.numRows == 0 || !buffer.rows)
        return;

    if (cursor.y < buffer.numRows - 1)
        cursor.y++;

    Row *row = &buffer.rows[cursor.y];

    if (cursor.x > row->size)
        cursor.x = row->size;
}