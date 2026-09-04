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

    char *newChars = realloc(row->chars, (size_t)row->size + 2);

    if (!newChars)
        return;

    row->chars = newChars;

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

    char *newChars = realloc(
        previous->chars,
        (size_t)previous->size + current->size + 1
    );

    if (!newChars)
        return;

    previous->chars = newChars;

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

    Row *newRows = realloc(buffer.rows, sizeof(Row) * buffer.numRows);

    if (newRows)
        buffer.rows = newRows;

    cursor.y--;

    cursor.x = oldSize;
}

void deleteForward(void)
{
    if (cursor.y < 0 || cursor.y >= buffer.numRows || !buffer.rows)
        return;

    Row *row = &buffer.rows[cursor.y];

    if (cursor.x < row->size)
    {
        memmove(
            &row->chars[cursor.x],
            &row->chars[cursor.x + 1],
            (size_t)(row->size - cursor.x)
        );
        row->size--;
        return;
    }

    if (cursor.y < buffer.numRows - 1)
    {
        Row *next = &buffer.rows[cursor.y + 1];
        int oldSize = row->size;
        char *newChars = realloc(
            row->chars,
            (size_t)row->size + next->size + 1
        );

        if (!newChars)
            return;

        row->chars = newChars;
        memcpy(&row->chars[row->size], next->chars, (size_t)next->size);
        row->size += next->size;
        row->chars[row->size] = '\0';
        free(next->chars);

        for (int i = cursor.y + 1; i < buffer.numRows - 1; i++)
            buffer.rows[i] = buffer.rows[i + 1];

        buffer.numRows--;
        Row *newRows = realloc(buffer.rows, sizeof(Row) * buffer.numRows);
        if (newRows)
            buffer.rows = newRows;
        cursor.x = oldSize;
    }
}

void insertNewLine(void)
{
    if (cursor.y >= buffer.numRows)
        return;

    Row *current = &buffer.rows[cursor.y];

    if (cursor.x < 0)
        cursor.x = 0;
    if (cursor.x > current->size)
        cursor.x = current->size;

    int leftSize = cursor.x;
    int rightSize = current->size - cursor.x;

    char *rightPart = malloc((size_t)rightSize + 1);

    if (!rightPart)
        return;

    memcpy(
        rightPart,
        &current->chars[cursor.x],
        rightSize
    );

    rightPart[rightSize] = '\0';

    current->size = leftSize;

    current->chars[leftSize] = '\0';

    Row *newRows = realloc(
        buffer.rows,
        sizeof(Row) * (buffer.numRows + 1)
    );

    if (!newRows)
    {
        free(rightPart);
        return;
    }

    buffer.rows = newRows;

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