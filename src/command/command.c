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
            if (!currentFile)
            {
                openSavePrompt();
                break;
            }

            static char statusBuf[128];

            if (saveFile(currentFile))
                snprintf(statusBuf, sizeof(statusBuf),
                         "Saved %s", currentFile);
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
