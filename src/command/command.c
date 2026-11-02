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
#include "plugin_api.h"
#include "indent.h"
#include "cursor.h"
#include "workspace.h"
#include "project.h"
#include "session.h"
#include "bookmark.h"
#include "explorer.h"
#include "file_switcher.h"
#include "viewport.h"
#include "highlight.h"
#include "recent.h"
#include "render.h"
#include "theme.h"
#include "layout.h"
#include "buffer_manager.h"
#include "split.h"
#include "terminal.h"
#include "symbols.h"
#include "lsp.h"
#include "dap.h"
#include "remote.h"
#include "templates.h"
#include "marketplace.h"
#include "package.h"

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
    bufferManagerSyncFromGlobal();
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

    gitView.pluginLog = 0;

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

        case CMD_PLUGIN_INFO:
        pluginShowInfo();
        break;

        case CMD_PLUGIN_HELLO:
        pluginExecute("hello");
        break;

        case CMD_PLUGIN_STATS:
        pluginExecute("stats");
        break;

        case CMD_PLUGIN_FORMAT:
        pluginExecute("format");
        break;

        case CMD_PLUGIN_STATS_VIEW:
        pluginShowStats();
        break;

        case CMD_PLUGIN_CLEAR_LOGS:
        pluginClearLogs();
        break;

        case CMD_PLUGIN_LOG:
        pluginOpenLog();
        break;

        case CMD_PLUGIN_MANAGER:
        openPluginManager();
        break;

        case CMD_PLUGIN_TOGGLE:
        pluginToggle();
        break;

        case CMD_PLUGIN_HEALTH:
        pluginHealthCheck();
        break;

        case CMD_PLUGIN_HELP:
        pluginShowCommands();
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

        case CMD_DUPLICATE_LINE:
        duplicateLine();
        break;

        case CMD_MOVE_LINE_UP:
        moveLineUp();
        break;

        case CMD_MOVE_LINE_DOWN:
        moveLineDown();
        break;

        case CMD_COMMENT_LINE:
        toggleCommentLine();
        break;

        case CMD_JOIN_LINES:
        joinLineWithNext();
        break;

        case CMD_UNINDENT:
        unindentLine();
        break;

        case CMD_GOTO_BRACKET:
        jumpToBracketMatch();
        break;

        case CMD_WORD_LEFT:
        moveCursorWordLeft();
        break;

        case CMD_WORD_RIGHT:
        moveCursorWordRight();
        break;

        case CMD_HOME:
        moveCursorToStart();
        break;

        case CMD_END:
        moveCursorToEnd();
        break;

        case CMD_GOTO_LINE:
        openGotoPrompt();
        break;

        case CMD_BOOKMARK_TOGGLE:
        {
            int res = bookmarkToggle(cursor.y);
            static char bmMsg[64];
            if (res == 1)
            {
                snprintf(bmMsg, sizeof(bmMsg), "Bookmark added at line %d", cursor.y + 1);
            }
            else if (res == 0)
            {
                snprintf(bmMsg, sizeof(bmMsg), "Bookmark removed from line %d", cursor.y + 1);
            }
            else
            {
                snprintf(bmMsg, sizeof(bmMsg), "Bookmark limit reached");
            }
            editorSetStatusMessage(bmMsg);
            break;
        }

        case CMD_BOOKMARK_NEXT:
        {
            int next = bookmarkNext(cursor.y);
            if (next >= 0)
            {
                cursor.y = next;
                cursor.x = 0;
                viewport.rowOffset = cursor.y - (viewport.screenRows / 2);
                if (viewport.rowOffset < 0)
                    viewport.rowOffset = 0;
                scrollEditor();
                static char bmMsg[64];
                snprintf(bmMsg, sizeof(bmMsg), "Jumped to bookmark at line %d", next + 1);
                editorSetStatusMessage(bmMsg);
            }
            else
            {
                editorSetStatusMessage("No bookmarks set");
            }
            break;
        }

        case CMD_BOOKMARK_PREV:
        {
            int prev = bookmarkPrev(cursor.y);
            if (prev >= 0)
            {
                cursor.y = prev;
                cursor.x = 0;
                viewport.rowOffset = cursor.y - (viewport.screenRows / 2);
                if (viewport.rowOffset < 0)
                    viewport.rowOffset = 0;
                scrollEditor();
                static char bmMsg[64];
                snprintf(bmMsg, sizeof(bmMsg), "Jumped to bookmark at line %d", prev + 1);
                editorSetStatusMessage(bmMsg);
            }
            else
            {
                editorSetStatusMessage("No bookmarks set");
            }
            break;
        }

        case CMD_BOOKMARK_CLEAR:
            bookmarkClearAll();
            editorSetStatusMessage("All bookmarks cleared");
            break;

        case CMD_QUICK_SWITCH:
            fileSwitcherOpen();
            break;

        case CMD_FILE_EXPLORER:
            explorerOpen(workspaceIsActive() ? workspaceGetRoot() : ".");
            break;

        case CMD_WORKSPACE_OPEN:
            workspaceInit();
            workspaceLoadConfig();
            projectInit();
            if (workspaceIsActive())
            {
                recentAddWorkspace(workspaceGetRoot());
            }
            {
                static char wsMsg[256];
                snprintf(wsMsg, sizeof(wsMsg), "Workspace: %s", workspaceGetName());
                editorSetStatusMessage(wsMsg);
            }
            break;

        case CMD_SESSION_SAVE:
            if (sessionAutoSave())
            {
                editorSetStatusMessage("Session saved successfully");
            }
            else
            {
                editorSetStatusMessage("Session save failed");
            }
            break;

        case CMD_SESSION_RESTORE:
            if (sessionAutoRestore())
            {
                highlightInit();
                gitRefreshStatus(currentFile);
                renderForceClear();
                editorSetStatusMessage("Session restored");
            }
            else
            {
                editorSetStatusMessage("No saved session found");
            }
            break;

        case CMD_PROJECT_BUILD:
            if (!projectIsActive())
            {
                editorSetStatusMessage("No project configuration found");
            }
            else if (!projectGetBuildCommand() ||
                     projectGetBuildCommand()[0] == '\0')
            {
                editorSetStatusMessage("No project build command configured");
            }
            else if (projectBuild())
            {
                editorSetStatusMessage("Project build succeeded");
            }
            else
            {
                editorSetStatusMessage("Project build failed");
            }
            break;

        case CMD_PROJECT_TEST:
            if (!projectIsActive())
            {
                editorSetStatusMessage("No project configuration found");
            }
            else if (!projectGetTestCommand() ||
                     projectGetTestCommand()[0] == '\0')
            {
                editorSetStatusMessage("No project test task configured");
            }
            else if (projectRunTask("test"))
            {
                editorSetStatusMessage("Project test task succeeded");
            }
            else
            {
                editorSetStatusMessage("Project test task failed");
            }
            break;

        case CMD_PROJECT_OUTPUT:
            if (projectOutput.length == 0)
                editorSetStatusMessage("No project output captured");
            else
                projectOutputOpen();
            break;

        case CMD_PROJECT_DIAGNOSTICS:
            if (projectDiagnosticCount() == 0)
            {
                editorSetStatusMessage("No project diagnostics");
            }
            else
            {
                projectDiagnosticsOpen();
            }
            break;

        case CMD_PROJECT_DIAGNOSTICS_NEXT:
            if (!projectDiagnosticsNext())
                editorSetStatusMessage("No project diagnostics");
            else
            {
                const ProjectDiagnostic *diagnostic =
                    projectGetSelectedDiagnostic();
                char message[256];
                snprintf(message, sizeof(message), "%.100s:%d: %.120s",
                         diagnostic->file, diagnostic->line,
                         diagnostic->message);
                editorSetStatusMessage(message);
            }
            break;

        case CMD_PROJECT_DIAGNOSTICS_PREVIOUS:
            if (!projectDiagnosticsPrevious())
                editorSetStatusMessage("No project diagnostics");
            else
            {
                const ProjectDiagnostic *diagnostic =
                    projectGetSelectedDiagnostic();
                char message[256];
                snprintf(message, sizeof(message), "%.100s:%d: %.120s",
                         diagnostic->file, diagnostic->line,
                         diagnostic->message);
                editorSetStatusMessage(message);
            }
            break;

        case CMD_PROJECT_DASHBOARD:
            projectDashboardOpen();
            break;

            case CMD_THEME_NEXT:
            {
                themeNext();
                editorSetStatusMessage("");

                break;
            }

            case CMD_STATUSBAR_SETTINGS:
                openStatusBarSettings();
                break;

            case CMD_LAYOUT_SETTINGS:
                openLayoutSettings();
                break;

            case CMD_THEME_MAKER:
                openThemeMaker();
                break;

            case CMD_THEME_MANAGER:
                openThemeManager();
                break;

            /* Phase 8 — IDE Foundation commands */
            case CMD_BUFFER_CLOSE:
                if (splitIsActive())
                {
                    splitClose();
                }
                else
                {
                    bufferManagerCloseActive();
                }
                break;

            case CMD_BUFFER_NEXT:
                if (bufferManagerNext())
                {
                    if (splitIsActive())
                    {
                        splitLayout.panes[splitLayout.activePane].bufferIndex = bufferManagerActiveIndex();
                    }
                    char buf[128];
                    snprintf(buf, sizeof(buf), "Buffer [%d/%d]: %s",
                             bufferManagerActiveIndex() + 1, bufferManagerCount(),
                             bufferManagerGetDisplayName(bufferManagerActiveIndex()));
                    editorSetStatusMessage(buf);
                }
                break;

            case CMD_BUFFER_PREV:
                if (bufferManagerPrev())
                {
                    if (splitIsActive())
                    {
                        splitLayout.panes[splitLayout.activePane].bufferIndex = bufferManagerActiveIndex();
                    }
                    char buf[128];
                    snprintf(buf, sizeof(buf), "Buffer [%d/%d]: %s",
                             bufferManagerActiveIndex() + 1, bufferManagerCount(),
                             bufferManagerGetDisplayName(bufferManagerActiveIndex()));
                    editorSetStatusMessage(buf);
                }
                break;

            case CMD_SPLIT_VERTICAL:
                splitVertical();
                break;

            case CMD_SPLIT_HORIZONTAL:
                splitHorizontal();
                break;

            case CMD_PANE_SWITCH:
                splitSwitchPane();
                break;

            case CMD_TERMINAL_TOGGLE:
                terminalToggle();
                break;

            case CMD_SYMBOL_SEARCH:
                symbolsOpen();
                break;

            case CMD_BUFFER_1:
            case CMD_BUFFER_2:
            case CMD_BUFFER_3:
            case CMD_BUFFER_4:
            case CMD_BUFFER_5:
            case CMD_BUFFER_6:
            case CMD_BUFFER_7:
            case CMD_BUFFER_8:
            case CMD_BUFFER_9:
            {
                int target = action - CMD_BUFFER_1;
                if (target < bufferManagerCount())
                {
                    bufferManagerSwitchTo(target);
                    if (splitIsActive())
                    {
                        splitLayout.panes[splitLayout.activePane].bufferIndex = target;
                    }
                    char buf[128];
                    snprintf(buf, sizeof(buf), "Buffer [%d/%d]: %s",
                             target + 1, bufferManagerCount(),
                             bufferManagerGetDisplayName(target));
                    editorSetStatusMessage(buf);
                }
                break;
            }

            /* Phase 9 — Source Control Expansion commands */
            case CMD_GIT_DASHBOARD:
                gitDashboardOpen();
                break;

            case CMD_GIT_DIFF:
                gitDiffViewerOpen(currentFile, 0);
                break;

            case CMD_GIT_LOG:
                gitLogViewerOpen();
                break;

            case CMD_GIT_COMMIT:
                gitCommitDialogOpen();
                break;

            case CMD_GIT_BRANCH:
                gitBranchModalOpen();
                break;

            case CMD_GIT_STASH:
                gitStashModalOpen();
                break;

            /* Phase 11 — Intelligent IDE Services Commands */
            case CMD_LSP_COMPLETION:
                lspCompletionOpen();
                break;

            case CMD_LSP_DEFINITION:
                lspGotoDefinition();
                break;

            case CMD_LSP_HOVER:
                lspHoverOpen();
                break;

            case CMD_LSP_REFERENCES:
                lspReferencesOpen();
                break;

            case CMD_LSP_CODE_ACTION:
                lspCodeActionsOpen();
                break;

            case CMD_LSP_DIAGNOSTICS:
            {
                int count = lspTotalDiagnosticsCount(currentFile);
                char dmsg[128];
                snprintf(dmsg, sizeof(dmsg), "Document diagnostics: %d issues recorded", count);
                editorSetStatusMessage(dmsg);
                break;
            }

            /* Phase 12 — Advanced Platform Services Commands */
            case CMD_DAP_START_CONTINUE:
                if (dapGetState() == DAP_STATE_STOPPED)
                    dapStartSession(currentFile, NULL);
                else
                    dapContinue();
                break;

            case CMD_DAP_STOP:
                dapStopSession();
                break;

            case CMD_DAP_PAUSE:
                dapPause();
                break;

            case CMD_DAP_BREAKPOINT_TOGGLE:
                if (currentFile && cursor.y >= 0)
                    dapToggleBreakpoint(currentFile, cursor.y + 1);
                else
                    editorSetStatusMessage("No active file for breakpoint");
                break;

            case CMD_DAP_STEP_OVER:
                dapStepOver();
                break;

            case CMD_DAP_STEP_INTO:
                dapStepInto();
                break;

            case CMD_DAP_STEP_OUT:
                dapStepOut();
                break;

            case CMD_DAP_DASHBOARD:
                dapDashboardOpen();
                break;

            case CMD_REMOTE_OPEN:
                remoteOpenPrompt();
                break;

            case CMD_PROJECT_NEW:
                templateWizardOpen();
                break;

            /* Phase 13 — Claw Ecosystem Commands */
            case CMD_MARKETPLACE:
            case CMD_PACKAGE_MANAGER:
                marketplaceOpen();
                break;

        default:
            break;
    }
}