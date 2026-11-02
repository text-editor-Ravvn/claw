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
#include "buffer_manager.h"
#include "split.h"
#include "terminal.h"
#include "symbols.h"
#include "git.h"
#include "utils.h"
#include "lsp.h"
#include "dap.h"
#include "remote.h"
#include "templates.h"
#include "marketplace.h"

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
        image = fopen("../assets/claw-splash.ansi", "r");

    if (image == NULL)
    {
        printf("\033[2J\033[H");
        printf("Unable to load assets/claw-splash.ansi\n");
        printf("Press Space to continue.\n");
        return;
    }

    const int imageWidth = 100;
    const int totalSplashRows = 40;

    int startRow = 1;
    if (viewport.screenRows > totalSplashRows + 2)
        startRow = (viewport.screenRows - (totalSplashRows + 2)) / 2 + 1;

    int row = startRow;
    char *line = NULL;
    size_t capacity = 0;
    ssize_t length;

    /* Disable auto-wrap during splash render to prevent distortion on narrow terminals */
    printf("\033[?7l\033[2J");

    /* Leave at least one row at the bottom for the continue prompt */
    int maxImageRows = viewport.screenRows > 2 ? viewport.screenRows - 2 : 1;

    while ((length = getline(&line, &capacity, image)) != -1) {
        if (row >= maxImageRows)
            break;

        while (length > 0 &&
               (line[length - 1] == '\n' ||
                line[length - 1] == '\r')) {
            line[--length] = '\0';
        }

        int col = (viewport.screenCols - imageWidth) / 2 + 1;
        if (col < 1)
            col = 1;

        printf("\033[%d;%dH%s\033[0m", row++, col, line);
    }

    /* Restore terminal auto-wrap mode */
    printf("\033[?7h");

    free(line);
    fclose(image);

    const char *prompt = "PRESS SPACE OR ENTER TO CONTINUE";
    int promptRow = row + 1;
    if (promptRow >= viewport.screenRows)
        promptRow = viewport.screenRows;
    int promptCol = (viewport.screenCols - (int)strlen(prompt)) / 2 + 1;

    if (promptCol < 1)
        promptCol = 1;

    printf("\033[%d;%dH\033[1;96m%s\033[0m",
           promptRow, promptCol, prompt);
}

static int lastRows = 0;
static int lastCols = 0;
extern PluginManagerState pluginManager;
void drawPluginManager(void);
void drawPluginDetails(void);
void drawFileExplorer(void);
void drawFileSwitcher(void);

void drawSymbolNavigator(void)
{
    printf("\033[2J\033[H");
    int rows = viewport.screenRows;

    printf("\033[1;36m=== Symbol Navigator (Claw IDE) ===\033[0m\r\n");
    printf("\033[1;33mQuery: %s_\033[0m\r\n", symbolState.query);
    printf("\033[90m------------------------------------------------------------\033[0m\r\n");

    int maxDisplay = rows - 6;
    if (maxDisplay < 1) maxDisplay = 1;

    int start = 0;
    if (symbolState.selectedIndex >= maxDisplay)
        start = symbolState.selectedIndex - maxDisplay + 1;

    for (int i = 0; i < maxDisplay; i++)
    {
        int idx = start + i;
        if (idx >= symbolState.filteredCount)
        {
            printf("\033[K\r\n");
            continue;
        }

        const Symbol *sym = &symbolState.symbols[symbolState.filteredIndices[idx]];
        int isSelected = (idx == symbolState.selectedIndex);

        if (isSelected)
            printf("\033[7m");

        printf(" [%-6s] %-24.24s %s:%d\033[0m\r\n",
               symbolKindToString(sym->kind),
               sym->name,
               sym->file,
               sym->line);
    }

    printf("\033[%d;1H\033[90m[Enter] Jump  [Up/Down] Select  [Esc] Cancel  (%d matches)\033[0m",
           rows, symbolState.filteredCount);
}

void drawTerminalPanel(int startRow, int numRows)
{
    printf("\033[%d;1H\033[1;34m─── Terminal: %s (Ctrl+` or Esc to close) ───\033[0m\033[K\r\n",
           startRow, integratedTerminal.workingDir);

    int textRows = numRows - 2;
    if (textRows < 1) textRows = 1;

    int startLine = integratedTerminal.lineCount - textRows - integratedTerminal.scrollOffset;
    if (startLine < 0) startLine = 0;

    for (int i = 0; i < textRows; i++)
    {
        int lIdx = startLine + i;
        printf("\033[%d;1H\033[K", startRow + 1 + i);
        if (lIdx >= 0 && lIdx < integratedTerminal.lineCount)
        {
            printf("%s", integratedTerminal.lines[lIdx]);
        }
    }

    int promptRow = startRow + numRows - 1;
    printf("\033[%d;1H\033[K\033[1;32mclaw:~$ \033[0m%s",
           promptRow, integratedTerminal.currentCommand);
}

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

        if (length > 0 && line[length - 1] == '\r')
            length--;

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
    int maxDisplay = viewport.screenRows - 2;
    if (maxDisplay < 1)
        maxDisplay = 1;

    size_t start = 0;
    if (count > (size_t)maxDisplay)
    {
        if (projectDiagnosticsView.selected >= (size_t)maxDisplay)
            start = projectDiagnosticsView.selected - (size_t)maxDisplay + 1;
    }

    size_t end = start + (size_t)maxDisplay;
    if (end > count)
        end = count;

    int row = 1;

    printf("\033[2J\033[H");
    printf("\033[1;96mPROJECT DIAGNOSTICS (Esc to close, Enter to jump, Up/Down to navigate)\033[0m");

    for (size_t i = start; i < end && row < viewport.screenRows - 1; i++)
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

    printf("\033[%d;1H\033[KDiagnostic %d of %d",
           viewport.screenRows, count > 0 ? (int)(projectDiagnosticsView.selected + 1) : 0, (int)count);
}

static void drawProjectDashboard(void)
{
    int row = 1;
    const char *name = projectGetName();
    const char *type = projectGetType();
    const char *root = projectGetRoot();
    const char *build = projectGetBuildCommand();
    const char *run = projectGetRunCommand();
    const char *test = projectGetTestCommand();

    printf("\033[2J\033[H");
    printf("\033[1;96mPROJECT DASHBOARD (Esc to close)\033[0m");
    printf("\033[%d;1H\033[KProject: %s", ++row,
           name && *name ? name : "No project configuration");
    printf("\033[%d;1H\033[KType: %s", ++row, type && *type ? type : "Unknown");
    printf("\033[%d;1H\033[KRoot: %s", ++row, root && *root ? root : "None");
    printf("\033[%d;1H\033[KBuild: %s", ++row,
           build && *build ? build : "Not configured");
    printf("\033[%d;1H\033[KRun: %s", ++row,
           run && *run ? run : "Not configured");
    printf("\033[%d;1H\033[KTest: %s", ++row,
           test && *test ? test : "Not configured");
    printf("\033[%d;1H\033[KNamed tasks: %d", ++row,
           (int)projectTaskCount());
    printf("\033[%d;1H\033[KDiagnostics: %d", ++row,
           (int)projectDiagnosticCount());
    printf("\033[%d;1H\033[KOutput: %s", ++row,
           projectOutput.length > 0 ? "Available" : "None");
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

static Buffer *getPaneBuffer(int paneIdx)
{
    if (paneIdx == splitLayout.activePane)
    {
        return &buffer;
    }
    int bIdx = splitLayout.panes[paneIdx].bufferIndex;
    if (bIdx == bufferManagerActiveIndex())
    {
        return &buffer;
    }
    FileBuffer *fb = bufferManagerGet(bIdx);
    if (fb)
        return &fb->doc;
    return &buffer;
}

static int getPaneRowOffset(int paneIdx)
{
    if (paneIdx == splitLayout.activePane)
        return viewport.rowOffset;
    return splitLayout.panes[paneIdx].rowOffset;
}

static int getPaneColOffset(int paneIdx)
{
    if (paneIdx == splitLayout.activePane)
        return viewport.colOffset;
    return splitLayout.panes[paneIdx].colOffset;
}

static void renderPaneTextRow(Buffer *doc, int fileRow, int colOffset, int textCols, int isActivePane, int padEnd)
{
    if (textCols <= 0)
        return;

    if (!doc || fileRow >= doc->numRows)
    {
        putchar('~');
        if (padEnd)
        {
            for (int s = 1; s < textCols; s++)
                putchar(' ');
        }
        return;
    }

    Row *row = &doc->rows[fileRow];
    if (!row->hl)
        highlightRow(row, fileRow);

    int len = row->size - colOffset;
    if (len < 0)
        len = 0;
    if (len > textCols)
        len = textCols;

    int prevColor = -1;
    for (int j = 0; j < len; j++)
    {
        int fileCol = colOffset + j;
        int highlight = 0;

        if (isActivePane && searchState.matchCount > 0)
        {
            int matchRow = searchState.matchRows[searchState.currentMatch];
            int matchCol = searchState.matchCols[searchState.currentMatch];
            if (fileRow == matchRow && matchRow >= 0 && matchRow < doc->numRows &&
                matchCol >= 0 && searchState.length > 0 &&
                matchCol + searchState.length <= doc->rows[matchRow].size &&
                fileCol >= matchCol && fileCol < matchCol + searchState.length &&
                strncmp(doc->rows[matchRow].chars + matchCol, searchState.query, (size_t)searchState.length) == 0)
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
        else if (isActivePane && bracketMatchY >= 0 &&
                 ((fileRow == bracketMatchY && fileCol == bracketMatchX) ||
                  (fileRow == cursor.y && fileCol == cursor.x)))
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

        putchar(row->chars[fileCol]);
    }

    if (prevColor != HL_NORMAL && prevColor != -1)
        printf("\033[m");

    if (padEnd)
    {
        for (int s = len; s < textCols; s++)
            putchar(' ');
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

    /* Phase 9 — Source Control Interactive Views */
    if (gitDashboardIsActive())
    {
        gitDashboardDraw();
        printf("\033[?25l");
        fflush(stdout);
        return;
    }

    if (gitDiffViewerIsActive())
    {
        gitDiffViewerDraw();
        printf("\033[?25l");
        fflush(stdout);
        return;
    }

    if (gitLogViewerIsActive())
    {
        gitLogViewerDraw();
        printf("\033[?25l");
        fflush(stdout);
        return;
    }

    if (gitCommitDialogIsActive())
    {
        gitCommitDialogDraw();
        printf("\033[?25h");
        fflush(stdout);
        return;
    }

    if (gitBranchModalIsActive())
    {
        gitBranchModalDraw();
        printf("\033[?25l");
        fflush(stdout);
        return;
    }

    if (gitStashModalIsActive())
    {
        gitStashModalDraw();
        printf("\033[?25l");
        fflush(stdout);
        return;
    }

    /* Phase 13 — Claw Ecosystem Modal */
    if (marketplaceIsActive())
    {
        marketplaceDraw();
        printf("\033[?25l");
        fflush(stdout);
        return;
    }

    /* Phase 12 — Advanced Platform Services Modals */
    if (dapDashboardIsActive())
    {
        dapDashboardDraw();
        printf("\033[?25l");
        fflush(stdout);
        return;
    }

    if (templateWizardIsActive())
    {
        templateWizardDraw();
        printf("\033[?25h");
        fflush(stdout);
        return;
    }

    if (remotePromptIsActive())
    {
        remotePromptDraw();
        printf("\033[?25h");
        fflush(stdout);
        return;
    }

    /* Phase 11 — Intelligent IDE Services Modal */
    if (lspReferencesIsActive())
    {
        lspReferencesDraw();
        printf("\033[?25h");
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

    if (projectDashboardIsActive())
    {
        drawProjectDashboard();
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

    if (symbolsIsActive())
    {
        drawSymbolNavigator();
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

    int termRows = terminalIsActive() ? integratedTerminal.height : 0;
    int editRows = viewport.screenRows - 2 - termRows;
    if (editRows < 1)
        editRows = 1;

    if (splitIsActive())
        splitUpdateDimensions(viewport.screenRows, viewport.screenCols);

    for (int i = 0; i < editRows; i++)
    {
        if (splitIsActive() && splitGetMode() == SPLIT_VERTICAL)
        {
            int w0 = splitLayout.panes[0].width;
            int g0 = (layoutState.showLineNumbers && w0 >= 15) ? viewport.gutterWidth : 0;
            int textCols0 = w0 - g0;
            int fileRow0 = getPaneRowOffset(0) + i;
            Buffer *doc0 = getPaneBuffer(0);

            int w1 = splitLayout.panes[1].width;
            int g1 = (layoutState.showLineNumbers && w1 >= 15) ? viewport.gutterWidth : 0;
            int textCols1 = w1 - g1;
            int fileRow1 = getPaneRowOffset(1) + i;
            Buffer *doc1 = getPaneBuffer(1);

            printf("\033[K");

            /* Pane 0 Gutter */
            if (g0 > 0)
            {
                if (fileRow0 < doc0->numRows)
                {
                    FileBuffer *fb0 = bufferManagerGet(splitLayout.panes[0].bufferIndex);
                    int showGit0 = (fb0 && fb0->filename[0] != '\0' && currentFile && strcmp(fb0->filename, currentFile) == 0);
                    int gitHl = showGit0 ? gitGutterGetLineStatus(fileRow0 + 1) : GIT_GUTTER_NONE;
                    if (bookmarkIsBookmarked(fileRow0))
                        printf("\033[1;33m%*d*\033[m", g0 - 1, fileRow0 + 1);
                    else if (gitHl == GIT_GUTTER_ADDED)
                        printf("\033[1;32m%*d+\033[m", g0 - 1, fileRow0 + 1);
                    else if (gitHl == GIT_GUTTER_MODIFIED)
                        printf("\033[1;33m%*d~\033[m", g0 - 1, fileRow0 + 1);
                    else if (gitHl == GIT_GUTTER_DELETED)
                        printf("\033[1;31m%*d_\033[m", g0 - 1, fileRow0 + 1);
                    else
                        printf("\033[90m%*d \033[m", g0 - 1, fileRow0 + 1);
                }
                else
                {
                    printf("%*s", g0, "");
                }
            }

            /* Pane 0 Text (padded to textCols0) */
            renderPaneTextRow(doc0, fileRow0, getPaneColOffset(0), textCols0, (splitLayout.activePane == 0), 1);

            /* Vertical Divider */
            printf("\033[90m│\033[0m");

            /* Pane 1 Gutter */
            if (g1 > 0)
            {
                if (fileRow1 < doc1->numRows)
                {
                    FileBuffer *fb1 = bufferManagerGet(splitLayout.panes[1].bufferIndex);
                    int showGit1 = (fb1 && fb1->filename[0] != '\0' && currentFile && strcmp(fb1->filename, currentFile) == 0);
                    int gitHl = showGit1 ? gitGutterGetLineStatus(fileRow1 + 1) : GIT_GUTTER_NONE;
                    if (bookmarkIsBookmarked(fileRow1))
                        printf("\033[1;33m%*d*\033[m", g1 - 1, fileRow1 + 1);
                    else if (gitHl == GIT_GUTTER_ADDED)
                        printf("\033[1;32m%*d+\033[m", g1 - 1, fileRow1 + 1);
                    else if (gitHl == GIT_GUTTER_MODIFIED)
                        printf("\033[1;33m%*d~\033[m", g1 - 1, fileRow1 + 1);
                    else if (gitHl == GIT_GUTTER_DELETED)
                        printf("\033[1;31m%*d_\033[m", g1 - 1, fileRow1 + 1);
                    else
                        printf("\033[90m%*d \033[m", g1 - 1, fileRow1 + 1);
                }
                else
                {
                    printf("%*s", g1, "");
                }
            }

            /* Pane 1 Text */
            renderPaneTextRow(doc1, fileRow1, getPaneColOffset(1), textCols1, (splitLayout.activePane == 1), 0);
            printf("\r\n");
            continue;
        }

        if (splitIsActive() && splitGetMode() == SPLIT_HORIZONTAL)
        {
            if (i == splitLayout.panes[0].height)
            {
                FileBuffer *fb0 = bufferManagerGet(splitLayout.panes[0].bufferIndex);
                FileBuffer *fb1 = bufferManagerGet(splitLayout.panes[1].bufferIndex);
                printf("\033[K\033[7m─── Pane 1: %s %s─── Pane 2: %s %s───\033[0m\r\n",
                       fb0 ? fb0->displayName : "[Untitled]",
                       splitLayout.activePane == 0 ? "(Active) " : "",
                       fb1 ? fb1->displayName : "[Untitled]",
                       splitLayout.activePane == 1 ? "(Active) " : "");
                continue;
            }
        }

        int paneIdx = 0;
        int fileRow = viewport.rowOffset + i;
        if (splitIsActive() && splitGetMode() == SPLIT_HORIZONTAL)
        {
            if (i < splitLayout.panes[0].height)
            {
                paneIdx = 0;
                fileRow = getPaneRowOffset(0) + i;
            }
            else
            {
                paneIdx = 1;
                fileRow = getPaneRowOffset(1) + (i - splitLayout.panes[0].height - 1);
            }
        }

        Buffer *curDoc = splitIsActive() ? getPaneBuffer(paneIdx) : &buffer;
        int colOff = splitIsActive() ? getPaneColOffset(paneIdx) : viewport.colOffset;
        int isActive = splitIsActive() ? (splitLayout.activePane == paneIdx) : 1;

        printf("\033[K");

        if (!showWelcome && leftPadding > 0)
            printf("%*s", leftPadding, "");

        /* Draw line number gutter */
        if (gutter > 0)
        {
            if (fileRow < curDoc->numRows)
            {
                FileBuffer *fbCur = splitIsActive() ? bufferManagerGet(splitLayout.panes[paneIdx].bufferIndex) : NULL;
                int showGit = !splitIsActive() || (fbCur && fbCur->filename[0] != '\0' && currentFile && strcmp(fbCur->filename, currentFile) == 0);
                int gitHl = showGit ? gitGutterGetLineStatus(fileRow + 1) : GIT_GUTTER_NONE;
                int dapHl = dapGutterStatus(currentFile, fileRow + 1);
                int lspHl = lspGutterStatus(currentFile, fileRow + 1);

                if (dapHl == DAP_GUTTER_EXEC_POINT || dapHl == DAP_GUTTER_BREAK_AND_EXEC)
                {
                    printf("\033[1;33m%*d>\033[m", gutter - 1, fileRow + 1);
                }
                else if (dapHl == DAP_GUTTER_BREAKPOINT)
                {
                    printf("\033[1;31m%*dB\033[m", gutter - 1, fileRow + 1);
                }
                else if (lspHl == LSP_SEVERITY_ERROR)
                {
                    printf("\033[1;31m%*dE\033[m", gutter - 1, fileRow + 1);
                }
                else if (lspHl == LSP_SEVERITY_WARNING)
                {
                    printf("\033[1;33m%*dW\033[m", gutter - 1, fileRow + 1);
                }
                else if (bookmarkIsBookmarked(fileRow))
                {
                    printf("\033[1;33m%*d*\033[m", gutter - 1, fileRow + 1);
                }
                else if (gitHl == GIT_GUTTER_ADDED)
                {
                    printf("\033[1;32m%*d+\033[m", gutter - 1, fileRow + 1);
                }
                else if (gitHl == GIT_GUTTER_MODIFIED)
                {
                    printf("\033[1;33m%*d~\033[m", gutter - 1, fileRow + 1);
                }
                else if (gitHl == GIT_GUTTER_DELETED)
                {
                    printf("\033[1;31m%*d_\033[m", gutter - 1, fileRow + 1);
                }
                else
                {
                    printf("\033[90m%*d \033[m", gutter - 1, fileRow + 1);
                }
            }
            else
            {
                printf("%*s", gutter, "");
            }
        }

        renderPaneTextRow(curDoc, fileRow, colOff, textCols, isActive, 0);
        printf("\r\n");
    }

    if (terminalIsActive())
    {
        drawTerminalPanel(editRows + 1, termRows);
    }

    printf("\033[%d;1H\033[K", viewport.screenRows - 1);

    if (layoutState.showStatusBar)
        drawStatusBar();
    else
        printf("\033[K");

    printf("\033[%d;1H\033[K", viewport.screenRows);

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

    if (terminalIsActive())
    {
        int promptRow = editRows + termRows;
        printf(
            "\033[%d;%dH",
            promptRow,
            (int)strlen("claw:~$ ") + integratedTerminal.cursorCol + 1
        );
    }
    else if (gotoPrompt.active)
    {
        printf(
            "\033[%d;%dH",
            viewport.screenRows,
            (int)strlen("Go to line: ") + gotoPrompt.length + 1
        );
    }
    else
    {
        int screenRow = cursor.y - viewport.rowOffset + 1;
        int screenCol = cursor.x - viewport.colOffset + gutter + leftPadding + 1;

        if (splitIsActive())
        {
            if (splitGetMode() == SPLIT_HORIZONTAL)
            {
                if (splitLayout.activePane == 0)
                {
                    screenRow = cursor.y - viewport.rowOffset + 1;
                    if (screenRow > splitLayout.panes[0].height)
                        screenRow = splitLayout.panes[0].height;
                }
                else
                {
                    screenRow = cursor.y - viewport.rowOffset + splitLayout.panes[0].height + 2;
                    if (screenRow > editRows)
                        screenRow = editRows;
                    if (screenRow < splitLayout.panes[0].height + 2)
                        screenRow = splitLayout.panes[0].height + 2;
                }
            }
            else if (splitGetMode() == SPLIT_VERTICAL)
            {
                int g0 = (layoutState.showLineNumbers && splitLayout.panes[0].width >= 15) ? viewport.gutterWidth : 0;
                int g1 = (layoutState.showLineNumbers && splitLayout.panes[1].width >= 15) ? viewport.gutterWidth : 0;
                if (splitLayout.activePane == 0)
                {
                    screenRow = cursor.y - viewport.rowOffset + 1;
                    screenCol = cursor.x - viewport.colOffset + g0 + 1;
                    if (screenCol > splitLayout.panes[0].width)
                        screenCol = splitLayout.panes[0].width;
                }
                else
                {
                    screenRow = cursor.y - viewport.rowOffset + 1;
                    screenCol = splitLayout.panes[0].width + 1 + g1 + (cursor.x - viewport.colOffset) + 1;
                    if (screenCol > viewport.screenCols)
                        screenCol = viewport.screenCols;
                    if (screenCol < splitLayout.panes[0].width + 2)
                        screenCol = splitLayout.panes[0].width + 2;
                }
            }
        }

        if (screenRow < 1)
            screenRow = 1;
        if (screenRow > viewport.screenRows)
            screenRow = viewport.screenRows;

        if (screenCol < 1)
            screenCol = 1;
        if (screenCol > viewport.screenCols)
            screenCol = viewport.screenCols;

        printf(
            "\033[%d;%dH",
            screenRow,
            screenCol
        );
    }

    if (lspHoverIsActive())
        lspHoverDraw();
    if (lspCodeActionsIsActive())
        lspCodeActionsDraw();
    if (lspCompletionIsActive())
        lspCompletionDraw();

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