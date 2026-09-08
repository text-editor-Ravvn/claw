#include <stdio.h>
#include <string.h>
#include <strings.h>
#include <ctype.h>

#include "keymap.h"
#include "input.h"
#include "utils.h"

int keymapParseKey(const char *str)
{
    /* Handle "Ctrl+X" format. */
    if (strncasecmp(str, "Ctrl+", 5) == 0 && str[5] != '\0')
    {
        char ch = str[5];

        if (isalpha((unsigned char)ch))
            return CTRL_KEY(tolower((unsigned char)ch));
    }

    /* Handle single printable character (for future extensibility). */
    if (strlen(str) == 1 && str[0] >= 32 && str[0] <= 126)
        return str[0];

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

    if (strcmp(name, "undo") == 0)
        return CMD_UNDO;

    if (strcmp(name, "redo") == 0)
        return CMD_REDO;

    if (strcmp(name, "next_match") == 0)
        return CMD_NEXT_MATCH;

    if (strcmp(name, "prev_match") == 0)
        return CMD_PREV_MATCH;

    return CMD_NONE;
}

int keymapParseFile(const char *path,
                    void (*setter)(int, int))
{
    FILE *fp = fopen(path, "r");

    if (!fp)
        return 0;

    char line[256];

    while (fgets(line, sizeof(line), fp))
    {
        line[strcspn(line, "\r\n")] = '\0';

        char *trimmed = trimWhitespace(line);

        if (*trimmed == '\0' || *trimmed == '#')
            continue;

        char *equals = strchr(trimmed, '=');

        if (!equals)
            continue;

        *equals = '\0';

        char *actionStr = trimWhitespace(trimmed);
        char *keyStr = trimWhitespace(equals + 1);

        int action = parseActionName(actionStr);

        if (action == CMD_NONE)
            continue;

        int key = keymapParseKey(keyStr);

        if (key < 0)
            continue;

        setter(action, key);
    }

    fclose(fp);
    return 1;
}
