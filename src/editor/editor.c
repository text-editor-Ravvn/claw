#include <stdlib.h>
#include <stdio.h>
#include "editor.h"
#include "input.h"
#include "render.h"
#include "cursor.h"
#include "rawmode.h"
#include "buffer.h"
#include "fileio.h"
#include "statusbar.h"
#include "viewport.h"
#include "search.h"
#include <string.h>
#include "history.h"
#include "config.h"
#include "keymap.h"
#include "command.h"
#include "highlight.h"
#include "git.h"
#include "plugin.h"
#include "plugin_api.h"
#include "plugin_commands.h"
#include "brackets.h"
#include "indent.h"
#include "workspace.h"
#include "recent.h"
#include "session.h"
#include "bookmark.h"
#include "explorer.h"
#include "file_switcher.h"
#include "theme.h"
#include "layout.h"

char *currentFile = NULL;
static char statusMessage[128] = "";
static int quitRequested = 0;

SavePrompt savePrompt = {0};
GotoLinePrompt gotoPrompt = {0};
GitViewState gitView = {0};
PluginManagerState pluginManager = {0};

void openSavePrompt(void)
{
    savePrompt.active = 1;
    savePrompt.length = 0;
    savePrompt.filename[0] = '\0';
    editorSetStatusMessage("");
}

void closeSavePrompt(void)
{
    savePrompt.active = 0;
}

void openGotoPrompt(void)
{
    gotoPrompt.active = 1;
    gotoPrompt.length = 0;
    gotoPrompt.input[0] = '\0';
    editorSetStatusMessage("");
}

void closeGotoPrompt(void)
{
    gotoPrompt.active = 0;
}

void performGotoLine(void)
{
    if (gotoPrompt.length == 0 || buffer.numRows <= 0)
        return;

    const char *str = gotoPrompt.input;
    if (*str == ':')
        str++;

    int line = 1;
    int col = 1;
    if (sscanf(str, "%d:%d", &line, &col) < 1)
        return;

    if (line < 1)
        line = 1;
    if (line > buffer.numRows)
        line = buffer.numRows;

    cursor.y = line - 1;

    int rowLen = (cursor.y < buffer.numRows) ? buffer.rows[cursor.y].size : 0;
    if (col < 1)
        col = 1;
    if (col - 1 > rowLen)
        col = rowLen + 1;

    cursor.x = col - 1;

    viewport.rowOffset = cursor.y - (viewport.screenRows / 2);
    if (viewport.rowOffset < 0)
        viewport.rowOffset = 0;

    char statusBuf[64];
    snprintf(statusBuf, sizeof(statusBuf), "Jumped to line %d, col %d", cursor.y + 1, cursor.x + 1);
    editorSetStatusMessage(statusBuf);
}

void editorInit(void)
{
    /* Raw mode must be active before the first screen refresh or key read. */
    enableRawMode();
    printf("\033[2 q");

    pluginInit();

    pluginLoadAll();

    pluginTriggerEvent(
    PLUGIN_EVENT_STARTUP
    );

    /* Load configuration before anything that reads settings.
       Order: compiled-in defaults → bundled defaults → user config.
       User config is searched at XDG_CONFIG_HOME/claw/claw.conf first
       (typically ~/.config/claw/claw.conf), then falls back to the
       relative config/ directory for development convenience. */
    configInit();
    themeInit();
    themeSet(THEME_DEFAULT);
    configLoad("config/default.conf");


    {
        char candidatePath[512] = "";
        char savePath[512] = "";
        int loaded = 0;

        const char *xdg = getenv("XDG_CONFIG_HOME");
        const char *home = getenv("HOME");

        /* Prefer the XDG configuration directory. */
        if (xdg && xdg[0] != '\0')
        {
            snprintf(candidatePath, sizeof(candidatePath),
                     "%s/claw/claw.conf", xdg);
            loaded = configLoad(candidatePath);

            if (loaded)
                snprintf(savePath, sizeof(savePath), "%s", candidatePath);
        }

        /* Fall back to the standard home configuration directory. */
        if (!loaded && home && home[0] != '\0')
        {
            snprintf(candidatePath, sizeof(candidatePath),
                     "%s/.config/claw/claw.conf", home);
            loaded = configLoad(candidatePath);

            if (loaded)
                snprintf(savePath, sizeof(savePath), "%s", candidatePath);
        }

        /*
         * If no user config exists, choose a destination for future
         * saves without modifying the bundled configuration files.
         */
        if (!loaded)
        {
            if (xdg && xdg[0] != '\0')
                snprintf(savePath, sizeof(savePath),
                         "%s/claw/claw.conf", xdg);
            else if (home && home[0] != '\0')
                snprintf(savePath, sizeof(savePath),
                         "%s/.config/claw/claw.conf", home);
        }

        configSetUserPath(savePath);

        /* Keep the development fallback for loading. */
        if (!loaded)
            configLoad("config/claw.conf");
    }

    layoutInit();
    /* Set up key bindings from defaults, then overlay the chosen keymap. */
    keymapInit();

    const char *mapName = configKeymapName();

    if (mapName && strlen(mapName) > 0)
    {
        char keymapPath[256];

        snprintf(keymapPath, sizeof(keymapPath),
                 "keymaps/%s.conf", mapName);

        keymapLoad(keymapPath);
    }

    bufferInit();
    historyInit();
    highlightInit();
    viewportInit();

    workspaceInit();
    workspaceLoadConfig();
    recentInit();
    bookmarkInit();
    sessionInit();

    if (workspaceIsActive())
    {
        recentAddWorkspace(workspaceGetRoot());
    }

    if (currentFile)
    {
        recentAddFile(currentFile);
    }

    gitRefreshStatus(currentFile);
}


StatusBarSettingsMenu statusBarSettingsMenu = {0};
LayoutSettingsMenu layoutSettingsMenu = {0};
ThemeMakerMenu themeMakerMenu = {0};
ThemeManagerMenu themeManagerMenu = {0};

static const char *statusBarOptionNames[STATUSBAR_OPTION_COUNT] = {
    "CLAW version",
    "Workspace name",
    "File name",
    "Programming language",
    "Git branch/status",
    "Bookmark count",
    "Theme name",
    "Total line count"
};

static void loadStatusBarValues(int values[])
{
    values[0] = !!configStatusbarShowVersion();
    values[1] = !!configStatusbarShowWorkspace();
    values[2] = !!configStatusbarShowFilename();
    values[3] = !!configStatusbarShowLanguage();
    values[4] = !!configStatusbarShowGit();
    values[5] = !!configStatusbarShowBookmarks();
    values[6] = !!configStatusbarShowTheme();
    values[7] = !!configStatusbarShowLineCount();
}

void openStatusBarSettings(void)
{
    loadStatusBarValues(statusBarSettingsMenu.values);

    for (int i = 0; i < STATUSBAR_OPTION_COUNT; i++)
        statusBarSettingsMenu.original[i] =
            statusBarSettingsMenu.values[i];

    statusBarSettingsMenu.selected = 0;
    statusBarSettingsMenu.active = 1;

    editorSetStatusMessage("");
}

void closeStatusBarSettings(void)
{
    statusBarSettingsMenu.active = 0;
}

int saveStatusBarSettings(void)
{
    return configApplyStatusBarSettings(
        statusBarSettingsMenu.values,
        STATUSBAR_OPTION_COUNT
    );
}

void drawStatusBarSettings(void)
{
    int rows = viewport.screenRows;
    int cols = viewport.screenCols;

    int panelWidth = cols - 4;
    if (panelWidth > 64)
        panelWidth = 64;
    if (panelWidth < 36)
        panelWidth = cols;

    if (panelWidth > cols)
        panelWidth = cols;

    int left = (cols - panelWidth) / 2;
    if (left < 0)
        left = 0;

    /* Absolute cursor positioning: ESC[row;columnH */
    printf("\033[2J");

    int row = 1;

    /* Top border */
    printf("\033[%d;%dH+", row++, left + 1);
    for (int i = 0; i < panelWidth - 2; i++)
        putchar('-');
    putchar('+');

    /* Title */
    printf("\033[%d;%dH|", row, left + 1);
    const char *title = "STATUS BAR SETTINGS";
    int titleCol = left + (panelWidth - (int)strlen(title)) / 2 + 1;
    printf("\033[%d;%dH%s", row++, titleCol, title);
    printf("\033[%d;%dH|", row - 1, left + panelWidth);

    /* Divider */
    printf("\033[%d;%dH+", row, left + 1);
    for (int i = 0; i < panelWidth - 2; i++)
        putchar('-');
    putchar('+');
    row += 2;

    printf("\033[%d;%dHChoose which fields to display:", row++, left + 2);
    row++;

    for (int i = 0; i < STATUSBAR_OPTION_COUNT; i++)
    {
        char line[128];

        snprintf(line, sizeof(line), "[%c] %-25s %s",
                 statusBarSettingsMenu.values[i] ? 'x' : ' ',
                 statusBarOptionNames[i],
                 statusBarSettingsMenu.values[i] ? "ON" : "OFF");

        printf("\033[%d;%dH|", row, left + 1);

        if (i == statusBarSettingsMenu.selected)
            printf("\033[7m");

        printf("\033[%d;%dH%-*.*s",
               row, left + 3,
               panelWidth - 4, panelWidth - 4, line);

        printf("\033[0m\033[%d;%dH|", row++, left + panelWidth);
    }

    /* Bottom border */
    printf("\033[%d;%dH+", row, left + 1);
    for (int i = 0; i < panelWidth - 2; i++)
        putchar('-');
    putchar('+');
    row += 2;

    /* Help text is placed inside the panel and clipped to its width. */
    printf("\033[%d;%dH%-*.*s",
           row, left + 2,
           panelWidth - 4, panelWidth - 4,
           "Up/Down: Navigate  Space: Toggle  Enter: Save  Esc: Cancel");

    /* Don't draw beyond a short terminal. */
    if (rows > 0 && row > rows)
        printf("\033[%d;1H", rows);

    printf("\033[0m\033[?25l");
    fflush(stdout);
}

static const char *layoutOptionNames[LAYOUT_OPTION_COUNT] = {
    "Line numbers",
    "Status bar",
    "Welcome screen",
    "Centered mode",
    "Wide mode"
};

void openLayoutSettings(void)
{
    layoutSettingsMenu.values[0] = !!layoutState.showLineNumbers;
    layoutSettingsMenu.values[1] = !!layoutState.showStatusBar;
    layoutSettingsMenu.values[2] = !!layoutState.showWelcome;
    layoutSettingsMenu.values[3] = !!layoutState.centeredMode;
    layoutSettingsMenu.values[4] = !!layoutState.wideMode;

    for (int i = 0; i < LAYOUT_OPTION_COUNT; i++)
        layoutSettingsMenu.original[i] = layoutSettingsMenu.values[i];

    layoutSettingsMenu.selected = 0;
    layoutSettingsMenu.active = 1;
    editorSetStatusMessage("");
}

void closeLayoutSettings(void)
{
    layoutSettingsMenu.active = 0;
}

void applyLayoutSettings(void)
{
    layoutState.showLineNumbers = layoutSettingsMenu.values[0];
    layoutState.showStatusBar = layoutSettingsMenu.values[1];
    layoutState.showWelcome = layoutSettingsMenu.values[2];
    layoutState.centeredMode = layoutSettingsMenu.values[3];
    layoutState.wideMode = layoutSettingsMenu.values[4];
}

void drawLayoutSettings(void)
{
    int rows = viewport.screenRows;
    int cols = viewport.screenCols;

    int panelWidth = cols - 4;
    if (panelWidth > 56)
        panelWidth = 56;
    if (panelWidth < 36)
        panelWidth = cols;
    if (panelWidth > cols)
        panelWidth = cols;

    int left = (cols - panelWidth) / 2;
    if (left < 0)
        left = 0;

    printf("\033[2J");

    int row = 1;

    printf("\033[%d;%dH+", row++, left + 1);
    for (int i = 0; i < panelWidth - 2; i++)
        putchar('-');
    putchar('+');

    const char *title = "LAYOUT SETTINGS";
    printf("\033[%d;%dH|", row, left + 1);
    printf("\033[%d;%dH%s", row, left + (panelWidth - (int)strlen(title)) / 2 + 1, title);
    printf("\033[%d;%dH|", row++, left + panelWidth);

    printf("\033[%d;%dH+", row++, left + 1);
    for (int i = 0; i < panelWidth - 2; i++)
        putchar('-');
    putchar('+');

    row++;

    for (int i = 0; i < LAYOUT_OPTION_COUNT; i++)
    {
        printf("\033[%d;%dH|", row, left + 1);

        if (i == layoutSettingsMenu.selected)
            printf("\033[7m");

        printf("\033[%d;%dH[%c] %-24s %s",
               row,
               left + 3,
               layoutSettingsMenu.values[i] ? 'x' : ' ',
               layoutOptionNames[i],
               layoutSettingsMenu.values[i] ? "ON" : "OFF");

        if (i == layoutSettingsMenu.selected)
            printf("\033[0m");

        printf("\033[%d;%dH|", row++, left + panelWidth);
    }

    row++;
    printf("\033[%d;%dH|", row, left + 1);
    printf("\033[%d;%dHUp/Down: Navigate  Space: Toggle",
           row++, left + 3);
    printf("\033[%d;%dH|", row - 1, left + panelWidth);

    printf("\033[%d;%dH|", row, left + 1);
    printf("\033[%d;%dHEnter: Apply  Esc: Cancel",
           row++, left + 3);
    printf("\033[%d;%dH|", row - 1, left + panelWidth);

    printf("\033[%d;%dH+", row++, left + 1);
    for (int i = 0; i < panelWidth - 2; i++)
        putchar('-');
    putchar('+');

    (void)rows;
    printf("\033[?25l");
    fflush(stdout);
}

void editorRun(void)
{
    /* Redraw current state, wait for one command, then apply it. */
    while (1)
{
    viewportUpdateSize();
    updateBracketMatch();
    refreshScreen();

    KeyEvent event =
    readKey();

    int key =
        event.key;

    if (welcomeSplashIsActive())
    {
        if (event.key == ' ' && event.modifiers == 0)
            dismissWelcomeSplash();

        continue;
    }

    /* ---------- Custom Theme Manager Menu ---------- */
    if (themeManagerMenu.active)
    {
        size_t count = themeCustomCount();

        if (themeManagerMenu.pathPromptActive)
        {
            if (key == 27)
            {
                themeManagerMenu.pathPromptActive = 0;
                themeManagerMenu.path[0] = '\0';
                themeManagerMenu.pathLength = 0;
                editorSetStatusMessage("Theme operation cancelled");
                continue;
            }

            if (key == '\r' || key == '\n')
            {
                int success = 0;

                if (themeManagerMenu.pathLength > 0)
                {
                    if (themeManagerMenu.exportMode)
                    {
                        success = themeExportCustom(
                            (size_t)themeManagerMenu.selected,
                            themeManagerMenu.path);
                    }
                    else
                    {
                        success = themeImportCustom(
                            themeManagerMenu.path);
                    }
                }

                themeManagerMenu.pathPromptActive = 0;
                themeManagerMenu.path[0] = '\0';
                themeManagerMenu.pathLength = 0;

                if (success)
                {
                    snprintf(themeManagerMenu.message,
                        sizeof(themeManagerMenu.message),
                        "%s successful",
                        themeManagerMenu.exportMode ? "Export" : "Import");
                }
                else
                {
                    snprintf(themeManagerMenu.message,
                        sizeof(themeManagerMenu.message),
                        "%s failed: check path, file, or duplicate",
                        themeManagerMenu.exportMode ? "Export" : "Import");
                }

                editorSetStatusMessage(themeManagerMenu.message);

                continue;
            }

            if (key == 127 || key == 8)
            {
                if (themeManagerMenu.pathLength > 0)
                {
                    themeManagerMenu.pathLength--;
                    themeManagerMenu.path[
                        themeManagerMenu.pathLength] = '\0';
                }
                continue;
            }

            if (key >= 32 && key <= 126 &&
                themeManagerMenu.pathLength <
                    (int)sizeof(themeManagerMenu.path) - 1)
            {
                themeManagerMenu.path[
                    themeManagerMenu.pathLength++] = (char)key;

                themeManagerMenu.path[
                    themeManagerMenu.pathLength] = '\0';
            }

            continue;
        }

        if (themeManagerMenu.confirmDelete)
        {
            if (key == 'y' || key == 'Y')
            {
                if (themeDeleteCustom((size_t)themeManagerMenu.selected))
                {
                    count = themeCustomCount();

                    if (count == 0)
                        themeManagerMenu.selected = 0;
                    else if ((size_t)themeManagerMenu.selected >= count)
                        themeManagerMenu.selected = (int)count - 1;

                    themeManagerMenu.confirmDelete = 0;
                    editorSetStatusMessage("Custom theme deleted");
                }
                else
                {
                    themeManagerMenu.confirmDelete = 0;
                    editorSetStatusMessage("Could not delete custom theme");
                }
            }
            else if (key == 'n' || key == 'N' || key == 27)
            {
                themeManagerMenu.confirmDelete = 0;
            }

            continue;
        }

        if (key == 27)
        {
            closeThemeManager();
            continue;
        }

        if (key == ARROW_UP)
        {
            if (themeManagerMenu.selected > 0)
                themeManagerMenu.selected--;
            continue;
        }

        if (key == ARROW_DOWN)
        {
            if (count > 0 &&
                themeManagerMenu.selected < (int)count - 1 &&
                themeManagerMenu.selected < 4)
                themeManagerMenu.selected++;
            continue;
        }

        if (key == '\r' || key == '\n')
        {
            if (count > 0 &&
                themeActivateCustom((size_t)themeManagerMenu.selected))
            {
                editorSetStatusMessage("Custom theme activated");
            }
            continue;
        }

        if ((key == 'd' || key == 'D') &&
            count > 0 &&
            (size_t)themeManagerMenu.selected < count)
        {
            themeManagerMenu.confirmDelete = 1;
            continue;
        }

        if (key == 'i' || key == 'I')
        {
            themeManagerMenu.pathPromptActive = 1;
            themeManagerMenu.exportMode = 0;
            themeManagerMenu.pathLength = 0;
            themeManagerMenu.path[0] = '\0';
            continue;
        }

        if ((key == 'e' || key == 'E') &&
            count > 0 &&
            (size_t)themeManagerMenu.selected < count)
        {
            themeManagerMenu.pathPromptActive = 1;
            themeManagerMenu.exportMode = 1;
            themeManagerMenu.pathLength = 0;
            themeManagerMenu.path[0] = '\0';
            continue;
        }

            if ((key == 'm' || key == 'M') &&
            count > 0 &&
            (size_t)themeManagerMenu.selected < count)
        {
            size_t index = (size_t)themeManagerMenu.selected;
            const char *name = themeCustomName(index);

            if (name &&
                themeGetCustomColors(index, themeMakerMenu.codes))
            {
                snprintf(
                    themeMakerMenu.name,
                    sizeof(themeMakerMenu.name),
                    "%s",
                    name
                );

                themeMakerMenu.nameLength =
                    (int)strlen(themeMakerMenu.name);

                themeMakerMenu.editingIndex = (int)index;
                themeMakerMenu.active = 1;
                themeMakerMenu.selected = 0;

                themeManagerMenu.active = 0;

                if (!themePreviewCustom(
                        themeMakerMenu.name,
                        themeMakerMenu.codes))
                {
                    themeMakerMenu.active = 0;
                    themeManagerMenu.active = 1;
                    themeMakerMenu.editingIndex = -1;

                    editorSetStatusMessage(
                        "Could not preview selected theme"
                    );
                }
                else
                {
                    editorSetStatusMessage(
                        "Editing custom theme: Ctrl+S saves, Esc cancels"
                    );
                }
            }
            else
            {
                editorSetStatusMessage(
                    "Could not load selected theme colours"
                );
            }

            continue;
        }

        /* Consume all other keys while the manager is open. */
        continue;
    }

    /* ---------- Custom Theme Maker Menu ---------- */
    if (themeMakerMenu.active)
    {
        if (key == 27)
        {
            closeThemeMaker();
            editorSetStatusMessage("Theme Maker cancelled");
            continue;
        }

        if (key == 127 || key == 8)
        {
            if (themeMakerMenu.nameLength > 0) {
                themeMakerMenu.nameLength--;
                themeMakerMenu.name[themeMakerMenu.nameLength] = '\0';

                if (themeMakerMenu.nameLength > 0) {
                    themePreviewCustom(themeMakerMenu.name, themeMakerMenu.codes);
                }
            }
            continue;
        }

        if (event.key == 's' && (event.modifiers & MOD_CTRL))
        {
            saveThemeMaker();
            continue;
        }

        if (event.key == 'r' && (event.modifiers & MOD_CTRL))
        {
            resetThemeMaker();
            editorSetStatusMessage("Theme Maker reset");
            continue;
        }

        if (key == ARROW_UP)
        {
            if (themeMakerMenu.selected > 0)
                themeMakerMenu.selected--;
            continue;
        }

        if (key == ARROW_DOWN)
        {
            if (themeMakerMenu.selected < THEME_MAKER_COLOR_COUNT)
                themeMakerMenu.selected++;
            continue;
        }

        if (key == ARROW_LEFT || key == ARROW_RIGHT)
        {
            if (themeMakerMenu.selected > 0)
            {
                int index = themeMakerMenu.selected - 1;

                if (key == ARROW_LEFT &&
                    themeMakerMenu.codes[index] > 0)
                    themeMakerMenu.codes[index]--;

                if (key == ARROW_RIGHT &&
                    themeMakerMenu.codes[index] < 107)
                    themeMakerMenu.codes[index]++;

                themePreviewCustom(
                    themeMakerMenu.name,
                    themeMakerMenu.codes
                );
            }

            continue;
        }

        if (themeMakerMenu.selected == 0)
        {
            if (key == 127)
            {
                if (themeMakerMenu.nameLength > 1)
                {
                    themeMakerMenu.nameLength--;
                    themeMakerMenu.name[themeMakerMenu.nameLength] = '\0';

                    themePreviewCustom(
                        themeMakerMenu.name,
                        themeMakerMenu.codes
                    );
                }

                continue;
            }

            if (key >= 32 && key <= 126 &&
                themeMakerMenu.nameLength <
                    THEME_MAKER_NAME_SIZE - 1)
            {
                int valid =
                    (key >= 'a' && key <= 'z') ||
                    (key >= 'A' && key <= 'Z') ||
                    (key >= '0' && key <= '9') ||
                    key == ' ' || key == '-' || key == '_';

                if (valid)
                {
                    themeMakerMenu.name[
                        themeMakerMenu.nameLength++
                    ] = (char)key;

                    themeMakerMenu.name[
                        themeMakerMenu.nameLength
                    ] = '\0';

                    themePreviewCustom(
                        themeMakerMenu.name,
                        themeMakerMenu.codes
                    );
                }
            }

            continue;
        }

        /* Consume every key while the Theme Maker is active. */
        continue;
    }

    /* ---------- Status Bar Settings Menu ---------- */
    if (statusBarSettingsMenu.active)
    {
        if (key == 27)
        {
            closeStatusBarSettings();
            editorSetStatusMessage("Status bar settings cancelled");
            continue;
        }

        if (key == ARROW_UP)
        {
            if (statusBarSettingsMenu.selected > 0)
                statusBarSettingsMenu.selected--;
            continue;
        }

        if (key == ARROW_DOWN)
        {
            if (statusBarSettingsMenu.selected <
                STATUSBAR_OPTION_COUNT - 1)
                statusBarSettingsMenu.selected++;
            continue;
        }

        if (key == ' ')
        {
            int selected = statusBarSettingsMenu.selected;
            statusBarSettingsMenu.values[selected] =
                !statusBarSettingsMenu.values[selected];
            continue;
        }

        if (key == '\r' || key == '\n')
        {
            if (saveStatusBarSettings())
            {
                closeStatusBarSettings();
                editorSetStatusMessage(
                    "Status bar settings saved"
                );
            }
            else
            {
                editorSetStatusMessage(
                    "Could not save status bar settings"
                );
            }

            continue;
        }

        continue;
    }

    /* ---------- Layout Settings Menu ---------- */
        if (layoutSettingsMenu.active)
        {
            if (key == 27)
            {
                closeLayoutSettings();
                editorSetStatusMessage("Layout settings cancelled");
                continue;
            }

            if (key == ARROW_UP)
            {
                if (layoutSettingsMenu.selected > 0)
                    layoutSettingsMenu.selected--;
                continue;
            }

            if (key == ARROW_DOWN)
            {
                if (layoutSettingsMenu.selected < LAYOUT_OPTION_COUNT - 1)
                    layoutSettingsMenu.selected++;
                continue;
            }

            if (key == ' ')
            {
                int selected = layoutSettingsMenu.selected;
                layoutSettingsMenu.values[selected] =
                    !layoutSettingsMenu.values[selected];
                continue;
            }

            if (key == '\r' || key == '\n')
            {
                applyLayoutSettings();
                closeLayoutSettings();
                editorSetStatusMessage("Layout settings applied");
                continue;
            }

            continue;
        }

    /* Clear active search highlight */
    if (key == 27 &&
    searchState.matchCount > 0)
    {
    clearSearchResults();

    editorSetStatusMessage(
        "Search cancelled"
    );

    continue;
    }

    /* ---------- Git View (read-only) ---------- */
if (gitView.active)
{
    if (key == 27 || key == 'q')
    {
        closeGitView();

        editorSetStatusMessage(
            "Returned to file"
        );

        continue;
    }

    /* Allow navigation only — block all editing in Git/Plugin views. */
    if (key == ARROW_UP)
        moveCursorUp();
    else if (key == ARROW_DOWN)
        moveCursorDown();
    else if (key == ARROW_LEFT)
        moveCursorLeft();
    else if (key == ARROW_RIGHT)
        moveCursorRight();

    scrollEditor();
    continue;
}

   /* ---------- Plugin Manager ---------- */
if (pluginManager.active)
{
    /* Esc / q */
    if (key == 27 || key == 'q')
    {
        if (pluginManager.detailsView)
        {
            pluginManager.detailsView = 0;

            editorSetStatusMessage(
                "Plugin Dashboard"
            );
        }
        else
        {
            closePluginManager();
        }

        continue;
    }

    /* Dashboard Navigation */
    if (!pluginManager.detailsView)
    {
        if (key == ARROW_UP || key == 'k')
        {
            if (pluginManager.selected > 0)
            {
                pluginManager.selected--;
            }

            continue;
        }

        if (key == ARROW_DOWN || key == 'j')
        {
            if (
                pluginManager.selected <
                pluginCount() - 1
            )
            {
                pluginManager.selected++;
            }

            continue;
        }
    }

    /* Open Details View */
    if (
        key == '\r' ||
        key == '\n' ||
        key == 10 ||
        key == 13
    )
    {
        pluginManager.detailsView = 1;

        editorSetStatusMessage(
            "Plugin Details"
        );

        continue;
    }

        /* Toggle Plugin */
    if (
        key == ' ' ||
        key == 't' ||
        (
            event.key == 'v' &&
            (event.modifiers & MOD_CTRL)
        )
    )
    {
        pluginToggle();

        Plugin *p =
            pluginGet(
                pluginManager.selected
            );

        if (p)
        {
            char msg[128];

            snprintf(
                msg,
                sizeof(msg),
                "%s %s",
                p->name,
                p->enabled
                    ? "Enabled"
                    : "Disabled"
            );

            editorSetStatusMessage(msg);
        }

        continue;
    }

    /* Toggle Autoload */
    if (key == 'a')
    {
        Plugin *p =
            pluginGet(
                pluginManager.selected
            );

        if (p)
        {
            p->autoLoad =
                !p->autoLoad;

            char msg[128];

            snprintf(
                msg,
                sizeof(msg),
                "Autoload %s",
                p->autoLoad
                    ? "Enabled"
                    : "Disabled"
            );

            editorSetStatusMessage(msg);
        }

        continue;
    }

    /* Toggle Events */
    if (key == 'e')
    {
        Plugin *p =
            pluginGet(
                pluginManager.selected
            );

        if (p)
        {
            p->allowEvents =
                !p->allowEvents;

            char msg[128];

            snprintf(
                msg,
                sizeof(msg),
                "Events %s",
                p->allowEvents
                    ? "Enabled"
                    : "Disabled"
            );

            editorSetStatusMessage(msg);
        }

        continue;
    }

    /* Health Check */
    if (
        key == 'h' ||
        (
            event.key == 'h' &&
            (event.modifiers & MOD_ALT)
        )
    )
    {
        pluginHealthCheck();

        editorSetStatusMessage(
            "Plugin health checked"
        );

        continue;
    }

    /* Reload Plugins */
    if (
        key == 'r' ||
        (
            event.key == 'k' &&
            (event.modifiers & MOD_CTRL)
        )
    )
    {
        pluginReloadAll();

        editorSetStatusMessage(
            "Plugins reloaded successfully"
        );

        continue;
    }

    /* Ignore all other keys */
    continue;
}

    /* ---------- File Explorer modal input ---------- */
    if (explorerState.active)
    {
        if (key == 27)
        {
            explorerClose();
            renderForceClear();
            continue;
        }

        if (key == ARROW_UP)
        {
            explorerSelectPrev();
            continue;
        }

        if (key == ARROW_DOWN)
        {
            explorerSelectNext();
            continue;
        }

        if (key == 127 || key == ARROW_LEFT)
        {
            explorerGoUp();
            continue;
        }

        if (key == '\r' || key == '\n')
        {
            if (explorerEnter())
            {
                bookmarkClearAll();
                viewport.rowOffset = 0;
                viewport.colOffset = 0;
                highlightInit();
                gitRefreshStatus(currentFile);
                renderForceClear();
            }
            continue;
        }

        continue;
    }

    /* ---------- Quick File Switcher modal input ---------- */
    if (fileSwitcherState.active)
    {
        if (key == 27)
        {
            fileSwitcherClose();
            renderForceClear();
            continue;
        }

        if (key == ARROW_UP)
        {
            fileSwitcherSelectPrev();
            continue;
        }

        if (key == ARROW_DOWN)
        {
            fileSwitcherSelectNext();
            continue;
        }

        if (key == 127)
        {
            fileSwitcherBackspace();
            continue;
        }

        if (key == '\r' || key == '\n')
        {
            const char *sel = fileSwitcherGetSelected();
            if (sel)
            {
                char chosen[512];
                snprintf(chosen, sizeof(chosen), "%s", sel);
                if (openFile(chosen))
                {
                    if (currentFile)
                        free(currentFile);
                    currentFile = strdup(chosen);
                    recentAddFile(chosen);
                    bookmarkClearAll();
                    viewport.rowOffset = 0;
                    viewport.colOffset = 0;
                    highlightInit();
                    gitRefreshStatus(currentFile);
                    renderForceClear();
                }
            }
            fileSwitcherClose();
            continue;
        }

        if (key >= 32 && key <= 126)
        {
            fileSwitcherAddChar((char)key);
            continue;
        }

        continue;
    }

    /* ---------- Go-to-line prompt modal input ---------- */
    if (gotoPrompt.active)
    {
        if (key == 27)
        {
            closeGotoPrompt();
            editorSetStatusMessage("Go-to-line cancelled");
            continue;
        }

        if (key == '\r' || key == '\n')
        {
            performGotoLine();
            closeGotoPrompt();
            scrollEditor();
            continue;
        }

        if (key == 127)
        {
            if (gotoPrompt.length > 0)
            {
                gotoPrompt.input[--gotoPrompt.length] = '\0';
            }
            continue;
        }

        if ((key >= '0' && key <= '9') || key == ':')
        {
            if (gotoPrompt.length < (int)sizeof(gotoPrompt.input) - 1)
            {
                gotoPrompt.input[gotoPrompt.length++] = (char)key;
                gotoPrompt.input[gotoPrompt.length] = '\0';
            }
            continue;
        }

        continue;
    }

    /* ---------- Save-as prompt modal input ---------- */
    if (savePrompt.active)
    {
        if (key == 27)
        {
            closeSavePrompt();
            editorSetStatusMessage("Save cancelled");
            continue;
        }

        if (key == '\r' || key == '\n')
        {
            if (savePrompt.length == 0)
            {
                editorSetStatusMessage(
                    "Filename cannot be empty");
                continue;
            }

           if (saveFile(savePrompt.filename))
{
    currentFile =
        strdup(savePrompt.filename);

    highlightInit();

    gitRefreshStatus(currentFile);

    static char saveBuf[320];

    snprintf(
        saveBuf,
        sizeof(saveBuf),
        "Saved %s",
        currentFile
    );

    editorSetStatusMessage(saveBuf);
}
            else
            {
                static char errBuf[320];

                snprintf(
                    errBuf,
                    sizeof(errBuf),
                    "Could not save %s",
                    savePrompt.filename
                );

                editorSetStatusMessage(errBuf);
            }

            closeSavePrompt();
            continue;
        }

        if (key == 127)
        {
            if (savePrompt.length > 0)
            {
                savePrompt.length--;
                savePrompt.filename[
                    savePrompt.length
                ] = '\0';
            }

            continue;
        }

        if (key >= 32 && key <= 126 &&
            savePrompt.length < 255)
        {
            savePrompt.filename[
                savePrompt.length++
            ] = key;

            savePrompt.filename[
                savePrompt.length
            ] = '\0';
        }

        continue;
    }

    if (searchState.active)
{
    if (key == 27)
    {
        clearSearchResults();
        closeSearchPrompt();

        editorSetStatusMessage(
            searchState.replaceMode
                ? "Replace cancelled"
                : "Search cancelled"
        );

        continue;
    }
    if (searchState.replaceMode &&
        (event.key == 'a' && (event.modifiers & MOD_CTRL)))
{
    replaceAllMatches();

    closeSearchPrompt();

    continue;
}

    if (key == '\r' || key == '\n')
{
    if (searchState.replaceMode)
    {
        replaceCurrentMatch();

        if (searchState.matchCount == 0)
        {
            closeSearchPrompt();
        }

        continue;
    }
    else
    {
        performSearch();

        closeSearchPrompt();

        continue;
    }
}

    if (key == 127)
    {
        if (searchState.replaceMode)
        {
            if (searchState.replacementLength > 0)
            {
                searchState.replacementLength--;

                searchState.replacement[
                    searchState.replacementLength
                ] = '\0';
            }
        }
        else
        {
            if (searchState.length > 0)
            {
                searchState.length--;

                searchState.query[
                    searchState.length
                ] = '\0';
            }
        }

        continue;
    }

    if (searchState.replaceMode)
    {
        if (key >= 32 &&
            key <= 126 &&
            searchState.replacementLength < 127)
        {
            searchState.replacement[
                searchState.replacementLength++
            ] = key;

            searchState.replacement[
                searchState.replacementLength
            ] = '\0';
        }
    }
    else
    {
        if (key >= 32 &&
            key <= 126 &&
            searchState.length < 127)
        {
            searchState.query[
                searchState.length++
            ] = key;

            searchState.query[
                searchState.length
            ] = '\0';
        }
    }

    continue;
}

    if (key == -1)
        return;

    /* Check if this key has a command binding. */
   int action =
    keymapLookup(
        event.key,
        event.modifiers
    );

    /* Reset quit confirmation on any key except quit. */
    if (action != CMD_QUIT)
        quitRequested = 0;

    /* Handle quit specially because of the confirmation flow. */
    if (action == CMD_QUIT)
{
    if (buffer.modified && !quitRequested)
    {
        quitRequested = 1;

        snprintf(
            statusMessage,
            sizeof(statusMessage),
            "Unsaved changes. Press Ctrl-X again to quit."
        );

        scrollEditor();
        continue;
    }

    sessionAutoSave();
    recentSaveDefault();

    pluginTriggerEvent(
        PLUGIN_EVENT_EXIT
    );

    return;
}

    /* Dispatch bound commands. */
    if (action != CMD_NONE)
    {
        commandExecute(action);
        scrollEditor();
        continue;
    }

    /* Keys that are not bound to commands — editing and navigation. */
    switch (key)
    {
        case ARROW_UP:
            moveCursorUp();
            break;

        case ARROW_DOWN:
            moveCursorDown();
            break;

        case ARROW_LEFT:
            moveCursorLeft();
            break;

        case ARROW_RIGHT:
            moveCursorRight();
            break;

        case '\r':
        case '\n':
            insertNewLine();
            break;

        case '\t':
        indentLine();
        break;

        case BACKTAB:
            unindentLine();
            break;

        case HOME_KEY:
            moveCursorToStart();
            break;

        case END_KEY:
            moveCursorToEnd();
            break;

        case 127:
            deleteChar();
            break;

        case DELETE_KEY:
            deleteForward();
            break;

        default:
            if (key >= 32 && key <= 126)
            {
                if (!handleSmartCharacter((char)key))
                    insertChar((char)key);
            }
            break;
    }
    /* Update viewport after cursor movement */
    scrollEditor();

}
}

void editorShutdown(void)
{
    disableRawMode();

    printf("\033[?25h");   // show cursor
    printf("\033[2J");     // clear screen
    printf("\033[H");      // move to top-left
    fflush(stdout);

    bufferFree();

    if (currentFile)
    {
        free(currentFile);
        currentFile = NULL;
    }
}

const char *editorStatusMessage(void)
{
    return statusMessage;
}

void editorSetStatusMessage(const char *message)
{
    snprintf(statusMessage, sizeof(statusMessage), "%s", message);
}
void openGitView(
    const char *tempFile
)
{
    gitView.active = 1;

    gitView.pluginLog = 0;

    if (currentFile)
    {
        snprintf(
            gitView.originalFile,
            sizeof(gitView.originalFile),
            "%s",
            currentFile
        );
    }
    else
    {
        gitView.originalFile[0] = '\0';
    }

    openFile(tempFile);
}



void closeGitView(void)
{
    if (!gitView.active)
        return;

    if (strlen(gitView.originalFile) > 0)
    {
        openFile(
            gitView.originalFile
        );
    }
    else
    {
        bufferFree();
        bufferInit();
        cursor.x = 0;
        cursor.y = 0;
    }

    gitView.active = 0;
    gitView.pluginLog = 0;

    remove(".claw_blame.tmp");
    renderForceClear();
}

void openPluginManager(void)
{
    pluginManager.active = 1;
    pluginManager.selected = 0;

    pluginManager.detailsView = 0;

    editorSetStatusMessage(
        "Plugin Manager"
    );
}

void closePluginManager(void)
{
    pluginManager.active = 0;

    pluginManager.detailsView = 0;

    renderForceClear();

    editorSetStatusMessage(
        "Returned to editor"
    );
}

/* ---------- Custom Theme Maker ---------- */

static const int themeMakerDefaultCodes[THEME_MAKER_COLOR_COUNT] = {
    36, /* keyword */
    34, /* type */
    32, /* string */
    35, /* number */
    90, /* comment */
    33, /* preprocessor */
    37, /* operator */
    96, /* match */
    37  /* normal */
};

void resetThemeMaker(void)
{
    snprintf(
        themeMakerMenu.name,
        sizeof(themeMakerMenu.name),
        "%s",
        "MyTheme"
    );

    themeMakerMenu.nameLength =
        (int)strlen(themeMakerMenu.name);

    for (int i = 0; i < THEME_MAKER_COLOR_COUNT; i++)
    {
        themeMakerMenu.codes[i] =
            themeMakerDefaultCodes[i];
    }

    themePreviewCustom(
        themeMakerMenu.name,
        themeMakerMenu.codes
    );
}

void openThemeMaker(void)
{
    themeMakerMenu.active = 1;
    themeMakerMenu.selected = 0;
    themeMakerMenu.editingIndex = -1;

    resetThemeMaker();

    editorSetStatusMessage("");
}

void closeThemeMaker(void)
{
    themeCancelPreview();
    themeMakerMenu.active = 0;
    themeMakerMenu.editingIndex = -1;

    renderForceClear();
}

int saveThemeMaker(void)
{
    int success;

    if (themeMakerMenu.editingIndex >= 0)
    {
        success = themeUpdateCustom(
            (size_t)themeMakerMenu.editingIndex,
            themeMakerMenu.name,
            themeMakerMenu.codes
        );
    }
    else
    {
        success = themeSaveCustom(
            themeMakerMenu.name,
            themeMakerMenu.codes
        );
    }

    if (!success)
    {
        editorSetStatusMessage(
            "Could not save theme; check its name, uniqueness, and file"
        );

        return 0;
    }

    themeMakerMenu.active = 0;
    themeMakerMenu.editingIndex = -1;
    renderForceClear();

    editorSetStatusMessage("Custom theme saved and activated");

    return 1;
}



void drawThemeMaker(void)
{
    static const char *labels[THEME_MAKER_COLOR_COUNT] = {
        "Keyword",
        "Type",
        "String",
        "Number",
        "Comment",
        "Preprocessor",
        "Operator",
        "Match",
        "Normal text"
    };

    /* Display name + nine color parameters. */
    int rows = viewport.screenRows;
    int cols = viewport.screenCols;
    int panelWidth = cols - 4;

    if (panelWidth > 64)
        panelWidth = 64;
    if (panelWidth < 36)
        panelWidth = cols;
    if (panelWidth > cols)
        panelWidth = cols;

    int left = (cols - panelWidth) / 2;
    if (left < 0)
        left = 0;

    printf("\033[2J");

    int row = 1;
    printf("\033[%d;%dH+", row++, left + 1);
    for (int i = 0; i < panelWidth - 2; i++)
        putchar('-');
    putchar('+');

    const char *title = "CUSTOM THEME MAKER";
    printf("\033[%d;%dH|", row, left + 1);
    printf("\033[%d;%dH%s",
           row, left + (panelWidth - (int)strlen(title)) / 2 + 1,
           title);
    printf("\033[%d;%dH|", row++, left + panelWidth);

    printf("\033[%d;%dH+", row++, left + 1);
    for (int i = 0; i < panelWidth - 2; i++)
        putchar('-');
    putchar('+');

    row++;
    printf("\033[%d;%dHTheme name: %-*.*s",
           row++, left + 3,
           panelWidth - 16, panelWidth - 16,
           themeMakerMenu.name);

    for (int i = 0; i < THEME_MAKER_COLOR_COUNT; i++)
    {
        int codeIndex = i;
        int selected = themeMakerMenu.selected == i + 1;

        printf("\033[%d;%dH", row, left + 1);
        putchar('|');

        if (selected)
            printf("\033[7m");

        printf("\033[%d;%dH%-16s ANSI: %3d",
               row, left + 3,
               labels[i], themeMakerMenu.codes[codeIndex]);

        printf("\033[0m\033[%d;%dH|", row++, left + panelWidth);
    }

    row++;
    printf("\033[%d;%dHLive preview:", row++, left + 3);
    printf("\033[%d;%dH", row++, left + 3);

    printf("%sint ", themeColor(HL_KEYWORD));
    printf("%smain", themeColor(HL_TYPE));
    printf("%s(void) {", themeColor(HL_NORMAL));
    printf("\033[0m");

    printf("\033[%d;%dH", row++, left + 5);
    printf("%schar *s = ", themeColor(HL_TYPE));
    printf("%s\"Hello, CLAW!\"", themeColor(HL_STRING));
    printf("\033[0m;");

    printf("\033[%d;%dH", row++, left + 5);
    printf("%s// Sample comment", themeColor(HL_COMMENT));
    printf("\033[0m");

    printf("\033[%d;%dH", row++, left + 5);
    printf("%sreturn ", themeColor(HL_KEYWORD));
    printf("%s0", themeColor(HL_NUMBER));
    printf("%s;", themeColor(HL_NORMAL));
    printf("\033[0m");

    printf("\033[%d;%dH", row++, left + 3);
    printf("%s}", themeColor(HL_NORMAL));
    printf("\033[0m");

    row++;
    printf("\033[%d;%dH%-*.*s",
           row++, left + 3,
           panelWidth - 6, panelWidth - 6,
           "Up/Down: Select  Left/Right: Adjust color");

    printf("\033[%d;%dH%-*.*s",
           row++, left + 3,
           panelWidth - 6, panelWidth - 6,
           "Ctrl+S: Save  Ctrl+R: Reset  Esc: Cancel");

    if (rows > 0 && row > rows)
        printf("\033[%d;1H", rows);

    printf("\033[0m\033[?25l");
    fflush(stdout);
}


/* ---------- Custom Theme Manager ---------- */

void openThemeManager(void)
{
    themeManagerMenu.active = 1;
    themeManagerMenu.selected = 0;
    themeManagerMenu.confirmDelete = 0;
    themeManagerMenu.pathPromptActive = 0;
    themeManagerMenu.exportMode = 0;
    themeManagerMenu.pathLength = 0;
    themeManagerMenu.path[0] = '\0';
    editorSetStatusMessage("");
    themeManagerMenu.message[0] = '\0';
}

void closeThemeManager(void)
{
    themeManagerMenu.active = 0;
    themeManagerMenu.confirmDelete = 0;
    renderForceClear();
}

void drawThemeManager(void)
{
    int width = 60;
    int height = 14;
    int startX = (viewport.screenCols - width) / 2;
    int startY = (viewport.screenRows - height) / 2;

    size_t count = themeCustomCount();

    printf("\033[2J\033[H");
    printf("\033[%d;%dH", startY, startX);
    printf("\033[1;36m+----------------------------------------------------------+\033[0m");

    printf("\033[%d;%dH", startY + 1, startX);
    printf("\033[1;36m|                  CUSTOM THEME MANAGER                   |\033[0m");

    printf("\033[%d;%dH", startY + 2, startX);
    printf("\033[1;36m+----------------------------------------------------------+\033[0m");

    printf("\033[%d;%dH", startY + 3, startX);
    printf("| Current theme: %-42.42s |", themeCurrentName());

    if (count == 0)
    {
        printf("\033[%d;%dH", startY + 5, startX + 3);
        printf("No custom themes saved yet.");
        printf("\033[%d;%dH", startY + 7, startX + 3);
        printf("Use Alt+3 to create one in Theme Maker.");
    }
    else
    {
        int visible = (count < 5) ? (int)count : 5;

        for (int i = 0; i < visible; i++)
        {
            size_t index = (size_t)i;
            const char *name = themeCustomName(index);
            int active = name &&
                         strcmp(name, themeCurrentName()) == 0;

            printf("\033[%d;%dH", startY + 5 + i, startX + 3);

            if ((int)index == themeManagerMenu.selected)
                printf("\033[7m");

            printf("%c %-36.36s %s",
                   ((int)index == themeManagerMenu.selected) ? '>' : ' ',
                   name ? name : "(invalid)",
                   active ? "[ACTIVE]" : "");

            printf("\033[0m");
        }
    }

    printf("\033[%d;%dH", startY + 11, startX);
    printf("\033[1;36m+----------------------------------------------------------+\033[0m");

    if (themeManagerMenu.pathPromptActive)
    {
        printf("\033[%d;%dH", startY + 10, startX);
        printf("| %s: %-48.48s |",
               themeManagerMenu.exportMode
                   ? "Export path"
                   : "Import path",
               themeManagerMenu.path);
    }
    else if (themeManagerMenu.message[0] != '\0')
    {
        printf("\033[%d;%dH", startY + 10, startX + 2);
        printf("\033[1;33m%-56.56s\033[0m",
               themeManagerMenu.message);
    }

    printf("\033[%d;%dH", startY + 12, startX);

    if (themeManagerMenu.confirmDelete && count > 0 &&
        (size_t)themeManagerMenu.selected < count)
    {
        printf("Delete '%s'? Y: confirm  N/Esc: cancel",
               themeCustomName((size_t)themeManagerMenu.selected));
    }
    else
    {
        printf("Up/Down: select Enter: activate M: modify D: delete I: import E: export");
    }

    printf("\033[%d;%dH", startY + 13, startX);
    printf("\033[1;36m+----------------------------------------------------------+\033[0m");
}
