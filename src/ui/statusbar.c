#include <stdio.h>
#include <string.h>

#include "statusbar.h"
#include "buffer.h"
#include "cursor.h"
#include "editor.h"
#include "viewport.h"
#include "highlight.h"
#include "git.h"
#include "workspace.h"
#include "bookmark.h"
#include "theme.h"
#include "config.h"
#include "buffer_manager.h"
#include "split.h"

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
    if (gitView.pluginLog)
    {
        snprintf(
            left,
            sizeof(left),
            " Claw v%s | Plugin Log | Esc to return ",
            CLAW_VERSION
        );
    }
    else
    {
        snprintf(
            left,
            sizeof(left),
            " Claw v%s | Git Blame View | Esc to return ",
            CLAW_VERSION
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

    int maxLeft = viewport.screenCols - rightLen;
    if (maxLeft < 0)
        maxLeft = 0;
    if (leftLen > maxLeft)
        leftLen = maxLeft;

    int padding = viewport.screenCols - leftLen - rightLen;
    if (padding < 0)
        padding = 0;

    printf("\033[7m");
    printf("%.*s", leftLen, left);

    int padCount = padding;
    while (padding-- > 0)
        putchar(' ');

    int maxRight = viewport.screenCols - leftLen - padCount;
    if (maxRight > 0)
        printf("%.*s", maxRight, right);

    printf("\033[K");
    printf("\033[m");

    return;
}

    /* ---------------- Normal Status Bar ---------------- */

    char *separator = "";
    size_t used = 0;

    left[0] = '\0';

     /*
     * Append optional fields safely. Each field is added only when
     * its corresponding configuration option is enabled.
     */
    #define APPEND_FIELD(...)                                                   \
    do                                                                      \
    {                                                                       \
        int written = snprintf(                                             \
            left + used, sizeof(left) - used, "%s", separator);             \
        if (written < 0 || (size_t)written >= sizeof(left) - used)           \
            goto statusbar_done;                                            \
        used += (size_t)written;                                            \
        separator = " | ";                                                  \
        written = snprintf(                                                 \
            left + used, sizeof(left) - used, __VA_ARGS__);                 \
        if (written < 0 || (size_t)written >= sizeof(left) - used)           \
            goto statusbar_done;                                            \
        used += (size_t)written;                                            \
    } while (0)

    if (configStatusbarShowVersion())
        APPEND_FIELD("Claw v%s", CLAW_VERSION);

    if (configStatusbarShowWorkspace() && workspaceIsActive())
        APPEND_FIELD("%s", workspaceGetName());

    if (configStatusbarShowFilename())
    {
        APPEND_FIELD("%s%s",
                     currentFile ? currentFile : "[No Name]",
                     buffer.modified ? " [Modified]" : "");
    }
    else if (buffer.modified)
    {
        APPEND_FIELD("[Modified]");
    }

    if (bufferManagerCount() > 1)
        APPEND_FIELD("Buf %d/%d", bufferManagerActiveIndex() + 1, bufferManagerCount());

    if (splitIsActive())
        APPEND_FIELD("Split:%s (Pane %d)", splitGetMode() == SPLIT_HORIZONTAL ? "H" : "V", splitLayout.activePane + 1);

    if (configStatusbarShowLanguage())
        APPEND_FIELD("%s", highlightLanguageName());

    if (configStatusbarShowGit() &&
        gitCurrentBranch(branch, sizeof(branch)))
    {
        if (!currentFile)
        {
            snprintf(gitInfo, sizeof(gitInfo), "Git:%s", branch);
        }
        else if (!gitFileTracked(currentFile))
        {
            snprintf(gitInfo, sizeof(gitInfo), "Git:%s?", branch);
        }
        else if (buffer.modified)
        {
            snprintf(gitInfo, sizeof(gitInfo), "Git:%s*", branch);
        }
        else
        {
            snprintf(gitInfo, sizeof(gitInfo), "Git:%s", branch);
        }

        APPEND_FIELD("%s", gitInfo);
    }

    if (configStatusbarShowBookmarks())
    {
        int bmCount = bookmarkCount();
        if (bmCount > 0)
            APPEND_FIELD("Bm:%d", bmCount);
    }

    if (configStatusbarShowTheme())
        APPEND_FIELD("Theme: %s", themeCurrentName());

    if (configStatusbarShowLineCount())
        APPEND_FIELD("%d Lines", buffer.numRows);

    statusbar_done:
    #undef APPEND_FIELD

    snprintf(
        right,
        sizeof(right),
        "Ln %d, Col %d",
        cursor.y + 1,
        cursor.x + 1
    );

    int leftLen = (int)strlen(left);
    int rightLen = (int)strlen(right);

    int maxLeft = viewport.screenCols - rightLen;
    if (maxLeft < 0)
        maxLeft = 0;
    if (leftLen > maxLeft)
        leftLen = maxLeft;

    int padding = viewport.screenCols - leftLen - rightLen;
    if (padding < 0)
        padding = 0;

    printf("\033[7m");
    printf("%.*s", leftLen, left);

    int padCount = padding;
    while (padding-- > 0)
        putchar(' ');

    int maxRight = viewport.screenCols - leftLen - padCount;
    if (maxRight > 0)
        printf("%.*s", maxRight, right);

    printf("\033[K");
    printf("\033[m");
}