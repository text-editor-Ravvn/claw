#include <stdlib.h>
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
#include "plugin.h"
#include "brackets.h"
#include "bookmark.h"
#include "explorer.h"
#include "file_switcher.h"
#include "layout.h"
#include "project.h"

extern Buffer buffer;
extern Cursor cursor;
extern char *currentFile;

static int welcomeSplashDismissed = 0;

int welcomeSplashIsActive(void)
{
    return getenv("CLAW_GRAPHICAL_SPLASH_SHOWN") == NULL &&
           !welcomeSplashDismissed &&
           layoutState.showWelcome &&
           currentFile == NULL &&
           buffer.numRows == 1 &&
           buffer.rows[0].size == 0;
}

void dismissWelcomeSplash(void)
{
    welcomeSplashDismissed = 1;
    renderForceClear();
}

static void drawWelcomeSplash(void)
{
    FILE *image = fopen("assets/claw-splash.ansi", "r");

    if (image == NULL)
    {
        printf("\033[2J\033[H");
        printf("Unable to load assets/claw-splash.ansi\n");
        printf("Press Space to continue.\n");
        return;
    }

    char line[512];
    int imageWidth = 100;
    int row = 1;

    if (row < 1)
        row = 1;

    printf("\033[2J");

    while (fgets(line, sizeof(line), image) != NULL)
    {
        int col = (viewport.screenCols - imageWidth) / 2 + 1;
        if (col < 1)
            col = 1;

        printf("\033[%d;%dH%s", row++, col, line);
    }

    fclose(image);

    if (row < viewport.screenRows)
    {
        const char *prompt = "PRESS SPACE TO CONTINUE";
        int col = (viewport.screenCols - (int)strlen(prompt)) / 2 + 1;

        if (col < 1)
            col = 1;

        printf("\033[%d;%dH\033[1;96m%s\033[0m",
               row, col, prompt);
    }
}

static int lastRows = 0;
static int lastCols = 0;
extern PluginManagerState pluginManager;
void drawPluginManager(void);
void drawPluginDetails(void);
void drawFileExplorer(void);
void drawFileSwitcher(void);

static void drawProjectOutput(void)
{
    int row = 1;
    const char *line = projectOutput.text;

    printf("\033[2J\033[H");
    printf("\033[1;96mPROJECT OUTPUT (Esc to close)\033[0m");

    while (*line != '\0' && row < viewport.screenRows - 1)
    {
        const char *end = strchr(line, '\n');
        size_t length = end ? (size_t)(end - line) : strlen(line);
        size_t available = viewport.screenCols > 0
            ? (size_t)viewport.screenCols
            : 1;

        if (length > available)
            length = available;

        printf("\033[%d;1H\033[K%.*s", row + 1, (int)length, line);
        row++;
        line = end ? end + 1 : line + strlen(line);
    }

    printf("\033[%d;1H\033[KExit code: %d",
           viewport.screenRows, projectOutput.exitCode);
}

static const char *diagnosticSeverityName(ProjectDiagnosticSeverity severity)
{
    if (severity == PROJECT_DIAGNOSTIC_ERROR)
        return "error";
    if (severity == PROJECT_DIAGNOSTIC_WARNING)
        return "warning";
    return "note";
}

static void drawProjectDiagnostics(void)
{
    size_t count = projectDiagnosticCount();
    int row = 1;

    printf("\033[2J\033[H");
    printf("\033[1;96mPROJECT DIAGNOSTICS (Esc to close, Up/Down to navigate)\033[0m");

    for (size_t i = 0; i < count && row < viewport.screenRows - 1; i++)
    {
        const ProjectDiagnostic *diagnostic = projectGetDiagnostic(i);

        printf("\033[%d;1H\033[K", row + 1);
        if (i == projectDiagnosticsView.selected)
            printf("\033[7m");

        printf("%c %s:%d", i == projectDiagnosticsView.selected ? '>' : ' ',
               diagnostic->file, diagnostic->line);
        if (diagnostic->column > 0)
            printf(":%d", diagnostic->column);
        printf(": %s: %s\033[0m",
               diagnosticSeverityName(diagnostic->severity),
               diagnostic->message);
        row++;
    }

    printf("\033[%d;1H\033[K%d diagnostic(s)",
           viewport.screenRows, (int)count);
}

void renderForceClear(void)
{
    lastRows = 0;
    lastCols = 0;
}

static void drawWelcomeShortcuts(void)
{
    static const char *columns[3][12] = {
        {
            "EDITOR",
            "Ctrl+S  Save",
            "Ctrl+F  Search",
            "Ctrl+R  Replace",
            "Ctrl+A  Replace All",
            "Ctrl+Z  Undo",
            "Ctrl+Y  Redo",
            "Ctrl+N  Next Match",
            "Ctrl+P  Previous Match",
            "Ctrl+X  Quit",
            "",
            ""
        },
        {
            "GIT & NAVIGATION",
            "Ctrl+G  Git Add",
            "Ctrl+T  Git Restore",
            "Ctrl+U  Git Unstage",
            "Ctrl+B  Git Blame",
            "Alt+E   File Explorer",
            "Alt+Q   Quick Switch",
            "Alt+G   Go to Line",
            "Alt+B   Toggle Bookmark",
            "Alt+S   Save Session",
            "Alt+R   Restore Session",
            "Alt+T   Next Theme"
        },
        {
            "MENUS & PLUGINS",
            "Alt+1   Status Bar Settings",
            "Alt+2   Layout Settings",
            "Alt+3   Theme Maker",
            "Alt+4   Theme Manager",
            "Ctrl+L  Plugin List",
            "Ctrl+K  Reload Plugins",
            "Alt+M   Plugin Manager",
            "Alt+I   Plugin Info",
            "Ctrl+E  Plugin Log",
            "Ctrl+W  Plugin Help",
            ""
        }
    };

    const int columnWidth = 36;
    const int totalWidth = columnWidth * 3;

    int startCol = (viewport.screenCols - totalWidth) / 2 + 1;
    int startRow = (viewport.screenRows - 12) / 2;

    if (startCol < 1)
        startCol = 1;

    if (startRow < 1)
        startRow = 1;

    /* Avoid drawing the welcome content over the status area. */
    int maxRow = viewport.screenRows - 2;

    if (maxRow < 1)
        return;

    for (int r = 0; r < 12; r++)
    {
        int screenRow = startRow + r;

        if (screenRow > maxRow)
            break;

        for (int c = 0; c < 3; c++)
        {
            const char *line = columns[c][r];

            if (line[0] == '\0')
                continue;

            int screenCol = startCol + c * columnWidth;

            /* Clear this column's line before redrawing it. */
            printf("\033[%d;%dH\033[K", screenRow, screenCol);

            /* Keep headings highlighted and all text left-aligned. */
            if (r == 0)
                printf("\033[1;96m%s\033[0m", line);
            else
                printf("%s", line);
        }
    }
}

void refreshScreen(void)
{
    printf("\033[?25l");

        if (welcomeSplashIsActive())
    {
        drawWelcomeSplash();
        printf("\033[?25l");
        fflush(stdout);
        return;
    }

    if (projectOutputIsActive())
    {
        drawProjectOutput();
        printf("\033[?25h");
        fflush(stdout);
        return;
    }

    if (projectDiagnosticsIsActive())
    {
        drawProjectDiagnostics();
        printf("\033[?25h");
        fflush(stdout);
        return;
    }

    if (themeManagerMenu.active)
    {
        drawThemeManager();
        printf("\033[?25h");
        fflush(stdout);
        return;
    }


    if (themeMakerMenu.active)
    {
        drawThemeMaker();
        printf("\033[?25h");
        fflush(stdout);
        return;
    }

    if (statusBarSettingsMenu.active)
    {
        drawStatusBarSettings();
        printf("\033[?25h");
        fflush(stdout);
        return;
    }

    if (layoutSettingsMenu.active)
    {
        drawLayoutSettings();
        printf("\033[?25h");
        fflush(stdout);
        return;
    }

    if (explorerState.active)
    {
        printf("\033[2J");
        printf("\033[H");
        drawFileExplorer();
        printf("\033[?25h");
        fflush(stdout);
        return;
    }

    if (fileSwitcherState.active)
    {
        printf("\033[2J");
        printf("\033[H");
        drawFileSwitcher();
        printf("\033[?25h");
        fflush(stdout);
        return;
    }

   if (pluginManager.active)
{
    printf("\033[2J");
    printf("\033[H");

    if (pluginManager.detailsView)
    {
        drawPluginDetails();
    }
    else
    {
        drawPluginManager();
    }

    printf("\033[?25h");
    fflush(stdout);

    return;
}

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
        (layoutState.showWelcome &&
         currentFile == NULL &&
         buffer.numRows == 1 &&
         buffer.rows[0].size == 0);

    if (showWelcome)
    {
        printf("\033[2J\033[H");

        drawWelcomeShortcuts();

        if (layoutState.showStatusBar)
        {
            printf("\033[%d;1H", viewport.screenRows - 1);
            drawStatusBar();
        }

        printf("\033[%d;1H\033[K", viewport.screenRows);
        printf("%s", editorStatusMessage());

        printf("\033[?25h");
        fflush(stdout);
        return;
    }

    int gutter = layoutState.showLineNumbers ? viewport.gutterWidth : 0;
    int textCols = viewport.screenCols - gutter;
    int leftPadding = 0;

    if(layoutState.centeredMode && !layoutState.wideMode)
    {
        const int centeredWidth = 100;
        int availableWidth = viewport.screenCols - gutter;

        if(availableWidth > centeredWidth)
            textCols = centeredWidth;
        else
            textCols = availableWidth;

        if(textCols < 1)
        textCols = 1;

        leftPadding = (viewport.screenCols - gutter - textCols)/2;
    }

    if (textCols < 1)
        textCols = 1;

    for (int i = 0; i < viewport.screenRows - 2; i++)
    {
        int fileRow = viewport.rowOffset + i;

        printf("\033[K");

    if(!showWelcome && leftPadding > 0)
      printf("%*s", leftPadding, "");

        /* Draw line number gutter. */
        if (gutter > 0)
        {
            if (fileRow < buffer.numRows)
            {
                if (bookmarkIsBookmarked(fileRow))
                {
                    printf("\033[1;33m%*d*\033[m",
                           gutter - 1,
                           fileRow + 1);
                }
                else
                {
                    printf("\033[90m%*d \033[m",
                           gutter - 1,
                           fileRow + 1);
                }
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
                matchRow >= 0 &&
                matchRow < buffer.numRows &&
                matchCol >= 0 &&
                searchState.length > 0 &&
                matchCol + searchState.length <=
                    buffer.rows[matchRow].size &&
                fileCol >= matchCol &&
                fileCol < matchCol +
                          searchState.length &&
                strncmp(
                    buffer.rows[matchRow].chars + matchCol,
                    searchState.query,
                    (size_t)searchState.length
                ) == 0)
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
        else if (prevColor == HL_MATCH)
        {
            printf("\033[m");
            prevColor = -1;
        }
        else if (
            bracketMatchY >= 0 &&
            ((fileRow == bracketMatchY && fileCol == bracketMatchX) ||
             (fileRow == cursor.y && fileCol == cursor.x))
        )
        {
            if (prevColor != HL_BRACKET_MATCH)
            {
                printf("%s", highlightColor(HL_BRACKET_MATCH));
                prevColor = HL_BRACKET_MATCH;
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

    if (layoutState.showStatusBar)
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
    else if (gotoPrompt.active)
    {
        printf(
            "Go to line: %s",
            gotoPrompt.input
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

    if (gotoPrompt.active)
    {
        printf(
            "\033[%d;%dH",
            viewport.screenRows,
            (int)strlen("Go to line: ") + gotoPrompt.length + 1
        );
    }
    else
    {
        int screenRow =
            cursor.y - viewport.rowOffset + 1;

        int screenCol =
            cursor.x - viewport.colOffset + gutter + leftPadding + 1;

        if (screenRow < 1)
            screenRow = 1;

        if (screenCol < 1)
            screenCol = 1;

        printf(
            "\033[%d;%dH",
            screenRow,
            screenCol
        );
    }

    printf("\033[?25h");
    fflush(stdout);
}
void drawPluginManager(void)
{
    printf("\033[H");

    printf("Plugin Dashboard\r\n");
    printf("======================================================================\r\n\r\n");

    printf(
        " ID  Name                Version   Status      Commands\r\n"
    );

    printf(
        " ---------------------------------------------------------------------\r\n"
    );

    int enabledCount = 0;
    int disabledCount = 0;

    for (int i = 0; i < pluginCount(); i++)
    {
        Plugin *p = pluginGet(i);

        if (!p)
            continue;

        if (p->enabled)
            enabledCount++;
        else
            disabledCount++;

        printf(
            "%c%-2d  %-18s  %-8s  %-10s  %-8d\r\n",
            (i == pluginManager.selected)
                ? '>'
                : ' ',
            i,
            p->name,
            p->version,
            p->enabled
                ? "Enabled"
                : "Disabled",
            p->commandRuns
        );
    }

    printf(
        "\r\n ---------------------------------------------------------------------\r\n"
    );

    printf(
        " Loaded : %d\r\n",
        pluginCount()
    );

    printf(
        " Enabled: %d\r\n",
        enabledCount
    );

    printf(
        " Disabled: %d\r\n",
        disabledCount
    );

    printf("\r\n");

    const char *status =
        editorStatusMessage();

    if (status && status[0])
    {
        printf(
            " Status : %s\r\n",
            status
        );
    }

    printf("\r\n");

    printf(
        " [↑↓] Navigate   [Enter] Details   [Space] Toggle\r\n"
    );

    printf(
        " [r] Reload      [h] Health        [Esc] Return\r\n"
    );
}

void drawPluginDetails(void)
{
    printf("\033[H");

    Plugin *p =
        pluginGet(
            pluginManager.selected
        );

    if (!p)
    {
        printf(
            "No plugin selected\r\n"
        );

        return;
    }

    printf(
        "Plugin Details\r\n"
    );

    printf(
        "====================================================================\r\n\r\n"
    );

    /* ================================================================ */
    /* General Information */
    /* ================================================================ */

    printf(
        "General Information\r\n"
    );

    printf(
        "--------------------------------------------------------------------\r\n"
    );

    printf(
        "%-20s %s\r\n",
        "Name",
        p->name
    );

    printf(
        "%-20s %s\r\n",
        "Version",
        p->version
    );

    printf(
        "%-20s %s\r\n",
        "Status",
        p->enabled
            ? "Enabled"
            : "Disabled"
    );

    printf(
        "%-20s %s\r\n",
        "Author",
        p->author[0]
            ? p->author
            : "Claw Team"
    );

    printf(
        "%-20s %s\r\n",
        "Command",
        p->command[0]
            ? p->command
            : "-"
    );

    printf(
        "%-20s %s\r\n",
        "File",
        p->filename
    );

    printf(
        "%-20s %s\r\n",
        "Autoload",
        p->autoLoad
            ? "Enabled"
            : "Disabled"
    );

    printf(
        "%-20s %s\r\n",
        "Events",
        p->allowEvents
            ? "Enabled"
            : "Disabled"
    );

    printf("\r\n");

    /* ================================================================ */
    /* Statistics */
    /* ================================================================ */

    printf(
        "Statistics\r\n"
    );

    printf(
        "--------------------------------------------------------------------\r\n"
    );

    printf(
        "%-20s %d\r\n",
        "Load Count",
        p->loadCount
    );

    printf(
        "%-20s %d\r\n",
        "Commands Executed",
        p->commandRuns
    );

    printf(
        "%-20s %d\r\n",
        "Event Count",
        p->eventCount
    );

    printf(
        "%-20s %d\r\n",
        "Startup Hooks",
        p->startupHooks
    );

    printf(
        "%-20s %d\r\n",
        "Save Hooks",
        p->saveHooks
    );

    printf(
        "%-20s %d\r\n",
        "Search Hooks",
        p->searchHooks
    );

    printf(
        "%-20s %d\r\n",
        "Exit Hooks",
        p->exitHooks
    );

    printf("\r\n");

    /* ================================================================ */
    /* Health */
    /* ================================================================ */

    printf(
        "Health\r\n"
    );

    printf(
        "--------------------------------------------------------------------\r\n"
    );

    printf(
        "%-20s %s\r\n",
        "State",
        p->healthy
            ? "Healthy"
            : "Warning"
    );

    printf(
        "%-20s %d%%\r\n",
        "Health Score",
        p->healthScore
    );

    printf(
        "%-20s %d\r\n",
        "Warnings",
        p->warningCount
    );

    printf(
        "%-20s %d\r\n",
        "Errors",
        p->errorCount
    );

    printf(
        "%-20s %s\r\n",
        "Last Reload",
        "Success"
    );

    printf("\r\n");

    /* ================================================================ */
    /* Controls */
    /* ================================================================ */

    printf(
        "Controls\r\n"
    );

    printf(
        "--------------------------------------------------------------------\r\n"
    );

    printf(
        "[Space] Toggle Plugin\r\n"
    );

    printf(
        "[a]     Toggle Autoload\r\n"
    );

    printf(
        "[e]     Toggle Events\r\n"
    );

    printf(
        "[r]     Reload Plugin\r\n"
    );

    printf(
        "[h]     Health Check\r\n"
    );

    printf(
        "[Esc]   Back To Dashboard\r\n"
    );
}

void drawFileExplorer(void)
{
    printf("\033[H");
    printf("\033[1;36m Claw Workspace File Explorer\033[m\r\n");
    printf("======================================================================\r\n");
    printf(" Current: \033[1m%s\033[m\r\n", explorerState.currentDir);
    printf("----------------------------------------------------------------------\r\n");

    int maxDisplay = viewport.screenRows - 8;
    if (maxDisplay < 5)
        maxDisplay = 5;

    int start = explorerState.selectedIndex - (maxDisplay / 2);
    if (start < 0)
        start = 0;
    if (start + maxDisplay > explorerState.count)
        start = explorerState.count > maxDisplay ? explorerState.count - maxDisplay : 0;

    int end = start + maxDisplay;
    if (end > explorerState.count)
        end = explorerState.count;

    if (explorerState.count == 0)
    {
        printf(" (empty directory)\r\n");
    }

    for (int i = start; i < end; i++)
    {
        ExplorerEntry *e = &explorerState.entries[i];
        char cursorChar = (i == explorerState.selectedIndex) ? '>' : ' ';

        if (i == explorerState.selectedIndex)
            printf("\033[7m");

        if (e->isDir)
        {
            printf(" %c [DIR]  %-40s\r\n", cursorChar, e->name);
        }
        else
        {
            printf(" %c [FILE] %-40s  %6ld B\r\n", cursorChar, e->name, e->sizeBytes);
        }

        if (i == explorerState.selectedIndex)
            printf("\033[m");
    }

    for (int i = end - start; i < maxDisplay; i++)
    {
        printf("\r\n");
    }

    printf("----------------------------------------------------------------------\r\n");
    printf(" [Enter] Open/Descend  [Backspace/Left] Parent Dir  [Up/Down] Navigate  [Esc] Close\r\n");
}

void drawFileSwitcher(void)
{
    printf("\033[H");
    printf("\033[1;35m Quick File Switcher\033[m\r\n");
    printf("======================================================================\r\n");
    printf(" Filter: \033[1m%s\033[m_\r\n", fileSwitcherState.query);
    printf("----------------------------------------------------------------------\r\n");

    int maxDisplay = viewport.screenRows - 8;
    if (maxDisplay < 5)
        maxDisplay = 5;

    if (fileSwitcherState.filteredCount == 0)
    {
        printf(" (no matching files)\r\n");
    }

    int start = fileSwitcherState.selectedIndex - (maxDisplay / 2);
    if (start < 0)
        start = 0;
    if (start + maxDisplay > fileSwitcherState.filteredCount)
        start = fileSwitcherState.filteredCount > maxDisplay ? fileSwitcherState.filteredCount - maxDisplay : 0;

    int end = start + maxDisplay;
    if (end > fileSwitcherState.filteredCount)
        end = fileSwitcherState.filteredCount;

    for (int i = start; i < end; i++)
    {
        int realIdx = fileSwitcherState.filteredIndices[i];
        const char *path = fileSwitcherState.allFiles[realIdx];
        char cursorChar = (i == fileSwitcherState.selectedIndex) ? '>' : ' ';

        if (i == fileSwitcherState.selectedIndex)
            printf("\033[7m");

        printf(" %c %-60s\r\n", cursorChar, path);

        if (i == fileSwitcherState.selectedIndex)
            printf("\033[m");
    }

    for (int i = end - start; i < maxDisplay; i++)
    {
        printf("\r\n");
    }

    printf("----------------------------------------------------------------------\r\n");
    printf(" [Enter] Select  [Up/Down] Navigate  [Type to Filter]  [Esc] Cancel\r\n");
}