#include <stdio.h>

#include "render.h"
#include "buffer.h"
#include "editor.h"
#include "cursor.h"
#include "statusbar.h"
#include "viewport.h"
#include "search.h"
#include "config.h"
#include "highlight.h"
#include <string.h>

extern Buffer buffer;
extern Cursor cursor;
extern char *currentFile;
static int lastRows = 0;
static int lastCols = 0;

void refreshScreen(void)
{
    printf("\033[?25l");

/* Clear only when terminal size changes */
if (lastRows != viewport.screenRows ||
    lastCols != viewport.screenCols)
{
    printf("\033[2J");
    printf("\033[H");

    lastRows = viewport.screenRows;
    lastCols = viewport.screenCols;
}
else
{
    printf("\033[H");
}

    int showWelcome =
        (configShowWelcome() &&
         currentFile == NULL &&
         buffer.numRows == 1 &&
         buffer.rows[0].size == 0);

    int gutter = viewport.gutterWidth;
    int textCols = viewport.screenCols - gutter;

    if (textCols < 1)
        textCols = 1;

    for (int i = 0; i < viewport.screenRows - 2; i++)
    {
        int fileRow = viewport.rowOffset + i;

        printf("\033[K");

      if (showWelcome)
{
    const char *welcomeLines[] =
    {
        "Claw Text Editor",
        "",
        ("Version " CLAW_VERSION),
        "",
        "Ctrl+S    Save File",
        "Ctrl+F    Search",
        "Ctrl+R    Replace",
        "Ctrl+Z    Undo",
        "Ctrl+Y    Redo",
        "",
        "Ctrl+G    Git Add",
        "Ctrl+T    Git Restore",
        "Ctrl+B    Git Blame",
        "Esc       Exit Git View",
        "",
        "Ctrl+X    Quit Editor"
    };

    int lineCount =
        sizeof(welcomeLines) /
        sizeof(welcomeLines[0]);

    int startRow =
        (viewport.screenRows - 2 - lineCount) / 2;

    if (i >= startRow &&
        i < startRow + lineCount)
    {
        int index = i - startRow;
        const char *line = welcomeLines[index];

        /* Center title and version */
        if (index == 0 || index == 2)
        {
            int len = (int)strlen(line);

            int padding =
                (viewport.screenCols - len) / 2;

            if (padding < 0)
                padding = 0;

            for (int j = 0; j < padding; j++)
                putchar(' ');

            printf("%s", line);
        }
        else
        {
            int blockWidth = 28;

            int padding =
                (viewport.screenCols - blockWidth) / 2;

            if (padding < 0)
                padding = 0;

            for (int j = 0; j < padding; j++)
                putchar(' ');

            printf("%s", line);
        }
    }
    else
    {
        putchar(' ');
    }

    printf("\r\n");
    continue;
}

        /* Draw line number gutter. */
        if (gutter > 0)
        {
            if (fileRow < buffer.numRows)
            {
                printf("\033[90m%*d \033[m",
                       gutter - 1,
                       fileRow + 1);
            }
            else
            {
                printf("%*s", gutter, "");
            }
        }

        if (fileRow < buffer.numRows)
        {
            int len =
                buffer.rows[fileRow].size -
                viewport.colOffset;

            if (len < 0)
                len = 0;

            if (len > textCols)
                len = textCols;

            if (len > 0)
{
    /* Lazily compute highlighting for this row if needed. */
    Row *row = &buffer.rows[fileRow];

    if (!row->hl)
        highlightRow(row, fileRow);

    int prevColor = -1;

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
        {
            if (prevColor != HL_MATCH)
            {
                printf("%s", highlightColor(HL_MATCH));
                prevColor = HL_MATCH;
            }
        }
        else if (row->hl && fileCol < row->size)
        {
            int hlType = row->hl[fileCol];

            if (hlType != prevColor)
            {
                printf("%s", highlightColor(hlType));
                prevColor = hlType;
            }
        }
        else
        {
            if (prevColor != HL_NORMAL)
            {
                printf("\033[m");
                prevColor = HL_NORMAL;
            }
        }

        putchar(
            buffer.rows[fileRow].chars[
                fileCol
            ]
        );
    }

    /* Reset color at end of row. */
    if (prevColor != HL_NORMAL && prevColor != -1)
        printf("\033[m");
    }
        }
        else
        {
            printf("~");
        }

        printf("\r\n");
    }

    printf("\033[K");

    if (configShowStatusBar())
        drawStatusBar();
    else
        printf("\033[K");

    printf("\r\n");
    printf("\033[K");

   if (savePrompt.active)
    {
        printf(
            "Save as: %s",
            savePrompt.filename
        );
    }
    else if (searchState.active)
{
    if (searchState.replaceMode)
    {
        printf(
            "Replace: %s",
            searchState.replacement
        );
    }
    else
    {
    printf(
        "Search: %s",
        searchState.query
    );
    }
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
        cursor.x - viewport.colOffset + gutter + 1;

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