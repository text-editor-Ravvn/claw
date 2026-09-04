#include <stdlib.h>

#include "buffer.h"

Buffer buffer;

void bufferInit(void)
{
    /* Always start with one empty row so editing has a valid target. */
    buffer.numRows = 1;
    buffer.endsWithNewline = 0;
    buffer.modified = 0;

    buffer.rows = malloc(sizeof(Row));

    if (!buffer.rows)
    {
        buffer.numRows = 0;
        return;
    }

    buffer.rows[0].size = 0;

    buffer.rows[0].chars = malloc(1);

    if (!buffer.rows[0].chars)
    {
        free(buffer.rows);
        buffer.rows = NULL;
        buffer.numRows = 0;
        return;
    }

    buffer.rows[0].chars[0] = '\0';
}

void bufferFree(void)
{
    /* Rows own their character arrays, so release them before the row list. */
    if (!buffer.rows)
        return;

    for (int i = 0; i < buffer.numRows; i++)
        free(buffer.rows[i].chars);

    free(buffer.rows);
    buffer.rows = NULL;
    buffer.numRows = 0;
}