#include <sys/ioctl.h>
#include <unistd.h>

#include "viewport.h"
#include "cursor.h"
#include "config.h"
#include "buffer.h"
#include "layout.h"
#include "split.h"
#include "terminal.h"
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
    int termRows = terminalIsActive() ? integratedTerminal.height : 0;
    int availRows = viewport.screenRows - 2 - termRows;
    int availCols = viewport.screenCols - viewport.gutterWidth;

    if (splitIsActive())
    {
        EditorPane *ap = splitGetActivePane();
        if (ap)
        {
            availRows = ap->height;
            availCols = ap->width - viewport.gutterWidth;
        }
    }

    if (availRows < 1)
        availRows = 1;
    if (availCols < 1)
        availCols = 1;

    /* Vertical scrolling */
    if (cursor.y < viewport.rowOffset)
        viewport.rowOffset = cursor.y;

    if (cursor.y >= viewport.rowOffset + availRows)
    {
        viewport.rowOffset = cursor.y - availRows + 1;
    }

    /* Horizontal scrolling */
    if (cursor.x < viewport.colOffset + HSCROLL_MARGIN)
    {
        viewport.colOffset = cursor.x - HSCROLL_MARGIN;
        if (viewport.colOffset < 0)
            viewport.colOffset = 0;
    }

    if (cursor.x >= viewport.colOffset + availCols - HSCROLL_MARGIN)
    {
        viewport.colOffset = cursor.x - availCols + HSCROLL_MARGIN;
        if (viewport.colOffset < 0)
            viewport.colOffset = 0;
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
    if (layoutState.showLineNumbers)
    {
        int digits = digitCount(buffer.numRows);

        if (digits < 2)
            digits = 2;

        /* digits + one space + separator bar + one space */
        viewport.gutterWidth = digits + 1;
    }
    else
    {
        viewport.gutterWidth = 0;
    }
}