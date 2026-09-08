#include <ctype.h>
#include <string.h>
#include <strings.h>

#include "utils.h"

char *trimWhitespace(char *str)
{
    /* Skip leading whitespace. */
    while (*str && isspace((unsigned char)*str))
        str++;

    if (*str == '\0')
        return str;

    /* Trim trailing whitespace by walking backward from the end. */
    char *end = str + strlen(str) - 1;

    while (end > str && isspace((unsigned char)*end))
        end--;

    end[1] = '\0';

    return str;
}

int parseBool(const char *str)
{
    if (strcasecmp(str, "true") == 0)
        return 1;

    if (strcasecmp(str, "yes") == 0)
        return 1;

    if (strcmp(str, "1") == 0)
        return 1;

    return 0;
}
