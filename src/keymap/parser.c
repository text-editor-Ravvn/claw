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