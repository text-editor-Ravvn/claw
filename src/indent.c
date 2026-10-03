#include <string.h>
#include <stdlib.h>

#include "indent.h"
#include "buffer.h"
#include "cursor.h"

extern Buffer buffer;

#define TAB_WIDTH 4

int getIndentationLevel(const char *line)
{
    int count = 0;

    while (
        line[count] == ' ' ||
        line[count] == '\t'
    )
    {
        count++;
    }

    return count;
}

void indentLine(void)
{
    if (
        cursor.y < 0 ||
        cursor.y >= buffer.numRows
    )
        return;

    Row *row = &buffer.rows[cursor.y];

    char *newChars =
        realloc(
            row->chars,
            (size_t)row->size +
            TAB_WIDTH + 1
        );

    if (!newChars)
        return;

    row->chars = newChars;

    memmove(
        row->chars + TAB_WIDTH,
        row->chars,
        (size_t)row->size + 1
    );

    memset(
        row->chars,
        ' ',
        TAB_WIDTH
    );

    row->size += TAB_WIDTH;

    cursor.x += TAB_WIDTH;

    if (cursor.x > row->size)
        cursor.x = row->size;

    buffer.modified = 1;
}

void unindentLine(void)
{
    if (
        cursor.y < 0 ||
        cursor.y >= buffer.numRows
    )
        return;

    Row *row = &buffer.rows[cursor.y];

    int removeCount = 0;

    while (
        removeCount < TAB_WIDTH &&
        removeCount < row->size &&
        row->chars[removeCount] == ' '
    )
    {
        removeCount++;
    }

    if (removeCount == 0)
        return;

    memmove(
        row->chars,
        row->chars + removeCount,
        (size_t)(row->size - removeCount + 1)
    );

    row->size -= removeCount;

    if (cursor.x >= removeCount)
        cursor.x -= removeCount;
    else
        cursor.x = 0;

    buffer.modified = 1;
}