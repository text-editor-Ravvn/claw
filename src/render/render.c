#include <stdio.h>

#include "render.h"
#include "buffer.h"
#include "editor.h"
#include "cursor.h"
#include "statusbar.h"
#include "viewport.h"
#include "search.h"

extern Buffer buffer;
extern Cursor cursor;
extern char *currentFile;

void refreshScreen(void)
{
    printf("\033[?25l");
    printf("\033[H");

    int showWelcome =
        (currentFile == NULL &&
         buffer.numRows == 1 &&
         buffer.rows[0].size == 0);

    for (int i = 0; i < viewport.screenRows - 2; i++)
    {
        int fileRow = viewport.rowOffset + i;

        printf("\033[K");

        if (showWelcome)
        {
            if (i == viewport.screenRows / 3)
            {
                int padding =
            (viewport.screenCols - 16) / 2;

            for (int j = 0; j < padding; j++)
            putchar(' ');

            printf("Claw Text Editor");
            }
            else if (i == viewport.screenRows / 3 + 2)
            {
            int padding = (viewport.screenCols - 12) / 2;

            for (int j = 0; j < padding; j++)
                putchar(' ');

            printf("Version %s", CLAW_VERSION);
            }
            else if (i == viewport.screenRows / 3 + 4)
            {
                 int padding =(viewport.screenCols - 17) / 2;

            for (int j = 0; j < padding; j++)
            putchar(' ');
                printf("Ctrl+S  Save File");
            }
            else if (i == viewport.screenRows / 3 + 5)
            {
                int padding =(viewport.screenCols - 18) / 2;

                for (int j = 0; j < padding; j++)
                putchar(' ');
                printf("Ctrl+X  Quit Editor");
            }
            else
            {
                printf("~");
            }

            printf("\r\n");
            continue;
        }

        if (fileRow < buffer.numRows)
        {
            int len =
                buffer.rows[fileRow].size -
                viewport.colOffset;

            if (len < 0)
                len = 0;

            if (len > viewport.screenCols)
                len = viewport.screenCols;

            if (len > 0)
{
    for (int j = 0; j < len; j++)
    {
        int fileCol =
            viewport.colOffset + j;

        int highlight = 0;

        if (searchState.matchCount > 0)
        {
            int matchRow =
                searchState.matchRows[
                    searchState.currentMatch
                ];

            int matchCol =
                searchState.matchCols[
                    searchState.currentMatch
                ];

            if (fileRow == matchRow &&
                fileCol >= matchCol &&
                fileCol < matchCol +
                          searchState.length)
            {
                highlight = 1;
            }
        }

        if (highlight)
            printf("\033[30;43m");

        putchar(
            buffer.rows[fileRow].chars[
                fileCol
            ]
        );

        if (highlight)
            printf("\033[m");
    }
    }
        }
        else
        {
            printf("~");
        }

        printf("\r\n");
    }

    printf("\033[K");

    drawStatusBar();

    printf("\r\n");
    printf("\033[K");

    if (searchState.active)
    {
    printf(
        "Search: %s",
        searchState.query
    );
    }
    else
    {
    printf(
        "%s",
        editorStatusMessage()
    );
    }

    int screenRow =
        cursor.y - viewport.rowOffset + 1;

    int screenCol =
        cursor.x - viewport.colOffset + 1;

    if (screenRow < 1)
        screenRow = 1;

    if (screenCol < 1)
        screenCol = 1;

    printf(
        "\033[%d;%dH",
        screenRow,
        screenCol
    );

    printf("\033[?25h");
    fflush(stdout);
}