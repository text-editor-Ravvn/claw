#include <stdio.h>

#include "render.h"
#include "buffer.h"
#include "editor.h"
#include "cursor.h"
#include "statusbar.h"

extern Buffer buffer;
extern Cursor cursor;

void refreshScreen(void)
{
    printf("\033[?25l");
    printf("\033[H");
    printf("\033[J");

    for (int i = 0; i < buffer.numRows; i++)
    {
        printf("\033[K");

        if (buffer.rows[i].chars)
        {
            fwrite(
                buffer.rows[i].chars,
                1,
                buffer.rows[i].size,
                stdout
            );
        }

        printf("\r\n");
    }

    printf("\033[K");

    drawStatusBar();

    printf("\033[%d;%dH",
           cursor.y + 1,
           cursor.x + 1);

    printf("\033[?25h");
    fflush(stdout);
}