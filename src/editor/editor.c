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
char *currentFile = NULL;
static char statusMessage[128] = "";
static int quitRequested = 0;

void editorInit(void)
{
    /* Raw mode must be active before the first screen refresh or key read. */
    enableRawMode();

    bufferInit();
}

void editorRun(void)
{
    /* Redraw current state, wait for one command, then apply it. */
    while(1)
    {
        refreshScreen();

        int key = readKey();

        if (key == -1)
            return;

        if (key != CTRL_KEY('q'))
            quitRequested = 0;

      switch(key)
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
    
    case CTRL_KEY('s'):
    if (currentFile && saveFile(currentFile))
        snprintf(statusMessage, sizeof(statusMessage), "Saved %s", currentFile);
    else if (!currentFile)
        snprintf(statusMessage, sizeof(statusMessage), "No filename to save");
    else
        snprintf(statusMessage, sizeof(statusMessage), "Could not save %s", currentFile);

    break;

    case CTRL_KEY('q'):
        /* Require confirmation rather than discarding unsaved edits. */
        if (buffer.modified && !quitRequested)
        {
            quitRequested = 1;
            snprintf(statusMessage, sizeof(statusMessage), "Unsaved changes. Press Ctrl-Q again to quit.");
            break;
        }
        return;

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
    }
}

void editorShutdown(void)
{
    /* Explicit cleanup complements the atexit terminal restoration fallback. */
    disableRawMode();
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