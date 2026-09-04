#include <stdlib.h>
#include <string.h>

#include "buffer.h"
#include "cursor.h"

extern Buffer buffer;

void insertChar(char c)
{
    if (cursor.y >= buffer.numRows)
        return;

    Row *row = &buffer.rows[cursor.y];

    if (cursor.x > row->size)
        cursor.x = row->size;

    row->chars = realloc(
        row->chars,
        row->size + 2
    );

    memmove(
        &row->chars[cursor.x + 1],
        &row->chars[cursor.x],
        row->size - cursor.x + 1
    );

    row->chars[cursor.x] = c;

    row->size++;

    row->chars[row->size] = '\0';

    cursor.x++;
}

void deleteChar(void)
{
    if (cursor.y >= buffer.numRows)
        return;

    Row *row = &buffer.rows[cursor.y];

    if (cursor.x <= 0)
        return;

    memmove(
        &row->chars[cursor.x - 1],
        &row->chars[cursor.x],
        row->size - cursor.x + 1
    );

    row->size--;

    row->chars[row->size] = '\0';

    cursor.x--;
}
void insertNewLine(void)
{
    buffer.rows = realloc(
        buffer.rows,
        sizeof(Row) * (buffer.numRows + 1)
    );

    for (int i = buffer.numRows; i > cursor.y + 1; i--)
    {
        buffer.rows[i] = buffer.rows[i - 1];
    }

    buffer.rows[cursor.y + 1].size = 0;

    buffer.rows[cursor.y + 1].chars = malloc(1);

    buffer.rows[cursor.y + 1].chars[0] = '\0';

    buffer.numRows++;

    cursor.y++;

    cursor.x = 0;
}