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
char *currentFile = NULL;
static char statusMessage[128] = "";
static int quitRequested = 0;

SavePrompt savePrompt = {0};

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

void editorInit(void)
{
    /* Raw mode must be active before the first screen refresh or key read. */
    enableRawMode();
    printf("\033[2 q");

    /* Load configuration before anything that reads settings.
       Order: compiled-in defaults → bundled defaults → user config.
       User config is searched at XDG_CONFIG_HOME/claw/claw.conf first
       (typically ~/.config/claw/claw.conf), then falls back to the
       relative config/ directory for development convenience. */
    configInit();
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

    char branch[128];

gitCurrentBranch(
    branch,
    sizeof(branch)
);
    }
void editorRun(void)
{
    /* Redraw current state, wait for one command, then apply it. */
    while (1)
{
    viewportUpdateSize();
    refreshScreen();

    int key = readKey();

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
        closeSearchPrompt();

        editorSetStatusMessage(
            searchState.replaceMode
                ? "Replace cancelled"
                : "Search cancelled"
        );

        continue;
    }
    if (searchState.replaceMode &&
    key == CTRL_KEY('a'))
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
    int action = keymapLookup(key);

    /* Reset quit confirmation on any key except quit. */
    if (action != CMD_QUIT)
        quitRequested = 0;

    /* Handle quit specially because of the confirmation flow. */
    if (action == CMD_QUIT)
    {
        if (buffer.modified && !quitRequested)
        {
            quitRequested = 1;
            snprintf(statusMessage,
                     sizeof(statusMessage),
                     "Unsaved changes. Press Ctrl-X again to quit.");
            scrollEditor();
            continue;
        }
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
        {
            int width = configTabWidth();

            for (int i = 0; i < width; i++)
                insertChar(' ');

            break;
        }

        case 127:
            deleteChar();
            break;

        case DELETE_KEY:
            deleteForward();
            break;

        default:
            if (key >= 32 && key <= 126)
                insertChar(key);
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
}

const char *editorStatusMessage(void)
{
    return statusMessage;
}

void editorSetStatusMessage(const char *message)
{
    snprintf(statusMessage, sizeof(statusMessage), "%s", message);
}