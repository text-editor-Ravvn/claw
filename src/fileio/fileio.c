#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>

#include "buffer.h"
#include "cursor.h"

extern Buffer buffer;

static void freeRows(Buffer *target)
{
    for (int i = 0; i < target->numRows; i++)
        free(target->rows[i].chars);

    free(target->rows);
}

static int appendRow(Buffer *target, const char *chars, size_t size)
{
    if (size > INT_MAX)
        return 0;

    Row *rows = realloc(target->rows, sizeof(Row) * (target->numRows + 1));

    if (!rows)
        return 0;

    target->rows = rows;

    char *rowChars = malloc(size + 1);

    if (!rowChars)
        return 0;

    memcpy(rowChars, chars, size);
    rowChars[size] = '\0';
    target->rows[target->numRows].size = (int)size;
    target->rows[target->numRows].chars = rowChars;
    target->numRows++;
    return 1;
}

int openFile(const char *filename)
{
    FILE *fp = fopen(filename, "rb");
    Buffer loaded = {0, NULL, 0};
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

    if (size > 0 || loaded.numRows == 0)
    {
        if (!appendRow(&loaded, line, size))
            goto failure;
    }

    loaded.endsWithNewline = lastWasNewline;
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
    FILE *fp = fopen(filename, "wb");

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
            return 0;
        }

        if (i < buffer.numRows - 1 || buffer.endsWithNewline)
        {
            if (fputc('\n', fp) == EOF)
            {
                fclose(fp);
                return 0;
            }
        }
    }

    return fclose(fp) == 0;
}