#include <stdlib.h>

#include "buffer.h"

Buffer buffer;

void bufferInit(void)
{
    buffer.numRows = 1;

    buffer.rows = malloc(sizeof(Row));

    buffer.rows[0].size = 0;

    buffer.rows[0].chars = malloc(1);

    buffer.rows[0].chars[0] = '\0';
}