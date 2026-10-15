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
        char userConfigPath[512];
        int loaded = 0;

        /* Try $XDG_CONFIG_HOME/claw/claw.conf first. */
        const char *xdg = getenv("XDG_CONFIG_HOME");

        if (xdg && strlen(xdg) > 0)
        {
            snprintf(userConfigPath, sizeof(userConfigPath),
                     "%s/claw/claw.conf", xdg);
            loaded = configLoad(userConfigPath);
        }

        /* Fall back to ~/.config/claw/claw.conf. */
        if (!loaded)
        {
            const char *home = getenv("HOME");

            if (home && strlen(home) > 0)
            {
                snprintf(userConfigPath, sizeof(userConfigPath),
                         "%s/.config/claw/claw.conf", home);
                loaded = configLoad(userConfigPath);
            }
        }

        /* Final fallback: relative config/claw.conf (dev mode). */
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