#include <stdio.h>
#include <string.h>

#include "statusbar.h"
#include "buffer.h"
#include "cursor.h"
#include "editor.h"
#include "viewport.h"
#include "highlight.h"
#include "git.h"

extern Buffer buffer;
extern Cursor cursor;
extern char *currentFile;
extern GitViewState gitView;

void drawStatusBar(void)
{
    char left[512];
    char right[64];
    char branch[128];
    char gitInfo[160] = "";

    /* ---------------- Git Viewer Mode ---------------- */
    if (gitView.active)
    {
        snprintf(
            left,
            sizeof(left),
            " Claw v%s | Git Blame View | Esc to return ",
            CLAW_VERSION
        );

        snprintf(
            right,
            sizeof(right),
            "Ln %d, Col %d",
            cursor.y + 1,
            cursor.x + 1
        );

        int leftLen = (int)strlen(left);
        int rightLen = (int)strlen(right);

        printf("\033[7m");

        printf("%s", left);

        int padding =
            viewport.screenCols -
            leftLen -
            rightLen;

        if (padding < 1)
            padding = 1;

        while (padding-- > 0)
            putchar(' ');

        printf("%s", right);

        printf("\033[K");
        printf("\033[m");

        return;
    }

    /* ---------------- Normal Status Bar ---------------- */
    if (gitCurrentBranch(branch, sizeof(branch)))
    {
        if (currentFile)
        {
            if (!gitFileTracked(currentFile))
            {
                snprintf(
                    gitInfo,
                    sizeof(gitInfo),
                    "Git:%s?",
                    branch
                );
            }
            else if (buffer.modified)
            {
                snprintf(
                    gitInfo,
                    sizeof(gitInfo),
                    "Git:%s*",
                    branch
                );
            }
            else
            {
                snprintf(
                    gitInfo,
                    sizeof(gitInfo),
                    "Git:%s",
                    branch
                );
            }
        }
        else
        {
            snprintf(
                gitInfo,
                sizeof(gitInfo),
                "Git:%s",
                branch
            );
        }

        snprintf(
            left,
            sizeof(left),
            " Claw v%s | %s%s | %s | %s | %d Lines",
            CLAW_VERSION,
            currentFile ? currentFile : "[No Name]",
            buffer.modified ? " [Modified]" : "",
            highlightLanguageName(),
            gitInfo,
            buffer.numRows
        );
    }
    else
    {
        snprintf(
            left,
            sizeof(left),
            " Claw v%s | %s%s | %s | %d Lines",
            CLAW_VERSION,
            currentFile ? currentFile : "[No Name]",
            buffer.modified ? " [Modified]" : "",
            highlightLanguageName(),
            buffer.numRows
        );
    }

    snprintf(
        right,
        sizeof(right),
        "Ln %d, Col %d",
        cursor.y + 1,
        cursor.x + 1
    );

    int leftLen = (int)strlen(left);
    int rightLen = (int)strlen(right);

    printf("\033[7m");

    printf("%s", left);

    int padding =
        viewport.screenCols -
        leftLen -
        rightLen;

    if (padding < 1)
        padding = 1;

    while (padding-- > 0)
        putchar(' ');

    printf("%s", right);

    printf("\033[K");
    printf("\033[m");
}