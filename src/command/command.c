#include <stdio.h>

#include "command.h"
#include "buffer.h"
#include "fileio.h"
#include "search.h"
#include "history.h"
#include "editor.h"
#include "git.h"
#include "plugin.h"
#include "plugin_commands.h"

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
            {
                gitRefreshStatus(currentFile);

                snprintf(
                    statusBuf,
                    sizeof(statusBuf),
                    "Saved %s",
                    currentFile
                );
            }
            else
            {
                snprintf(
                    statusBuf,
                    sizeof(statusBuf),
                    "Could not save %s",
                    currentFile
                );
            }

            editorSetStatusMessage(statusBuf);
            break;
        }

        case CMD_GIT_ADD:
        {
        if (!currentFile)
        {
        editorSetStatusMessage(
            "No file to add"
        );
        break;
        }

        if (gitAddFile(currentFile))
        {
        static char msg[256];

        snprintf(
            msg,
            sizeof(msg),
            "Added %s to staging",
            currentFile
        );

        editorSetStatusMessage(msg);
    }
    else
    {
        editorSetStatusMessage(
            "Git add failed"
        );
    }

    break;
}

case CMD_GIT_RESTORE:
{
    if (!currentFile)
    {
        editorSetStatusMessage(
            "No file to restore"
        );
        break;
    }

    if (gitRestoreFile(currentFile))
    {
        if (openFile(currentFile))
        {
            static char msg[256];

            snprintf(
                msg,
                sizeof(msg),
                "Restored %s",
                currentFile
            );

            editorSetStatusMessage(msg);
        }
        else
        {
            editorSetStatusMessage(
                "Restore succeeded but reload failed"
            );
        }
    }
    else
    {
        editorSetStatusMessage(
            "Git restore failed"
        );
    }

    break;
}

case CMD_GIT_UNSTAGE:
{
    static char statusBuf[128];

    if (!currentFile)
    {
        editorSetStatusMessage(
            "No file selected"
        );
        break;
    }

    if (gitUnstageFile(currentFile))
    {
        gitRefreshStatus(currentFile);

        snprintf(
            statusBuf,
            sizeof(statusBuf),
            "Unstaged %s",
            currentFile
        );
    }
    else
    {
        snprintf(
            statusBuf,
            sizeof(statusBuf),
            "Could not unstage %s",
            currentFile
        );
    }

    editorSetStatusMessage(statusBuf);

    break;
}

    case CMD_GIT_BLAME:
{
    if (!currentFile)
    {
        editorSetStatusMessage(
            "No file selected"
        );
        break;
    }

    if (
        !gitBlameFile(
            currentFile,
            ".claw_blame.tmp"
        )
    )
    {
        editorSetStatusMessage(
            "Git blame failed"
        );

        break;
    }

    openGitView(
        ".claw_blame.tmp"
    );

    editorSetStatusMessage(
        "Git blame (Esc to return)"
    );

    break;
}
        case CMD_PLUGIN_LIST:
{
    int count = pluginCount();

    char message[64];

    snprintf(
        message,
        sizeof(message),
        "%d plugins loaded",
        count
    );

    editorSetStatusMessage(message);
    break;
}

case CMD_PLUGIN_RELOAD:
{
    pluginReloadAll();

    editorSetStatusMessage(
        "Plugins reloaded"
    );

    break;
}

        case CMD_PLUGIN_HELLO:
        pluginExecute("hello");
        break;

        case CMD_PLUGIN_STATS:
        pluginExecute("stats");
        break;

        case CMD_PLUGIN_FORMAT:
        pluginExecute("format");
        break;

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