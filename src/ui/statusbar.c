#include <stdio.h>

#include "statusbar.h"
#include "buffer.h"
#include "cursor.h"
#include "editor.h"

extern Buffer buffer;
extern Cursor cursor;
extern char *currentFile;

void drawStatusBar(void)
{
    printf("\033[7m");

    if (currentFile)
        printf(" %s ", currentFile);
    else
        printf(" [No Name] ");

    if (buffer.modified)
        printf(" [Modified] ");

    printf(" | Ln %d, Col %d",
           cursor.y + 1,
           cursor.x + 1);

    printf("\033[K");
    printf("\033[m");

    const char *msg = editorStatusMessage();

    if (msg && *msg)
    {
        printf("\r\n");
        printf("%s", msg);
        printf("\033[K");
    }
}