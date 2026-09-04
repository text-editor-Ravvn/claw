#include <stdio.h>

#include "render.h"
#include "cursor.h"
#include "buffer.h"

extern Buffer buffer;
extern Cursor cursor;

void refreshScreen(void)
{
    printf("\033[2J");
    printf("\033[H");

    printf("Claw Editor v0.5\n\n");

    for (int i = 0; i < buffer.numRows; i++)
    {
        if (buffer.rows[i].chars)
        {
            fwrite(
                buffer.rows[i].chars,
                1,
                buffer.rows[i].size,
                stdout
            );
        }

        printf("\n");
    }

    printf("\n");

    printf("Cursor X: %d\n", cursor.x);
    printf("Cursor Y: %d\n", cursor.y);

    fflush(stdout);
}