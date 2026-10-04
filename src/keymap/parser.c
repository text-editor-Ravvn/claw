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

    if (strcmp(name, "join_lines") == 0)
    return CMD_JOIN_LINES;

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