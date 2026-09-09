#include <stdio.h>

#include "command.h"
#include "buffer.h"
#include "fileio.h"
#include "search.h"
#include "history.h"
#include "editor.h"

extern char *currentFile;
extern Buffer buffer;

void commandExecute(int action)
{
    switch (action)
    {
        case CMD_SAVE:
        {
            static char statusBuf[128];

            if (currentFile && saveFile(currentFile))
                snprintf(statusBuf, sizeof(statusBuf),
                         "Saved %s", currentFile);
            else if (!currentFile)
                snprintf(statusBuf, sizeof(statusBuf),
                         "No filename to save");
            else
                snprintf(statusBuf, sizeof(statusBuf),
                         "Could not save %s", currentFile);

            editorSetStatusMessage(statusBuf);
            break;
        }

        case CMD_SEARCH:
            openSearchPrompt();
            break;
        
        case CMD_REPLACE:
            openReplacePrompt();
            break;

        case CMD_UNDO:
            historyUndo();
            break;

        case CMD_REDO:
            historyRedo();
            break;

        case CMD_REPLACE_ALL:
            replaceAllMatches();
            break;

        case CMD_NEXT_MATCH:
            nextMatch();
            break;

        case CMD_PREV_MATCH:
            previousMatch();
            break;

        default:
            break;
    }
}
