#include <string.h>
#include <stdlib.h>

#include "indent.h"
#include "buffer.h"
#include "cursor.h"
#include "config.h"

extern Buffer buffer;

static int getTabWidth(void)
{
    int w = configTabWidth();
    return w > 0 ? w : 4;
}

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
    int tabWidth = getTabWidth();

    char *newChars =
        realloc(
            row->chars,
            (size_t)row->size +
            tabWidth + 1
        );

    if (!newChars)
        return;

    row->chars = newChars;

    memmove(
        row->chars + tabWidth,
        row->chars,
        (size_t)row->size + 1
    );

    memset(
        row->chars,
        ' ',
        tabWidth
    );

    row->size += tabWidth;

    cursor.x += tabWidth;

    if (cursor.x > row->size)
        cursor.x = row->size;

    free(row->hl);
    row->hl = NULL;
    row->hlSize = 0;

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
    int tabWidth = getTabWidth();

    int removeCount = 0;

    while (
        removeCount < tabWidth &&
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

    free(row->hl);
    row->hl = NULL;
    row->hlSize = 0;

    buffer.modified = 1;
}