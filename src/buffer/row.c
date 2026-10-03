#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "buffer.h"
#include "cursor.h"
#include "history.h"

extern Buffer buffer;

/* ===== Phase 4: Smart Delimiter Support ===== */

static int getIndentation(const char *line)
{
    int count = 0;

    while (line[count] == ' ' || line[count] == '\t')
        count++;

    return count;
}

int isOpeningDelimiter(char c)
{
    return (
        c == '(' ||
        c == '[' ||
        c == '{' ||
        c == '"' ||
        c == '\''
    );
}

int isClosingDelimiter(char c)
{
    return (
        c == ')' ||
        c == ']' ||
        c == '}'
    );
}

char matchingDelimiter(char c)
{
    switch (c)
    {
        case '(': return ')';
        case '[': return ']';
        case '{': return '}';
        case '"': return '"';
        case '\'': return '\'';
        default: return '\0';
    }
}

void insertPair(char opening, char closing)
{
    if (cursor.y < 0 || cursor.y >= buffer.numRows)
        return;

    Row *row = &buffer.rows[cursor.y];

    if (cursor.x < 0)
        cursor.x = 0;

    if (cursor.x > row->size)
        cursor.x = row->size;

    int originalPos = cursor.x;

    insertChar(opening);
    insertChar(closing);

    cursor.x = originalPos + 1;
}

int handleSmartCharacter(char c)
{
    if (cursor.y < 0 || cursor.y >= buffer.numRows)
        return 0;

    Row *row = &buffer.rows[cursor.y];

    if (
        isClosingDelimiter(c) &&
        cursor.x < row->size &&
        row->chars[cursor.x] == c
    )
    {
        cursor.x++;
        return 1;
    }

    if (
        (c == '"' || c == '\'') &&
        cursor.x < row->size &&
        row->chars[cursor.x] == c
    )
    {
        cursor.x++;
        return 1;
    }

    if (isOpeningDelimiter(c))
    {
        char closing = matchingDelimiter(c);

        if (closing != '\0')
        {
            insertPair(c, closing);
            return 1;
        }
    }

    return 0;
}

void insertChar(char c)
{
    if (cursor.y >= buffer.numRows)
        return;

    historyPushInsert(
        cursor.y,
        cursor.x,
        c
    );

    /* Grow the row, shift its suffix right, then place the new byte. */

    Row *row = &buffer.rows[cursor.y];

    if (cursor.x > row->size)
        cursor.x = row->size;

    char *newChars = realloc(row->chars, (size_t)row->size + 2);

    if (!newChars)
        return;

    row->chars = newChars;

    memmove(
        &row->chars[cursor.x + 1],
        &row->chars[cursor.x],
        row->size - cursor.x + 1
    );

    row->chars[cursor.x] = c;

    row->size++;

    row->chars[row->size] = '\0';

    /* Invalidate syntax highlight cache. */
    free(row->hl);
    row->hl = NULL;
    row->hlSize = 0;

    cursor.x++;
    buffer.modified = 1;
}

void deleteChar(void)
{
    /* Backspace removes left; at column zero it joins with the prior row. */
    if (cursor.y >= buffer.numRows)
        return;

    Row *row = &buffer.rows[cursor.y];

    /* Normal backspace inside a line */
    if (cursor.x > 0)
    {
        historyPushDelete(
    cursor.y,
    cursor.x - 1,
    row->chars[cursor.x - 1]
    );
        memmove(
            &row->chars[cursor.x - 1],
            &row->chars[cursor.x],
            row->size - cursor.x + 1
        );

        row->size--;

        row->chars[row->size] = '\0';

        /* Invalidate syntax highlight cache. */
        free(row->hl);
        row->hl = NULL;
        row->hlSize = 0;

        cursor.x--;
        buffer.modified = 1;

        return;
    }

    /* Beginning of first line */
    if (cursor.y == 0)
        return;

    /* Merge with previous line */
    Row *current = &buffer.rows[cursor.y];
    Row *previous = &buffer.rows[cursor.y - 1];

    int oldSize = previous->size;

    char *newChars = realloc(
        previous->chars,
        (size_t)previous->size + current->size + 1
    );

    if (!newChars)
        return;

    previous->chars = newChars;

    memcpy(
        &previous->chars[previous->size],
        current->chars,
        current->size
    );

    previous->size += current->size;

    previous->chars[previous->size] = '\0';

    /* Invalidate the merged row's highlight cache. */
    free(previous->hl);
    previous->hl = NULL;
    previous->hlSize = 0;

    free(current->chars);
    free(current->hl);

    for (int i = cursor.y; i < buffer.numRows - 1; i++)
    {
        buffer.rows[i] = buffer.rows[i + 1];
    }

    buffer.numRows--;

    Row *newRows = realloc(buffer.rows, sizeof(Row) * buffer.numRows);

    if (newRows)
        buffer.rows = newRows;

    cursor.y--;

    cursor.x = oldSize;
    buffer.modified = 1;
}

void deleteForward(void)
{
    /* Delete right; at row end it joins the following row. */
    if (cursor.y < 0 || cursor.y >= buffer.numRows || !buffer.rows)
        return;

    Row *row = &buffer.rows[cursor.y];

    if (cursor.x < row->size)
    {
        historyPushDelete(
    cursor.y,
    cursor.x,
    row->chars[cursor.x]
    );
        memmove(
            &row->chars[cursor.x],
            &row->chars[cursor.x + 1],
            (size_t)(row->size - cursor.x)
        );
        row->size--;

        /* Invalidate syntax highlight cache. */
        free(row->hl);
        row->hl = NULL;
        row->hlSize = 0;
        buffer.modified = 1;
        return;
    }

    if (cursor.y < buffer.numRows - 1)
    {
        Row *next = &buffer.rows[cursor.y + 1];
        int oldSize = row->size;
        char *newChars = realloc(
            row->chars,
            (size_t)row->size + next->size + 1
        );

        if (!newChars)
            return;

        row->chars = newChars;
        memcpy(&row->chars[row->size], next->chars, (size_t)next->size);
        row->size += next->size;
        row->chars[row->size] = '\0';

        /* Invalidate the merged row's highlight cache. */
        free(row->hl);
        row->hl = NULL;
        row->hlSize = 0;

        free(next->chars);
        free(next->hl);

        for (int i = cursor.y + 1; i < buffer.numRows - 1; i++)
            buffer.rows[i] = buffer.rows[i + 1];

        buffer.numRows--;
        Row *newRows = realloc(buffer.rows, sizeof(Row) * buffer.numRows);
        if (newRows)
            buffer.rows = newRows;
        cursor.x = oldSize;
        buffer.modified = 1;
    }
}

void insertNewLine(void)
{
    historyPushNewLine(
    cursor.y,
    cursor.x
);
    /* Move the text after the cursor into a newly inserted row. */
    if (cursor.y >= buffer.numRows)
        return;

    Row *current = &buffer.rows[cursor.y];

    if (cursor.x < 0)
        cursor.x = 0;
    if (cursor.x > current->size)
        cursor.x = current->size;

    int leftSize = cursor.x;
    int rightSize = current->size - cursor.x;

    char *rightPart = malloc((size_t)rightSize + 1);

    if (!rightPart)
        return;

    memcpy(
        rightPart,
        &current->chars[cursor.x],
        rightSize
    );

    rightPart[rightSize] = '\0';

    current->size = leftSize;

    current->chars[leftSize] = '\0';

    /* Invalidate the truncated row's highlight cache. */
    free(current->hl);
    current->hl = NULL;
    current->hlSize = 0;

    Row *newRows = realloc(
        buffer.rows,
        sizeof(Row) * (buffer.numRows + 1)
    );

    if (!newRows)
    {
        free(rightPart);
        return;
    }

    buffer.rows = newRows;

    for (int i = buffer.numRows; i > cursor.y + 1; i--)
    {
        buffer.rows[i] = buffer.rows[i - 1];
    }

    buffer.rows[cursor.y + 1].size = rightSize;
    buffer.rows[cursor.y + 1].chars = rightPart;
    buffer.rows[cursor.y + 1].hl = NULL;
    buffer.rows[cursor.y + 1].hlSize = 0;

    buffer.numRows++;

    cursor.y++;

/* Auto indentation */
int indent =
    getIndentation(buffer.rows[cursor.y - 1].chars);

if (indent > 0)
{
    Row *row = &buffer.rows[cursor.y];

    char *newChars =
        realloc(
            row->chars,
            row->size + indent + 1
        );

    if (newChars)
    {
        row->chars = newChars;

        memmove(
            row->chars + indent,
            row->chars,
            row->size + 1
        );

        for (int i = 0; i < indent; i++)
            row->chars[i] = ' ';

        row->size += indent;
    }
}

cursor.x = indent;
buffer.modified = 1;
}