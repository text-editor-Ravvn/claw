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

    /* Normal backspace inside a line */
    if (cursor.x > 0)
    {
        memmove(
            &row->chars[cursor.x - 1],
            &row->chars[cursor.x],
            row->size - cursor.x + 1
        );

        row->size--;

        row->chars[row->size] = '\0';

        cursor.x--;

        return;
    }

    /* Beginning of first line */
    if (cursor.y == 0)
        return;

    /* Merge with previous line */
    Row *current = &buffer.rows[cursor.y];
    Row *previous = &buffer.rows[cursor.y - 1];

    int oldSize = previous->size;

    previous->chars = realloc(
        previous->chars,
        previous->size + current->size + 1
    );

    memcpy(
        &previous->chars[previous->size],
        current->chars,
        current->size
    );

    previous->size += current->size;

    previous->chars[previous->size] = '\0';

    free(current->chars);

    for (int i = cursor.y; i < buffer.numRows - 1; i++)
    {
        buffer.rows[i] = buffer.rows[i + 1];
    }

    buffer.numRows--;

    buffer.rows = realloc(
        buffer.rows,
        sizeof(Row) * buffer.numRows
    );

    cursor.y--;

    cursor.x = oldSize;
}
void insertNewLine(void)
{
    if (cursor.y >= buffer.numRows)
        return;

    Row *current = &buffer.rows[cursor.y];

    int leftSize = cursor.x;
    int rightSize = current->size - cursor.x;

    char *rightPart = malloc(rightSize + 1);

    memcpy(
        rightPart,
        &current->chars[cursor.x],
        rightSize
    );

    rightPart[rightSize] = '\0';

    current->size = leftSize;

    current->chars[leftSize] = '\0';

    buffer.rows = realloc(
        buffer.rows,
        sizeof(Row) * (buffer.numRows + 1)
    );

    for (int i = buffer.numRows; i > cursor.y + 1; i--)
    {
        buffer.rows[i] = buffer.rows[i - 1];
    }

    buffer.rows[cursor.y + 1].size = rightSize;

    buffer.rows[cursor.y + 1].chars = rightPart;

    buffer.numRows++;

    cursor.y++;

    cursor.x = 0;
}