#include <stdio.h>
#include "statusbar.h"
#include "buffer.h"
#include "cursor.h"
#include "editor.h"
#include "viewport.h"
#include <string.h>
extern Buffer buffer;
extern Cursor cursor;
extern char *currentFile;

void drawStatusBar(void)
{
    char left[256];
    char right[64];

    snprintf(
    left,
    sizeof(left),
    " Claw v%s | %s%s | %d Lines",
    CLAW_VERSION,
    currentFile ? currentFile : "[No Name]",
    buffer.modified ? " [Modified]" : "",
    buffer.numRows
);

    snprintf(
        right,
        sizeof(right),
        "Ln %d, Col %d",
        cursor.y + 1,
        cursor.x + 1
    );

    int leftLen = strlen(left);
    int rightLen = strlen(right);

    printf("\033[7m");

    printf("%s", left);

    int padding =
        viewport.screenCols -
        leftLen -
        rightLen;

    while (padding-- > 0)
        putchar(' ');

    printf("%s", right);

    printf("\033[K");
    printf("\033[m");
}