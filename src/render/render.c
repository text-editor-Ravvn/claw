#include <stdio.h>

#include "render.h"
#include "cursor.h"
#include "buffer.h"
#include "editor.h"
#include "statusbar.h"

extern Buffer buffer;
extern Cursor cursor;
extern char *currentFile;

#define TAB_WIDTH 8

static int visualColumn(const Row *row, int byteColumn)
{
    int column = 0;
    int limit = byteColumn < row->size ? byteColumn : row->size;

    /* Convert buffer byte offsets into terminal display columns. */
    for (int i = 0; i < limit; i++)
    {
        if ((unsigned char)row->chars[i] == '\t')
            column += TAB_WIDTH - (column % TAB_WIDTH);
        else
            column++;
    }

    return column;
}

static void renderRow(const Row *row)
{
    /* Control bytes must not execute terminal commands or move the cursor. */
    int column = 0;

    for (int i = 0; i < row->size; i++)
    {
        unsigned char value = (unsigned char)row->chars[i];

        if (value == '\t')
        {
            int spaces = TAB_WIDTH - (column % TAB_WIDTH);
            for (int j = 0; j < spaces; j++)
                putchar(' ');
            column += spaces;
        }
        else if (value < 32 || value == 127)
        {
            putchar(' ');
            column++;
        }
        else
        {
            putchar(value);
            column++;
        }
    }
}

void refreshScreen(void)
{
    /* Clear the frame before drawing the current document. */
    printf("\033[2J");
    printf("\033[H");

    /* Raw terminals need CRLF to move down and return to column one. */
    printf("Claw Editor v0.5\r\n\r\n");

    if (!buffer.rows || buffer.numRows <= 0)
    {
        printf("Unable to allocate the editor buffer.\r\n");
        fflush(stdout);
        return;
    }

    for (int i = 0; i < buffer.numRows; i++)
    {
        if (buffer.rows[i].chars)
        {
            renderRow(&buffer.rows[i]);
        }

        printf("\r\n");
    }

    refreshStatusBar(currentFile, buffer.modified, cursor.x, cursor.y, editorStatusMessage());

    int screenColumn = visualColumn(&buffer.rows[cursor.y], cursor.x);
    printf("\033[%d;%dH", cursor.y + 3, screenColumn + 1);
    fflush(stdout);
}