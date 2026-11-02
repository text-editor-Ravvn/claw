#include <ctype.h>
#include <string.h>
#include <strings.h>
#include <stdlib.h>

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

#ifdef _WIN32
ssize_t getline(char **lineptr, size_t *n, FILE *stream)
{
    if (!lineptr || !n || !stream)
        return -1;

    if (*lineptr == NULL || *n == 0)
    {
        *n = 128;
        *lineptr = malloc(*n);
        if (*lineptr == NULL)
            return -1;
    }

    size_t pos = 0;
    int c;
    while ((c = fgetc(stream)) != EOF)
    {
        if (pos + 2 > *n)
        {
            size_t new_size = *n * 2;
            char *new_ptr = realloc(*lineptr, new_size);
            if (!new_ptr)
                return -1;
            *lineptr = new_ptr;
            *n = new_size;
        }
        (*lineptr)[pos++] = (char)c;
        if (c == '\n')
            break;
    }

    if (pos == 0 && c == EOF)
        return -1;

    (*lineptr)[pos] = '\0';
    return (ssize_t)pos;
}
#endif

