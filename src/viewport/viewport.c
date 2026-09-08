#include <sys/ioctl.h>
#include <unistd.h>

#include "viewport.h"
#include "cursor.h"
#include "config.h"
#include "buffer.h"
#define HSCROLL_MARGIN 10
extern Cursor cursor;
extern Buffer buffer;
Viewport viewport;

static void getWindowSize(int *rows, int *cols)
{
    struct winsize ws;

    if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws) == -1)
    {
        *rows = 24;
        *cols = 80;
        return;
    }

    *rows = ws.ws_row;
    *cols = ws.ws_col;
}

void viewportInit(void)
{
    getWindowSize(
        &viewport.screenRows,
        &viewport.screenCols
    );

    viewport.rowOffset = 0;
    viewport.colOffset = 0;
    viewport.gutterWidth = 0;
}

void scrollEditor(void)
{
    /* Vertical scrolling */

    if (cursor.y < viewport.rowOffset)
        viewport.rowOffset = cursor.y;

    if (cursor.y >= viewport.rowOffset +
                    viewport.screenRows - 2)
    {
        viewport.rowOffset =
            cursor.y - viewport.screenRows + 3;
    }

    /* Horizontal scrolling */

    /* Move viewport left */
if (cursor.x < viewport.colOffset + HSCROLL_MARGIN)
{
    viewport.colOffset =
        cursor.x - HSCROLL_MARGIN;

    if (viewport.colOffset < 0)
        viewport.colOffset = 0;
}

/* Move viewport right */
if (cursor.x >= viewport.colOffset +
                viewport.screenCols -
                viewport.gutterWidth -
                HSCROLL_MARGIN)
{
    viewport.colOffset =
        cursor.x -
        viewport.screenCols +
        viewport.gutterWidth +
        HSCROLL_MARGIN;
}
}

static int digitCount(int n)
{
    int count = 1;

    while (n >= 10)
    {
        n /= 10;
        count++;
    }

    return count;
}

void viewportUpdateSize(void)
{
    getWindowSize(
        &viewport.screenRows,
        &viewport.screenCols
    );

    /* Compute gutter width: digits for the largest line number + separator. */
    if (configShowLineNumbers())
    {
        int digits = digitCount(buffer.numRows);

        if (digits < 3)
            digits = 3;

        /* digits + one space + separator bar + one space */
        viewport.gutterWidth = digits + 2;
    }
    else
    {
        viewport.gutterWidth = 0;
    }
}