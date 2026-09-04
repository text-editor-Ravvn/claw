#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>

#include "buffer.h"
#include "cursor.h"

extern Buffer buffer;

static void freeRows(Buffer *target)
{
    /* Used for transactional loads so a failed load cannot leak partial rows. */
    for (int i = 0; i < target->numRows; i++)
        free(target->rows[i].chars);

    free(target->rows);
}

static int appendRow(Buffer *target, const char *chars, size_t size)
{
    /* Append a NUL-terminated editor row without using strlen on file bytes. */
    if (size > INT_MAX)
        return 0;

    Row *rows = realloc(target->rows, sizeof(Row) * (target->numRows + 1));

    if (!rows)
        return 0;

    target->rows = rows;

    char *rowChars = malloc(size + 1);

    if (!rowChars)
        return 0;

    if (size > 0)
        memcpy(rowChars, chars, size);
    rowChars[size] = '\0';
    target->rows[target->numRows].size = (int)size;
    target->rows[target->numRows].chars = rowChars;
    target->numRows++;
    return 1;
}

int openFile(const char *filename)
{
    /* Load into a temporary buffer and replace the active document only at EOF. */
    FILE *fp = fopen(filename, "rb");
    Buffer loaded = {0, NULL, 0, 0};
    char *line = NULL;
    size_t size = 0;
    size_t capacity = 0;
    int lastWasNewline = 0;
    int value;

    if (!fp)
        return 0;

    while ((value = fgetc(fp)) != EOF)
    {
        if (value == '\n')
        {
            if (!appendRow(&loaded, line, size))
                goto failure;
            size = 0;
            lastWasNewline = 1;
            continue;
        }

        if (size == capacity)
        {
            size_t newCapacity = capacity ? capacity * 2 : 128;
            char *newLine = realloc(line, newCapacity);

            if (!newLine)
                goto failure;

            line = newLine;
            capacity = newCapacity;
        }

        line[size++] = (char)value;
        lastWasNewline = 0;
    }

    if (ferror(fp))
        goto failure;

    if (size > 0 || loaded.numRows == 0)
    {
        if (!appendRow(&loaded, line, size))
            goto failure;
    }

    loaded.endsWithNewline = lastWasNewline;
    loaded.modified = 0;
    free(line);
    fclose(fp);
    bufferFree();
    buffer = loaded;
    cursor.x = 0;
    cursor.y = 0;
    return 1;

failure:
    free(line);
    freeRows(&loaded);
    fclose(fp);
    return 0;
}

int saveFile(const char *filename)
{
    /* Binary output preserves embedded NUL bytes and the final newline choice. */
    char temporaryName[4096];
    int nameLength = snprintf(
        temporaryName,
        sizeof(temporaryName),
        "%s.claw.tmp",
        filename
    );

    if (nameLength < 0 || (size_t)nameLength >= sizeof(temporaryName))
        return 0;

    FILE *fp = fopen(temporaryName, "wb");

    if (!fp)
        return 0;

    for (int i = 0; i < buffer.numRows; i++)
    {
        if (fwrite(
            buffer.rows[i].chars,
            1,
            buffer.rows[i].size,
            fp
        ) != (size_t)buffer.rows[i].size)
        {
            fclose(fp);
            remove(temporaryName);
            return 0;
        }

        if (i < buffer.numRows - 1 || buffer.endsWithNewline)
        {
            if (fputc('\n', fp) == EOF)
            {
                fclose(fp);
                remove(temporaryName);
                return 0;
            }
        }
    }

    if (fclose(fp) != 0)
    {
        remove(temporaryName);
        return 0;
    }

    if (rename(temporaryName, filename) != 0)
    {
#ifdef _WIN32
        /* Windows cannot replace an existing file with rename(). */
        if (remove(filename) == 0 && rename(temporaryName, filename) == 0)
            goto saved;
#endif
        remove(temporaryName);
        return 0;
    }

saved:
    buffer.modified = 0;
    return 1;
}