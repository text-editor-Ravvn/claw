#include <stdio.h>
#include <string.h>
#include <strings.h>
#include <ctype.h>

#include "keymap.h"
#include "input.h"
#include "utils.h"

int keymapParseKey(const char *str)
{
    const char *plus =
        strrchr(str, '+');

    if (plus)
        str = plus + 1;

    if (
        strlen(str) == 1 &&
        isalpha(
            (unsigned char)str[0]
        )
    )
    {
        return tolower(
            (unsigned char)str[0]
        );
    }

    if (
        strlen(str) == 1 &&
        str[0] >= 32 &&
        str[0] <= 126
    )
    {
        return str[0];
    }

    if (strcasecmp(str, "Up") == 0)
        return ARROW_UP;

    if (strcasecmp(str, "Down") == 0)
        return ARROW_DOWN;

    if (strcasecmp(str, "Left") == 0)
        return ARROW_LEFT;

    if (strcasecmp(str, "Right") == 0)
        return ARROW_RIGHT;

    if (strcasecmp(str, "Home") == 0)
        return HOME_KEY;

    if (strcasecmp(str, "End") == 0)
        return END_KEY;

    if (strcasecmp(str, "Tab") == 0)
        return '\t';

    if (strcasecmp(str, "BackTab") == 0)
        return BACKTAB;

    if (strcasecmp(str, "Space") == 0)
        return ' ';

    if (strcasecmp(str, "F1") == 0) return KEY_F1;
    if (strcasecmp(str, "F2") == 0) return KEY_F2;
    if (strcasecmp(str, "F3") == 0) return KEY_F3;
    if (strcasecmp(str, "F4") == 0) return KEY_F4;
    if (strcasecmp(str, "F5") == 0) return KEY_F5;
    if (strcasecmp(str, "F6") == 0) return KEY_F6;
    if (strcasecmp(str, "F7") == 0) return KEY_F7;
    if (strcasecmp(str, "F8") == 0) return KEY_F8;
    if (strcasecmp(str, "F9") == 0) return KEY_F9;
    if (strcasecmp(str, "F10") == 0) return KEY_F10;
    if (strcasecmp(str, "F11") == 0) return KEY_F11;
    if (strcasecmp(str, "F12") == 0) return KEY_F12;

    return -1;
}

/* Map a human-readable action name to its enum value. */
static int parseActionName(const char *name)
{
    if (strcmp(name, "save") == 0)
        return CMD_SAVE;

    if (strcmp(name, "quit") == 0)
        return CMD_QUIT;

    if (strcmp(name, "search") == 0)
        return CMD_SEARCH;

    if (strcmp(name, "replace") == 0)
        return CMD_REPLACE;

    if (strcmp(name, "replace_all") == 0)
        return CMD_REPLACE_ALL;

    if (strcmp(name, "undo") == 0)
        return CMD_UNDO;

    if (strcmp(name, "redo") == 0)
        return CMD_REDO;

    if (strcmp(name, "next_match") == 0)
        return CMD_NEXT_MATCH;

    if (strcmp(name, "prev_match") == 0)
        return CMD_PREV_MATCH;

    if (strcmp(name, "duplicate_line") == 0)
        return CMD_DUPLICATE_LINE;

    if (strcmp(name, "move_line_up") == 0)
        return CMD_MOVE_LINE_UP;

    if (strcmp(name, "move_line_down") == 0)
        return CMD_MOVE_LINE_DOWN;

    if (strcmp(name, "comment_line") == 0)
        return CMD_COMMENT_LINE;

    if (strcmp(name, "join_lines") == 0)
        return CMD_JOIN_LINES;

    if (strcmp(name, "unindent") == 0)
        return CMD_UNINDENT;

    if (strcmp(name, "goto_bracket") == 0)
        return CMD_GOTO_BRACKET;

    if (strcmp(name, "word_left") == 0)
        return CMD_WORD_LEFT;

    if (strcmp(name, "word_right") == 0)
        return CMD_WORD_RIGHT;

    if (strcmp(name, "home") == 0)
        return CMD_HOME;

    if (strcmp(name, "end") == 0)
        return CMD_END;

    if (strcmp(name, "goto_line") == 0)
        return CMD_GOTO_LINE;

    if (strcmp(name, "bookmark_toggle") == 0)
        return CMD_BOOKMARK_TOGGLE;

    if (strcmp(name, "bookmark_next") == 0)
        return CMD_BOOKMARK_NEXT;

    if (strcmp(name, "bookmark_prev") == 0)
        return CMD_BOOKMARK_PREV;

    if (strcmp(name, "bookmark_clear") == 0)
        return CMD_BOOKMARK_CLEAR;

    if (strcmp(name, "quick_switch") == 0)
        return CMD_QUICK_SWITCH;

    if (strcmp(name, "file_explorer") == 0)
        return CMD_FILE_EXPLORER;

    if (strcmp(name, "workspace_open") == 0)
        return CMD_WORKSPACE_OPEN;

    if (strcmp(name, "session_save") == 0)
        return CMD_SESSION_SAVE;

    if (strcmp(name, "session_restore") == 0)
        return CMD_SESSION_RESTORE;

    if (strcmp(name, "statusbar_settings") == 0)
        return CMD_STATUSBAR_SETTINGS;

    if (strcmp(name, "layout_settings") == 0)
        return CMD_LAYOUT_SETTINGS;

    if (strcmp(name, "project_build") == 0)
        return CMD_PROJECT_BUILD;

    if (strcmp(name, "project_test") == 0)
        return CMD_PROJECT_TEST;

    if (strcmp(name, "project_output") == 0)
        return CMD_PROJECT_OUTPUT;

    if (strcmp(name, "project_diagnostics") == 0)
        return CMD_PROJECT_DIAGNOSTICS;

    if (strcmp(name, "project_diagnostics_next") == 0)
        return CMD_PROJECT_DIAGNOSTICS_NEXT;

    if (strcmp(name, "project_diagnostics_previous") == 0)
        return CMD_PROJECT_DIAGNOSTICS_PREVIOUS;

    if (strcmp(name, "project_dashboard") == 0)
        return CMD_PROJECT_DASHBOARD;

    /* Phase 8 IDE Foundation Actions */
    if (strcmp(name, "buffer_close") == 0)
        return CMD_BUFFER_CLOSE;

    if (strcmp(name, "buffer_next") == 0)
        return CMD_BUFFER_NEXT;

    if (strcmp(name, "buffer_prev") == 0)
        return CMD_BUFFER_PREV;

    if (strcmp(name, "split_vertical") == 0)
        return CMD_SPLIT_VERTICAL;

    if (strcmp(name, "split_horizontal") == 0)
        return CMD_SPLIT_HORIZONTAL;

    if (strcmp(name, "pane_switch") == 0)
        return CMD_PANE_SWITCH;

    if (strcmp(name, "terminal_toggle") == 0)
        return CMD_TERMINAL_TOGGLE;

    if (strcmp(name, "symbol_search") == 0)
        return CMD_SYMBOL_SEARCH;

    if (strcmp(name, "buffer_1") == 0) return CMD_BUFFER_1;
    if (strcmp(name, "buffer_2") == 0) return CMD_BUFFER_2;
    if (strcmp(name, "buffer_3") == 0) return CMD_BUFFER_3;
    if (strcmp(name, "buffer_4") == 0) return CMD_BUFFER_4;
    if (strcmp(name, "buffer_5") == 0) return CMD_BUFFER_5;
    if (strcmp(name, "buffer_6") == 0) return CMD_BUFFER_6;
    if (strcmp(name, "buffer_7") == 0) return CMD_BUFFER_7;
    if (strcmp(name, "buffer_8") == 0) return CMD_BUFFER_8;
    if (strcmp(name, "buffer_9") == 0) return CMD_BUFFER_9;

    /* Phase 9 Source Control Actions */
    if (strcmp(name, "git_dashboard") == 0) return CMD_GIT_DASHBOARD;
    if (strcmp(name, "git_diff") == 0) return CMD_GIT_DIFF;
    if (strcmp(name, "git_log") == 0) return CMD_GIT_LOG;
    if (strcmp(name, "git_commit") == 0) return CMD_GIT_COMMIT;
    if (strcmp(name, "git_branch") == 0) return CMD_GIT_BRANCH;
    if (strcmp(name, "git_stash") == 0) return CMD_GIT_STASH;

    /* Phase 11 Intelligent IDE Services Actions */
    if (strcmp(name, "lsp_completion") == 0) return CMD_LSP_COMPLETION;
    if (strcmp(name, "lsp_definition") == 0) return CMD_LSP_DEFINITION;
    if (strcmp(name, "lsp_hover") == 0) return CMD_LSP_HOVER;
    if (strcmp(name, "lsp_references") == 0) return CMD_LSP_REFERENCES;
    if (strcmp(name, "lsp_code_action") == 0) return CMD_LSP_CODE_ACTION;
    if (strcmp(name, "lsp_diagnostics") == 0) return CMD_LSP_DIAGNOSTICS;

    /* Phase 12 Advanced Platform Services Actions */
    if (strcmp(name, "dap_start") == 0 || strcmp(name, "dap_continue") == 0) return CMD_DAP_START_CONTINUE;
    if (strcmp(name, "dap_stop") == 0) return CMD_DAP_STOP;
    if (strcmp(name, "dap_pause") == 0) return CMD_DAP_PAUSE;
    if (strcmp(name, "dap_breakpoint_toggle") == 0) return CMD_DAP_BREAKPOINT_TOGGLE;
    if (strcmp(name, "dap_step_over") == 0) return CMD_DAP_STEP_OVER;
    if (strcmp(name, "dap_step_into") == 0) return CMD_DAP_STEP_INTO;
    if (strcmp(name, "dap_step_out") == 0) return CMD_DAP_STEP_OUT;
    if (strcmp(name, "dap_dashboard") == 0) return CMD_DAP_DASHBOARD;
    if (strcmp(name, "remote_open") == 0) return CMD_REMOTE_OPEN;
    if (strcmp(name, "project_new") == 0) return CMD_PROJECT_NEW;

    /* Phase 13 Claw Ecosystem Actions */
    if (strcmp(name, "marketplace") == 0) return CMD_MARKETPLACE;
    if (strcmp(name, "package_manager") == 0) return CMD_PACKAGE_MANAGER;

    return CMD_NONE;
}

int keymapParseFile(
    const char *path,
    void (*setter)(
        int action,
        int key,
        int modifiers
    )
)
{
    FILE *fp = fopen(path, "r");

    if (!fp)
        return 0;

    char line[256];

    while (fgets(line, sizeof(line), fp))
    {
        line[
            strcspn(
                line,
                "\r\n"
            )
        ] = '\0';

        char *trimmed =
            trimWhitespace(line);

        if (
            *trimmed == '\0' ||
            *trimmed == '#'
        )
        {
            continue;
        }

        char *equals =
            strchr(trimmed, '=');

        if (!equals)
            continue;

        *equals = '\0';

        char *actionStr =
            trimWhitespace(trimmed);

        char *keyStr =
            trimWhitespace(
                equals + 1
            );

        int action =
            parseActionName(
                actionStr
            );

        if (
            action ==
            CMD_NONE
        )
        {
            continue;
        }

        int modifiers = 0;

        if (
            strstr(
                keyStr,
                "Ctrl"
            )
        )
        {
            modifiers |= MOD_CTRL;
        }

        if (
            strstr(
                keyStr,
                "Alt"
            )
        )
        {
            modifiers |= MOD_ALT;
        }

        if (
            strstr(
                keyStr,
                "Shift"
            )
        )
        {
            modifiers |= MOD_SHIFT;
        }

        int key =
            keymapParseKey(
                keyStr
            );

        if (key < 0)
            continue;

        setter(
            action,
            key,
            modifiers
        );
    }

    fclose(fp);

    return 1;
}