#ifndef UTILS_H
#define UTILS_H

#include <stdio.h>
#include <sys/types.h>

/* Remove leading and trailing whitespace from a string in place.
   Returns the pointer to the first non-whitespace character. */
char *trimWhitespace(char *str);

/* Parse a boolean string value.
   Returns 1 for "true", "1", "yes" (case-insensitive), 0 otherwise. */
int parseBool(const char *str);

#ifdef _WIN32
/* POSIX getline implementation for Windows/MinGW */
ssize_t getline(char **lineptr, size_t *n, FILE *stream);
#endif

#endif

