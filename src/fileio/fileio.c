#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "buffer.h"

extern Buffer buffer;

void openFile(const char *filename)
{
    FILE *fp = fopen(filename, "r");

    if (!fp)
        return;

    char line[1024];

    buffer.numRows = 0;
    free(buffer.rows);
    buffer.rows = NULL;

    while (fgets(line, sizeof(line), fp))
    {
        size_t len = strlen(line);

        if (len > 0 && line[len - 1] == '\n')
        {
            line[len - 1] = '\0';
            len--;
        }

        buffer.rows = realloc(
            buffer.rows,
            sizeof(Row) * (buffer.numRows + 1)
        );

        buffer.rows[buffer.numRows].size = (int)len;

        buffer.rows[buffer.numRows].chars =
            malloc(len + 1);

        strcpy(
            buffer.rows[buffer.numRows].chars,
            line
        );

        buffer.numRows++;
    }

    fclose(fp);

    if (buffer.numRows == 0)
    {
        buffer.numRows = 1;

        buffer.rows = malloc(sizeof(Row));

        buffer.rows[0].size = 0;

        buffer.rows[0].chars = malloc(1);

        buffer.rows[0].chars[0] = '\0';
    }
}
void saveFile(const char *filename)
{
    FILE *fp = fopen(filename, "w");

    if (!fp)
        return;

    for (int i = 0; i < buffer.numRows; i++)
    {
        fwrite(
            buffer.rows[i].chars,
            1,
            buffer.rows[i].size,
            fp
        );

        fputc('\n', fp);
    }

    fclose(fp);
}