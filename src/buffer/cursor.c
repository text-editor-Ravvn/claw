#include "cursor.h"
#include "buffer.h"

Cursor cursor = {0, 0};
int bracketMatchX = -1;
int bracketMatchY = -1;

extern Buffer buffer;

void moveCursorLeft(void)
{
    /* Horizontal movement is limited to the beginning of the current row. */
    if (cursor.x > 0)
        cursor.x--;
}

void moveCursorRight(void)
{
    /* The cursor may sit immediately after the final character. */
    if (cursor.y >= buffer.numRows)
        return;

    Row *row = &buffer.rows[cursor.y];

    if (cursor.x < row->size)
        cursor.x++;
}

void moveCursorUp(void)
{
    /* Preserve the column where possible, clamping on shorter rows. */
    if (buffer.numRows == 0 || !buffer.rows)
        return;

    if (cursor.y > 0)
        cursor.y--;

    Row *row = &buffer.rows[cursor.y];

    if (cursor.x > row->size)
        cursor.x = row->size;
}

void moveCursorDown(void)
{
    /* Preserve the column where possible, clamping on shorter rows. */
    if (buffer.numRows == 0 || !buffer.rows)
        return;

    if (cursor.y < buffer.numRows - 1)
        cursor.y++;

    Row *row = &buffer.rows[cursor.y];

    if (cursor.x > row->size)
        cursor.x = row->size;
}

void moveCursorWordLeft(void)
{
    if (buffer.numRows == 0 || !buffer.rows)
        return;

    if (cursor.x <= 0 && cursor.y <= 0)
        return;

    /* Move one step left first */
    if (cursor.x <= 0)
    {
        if (cursor.y > 0)
        {
            cursor.y--;
            cursor.x = buffer.rows[cursor.y].size;
        }

        return;
    }

    Row *row = &buffer.rows[cursor.y];
    int x = cursor.x;

    /* Skip whitespace going left */
    while (
        x > 0 &&
        (row->chars[x - 1] == ' ' ||
         row->chars[x - 1] == '\t')
    )
    {
        x--;
    }

    if (x > 0)
    {
        char prev = row->chars[x - 1];
        int isDelim = (
            prev == '(' || prev == ')' ||
            prev == '[' || prev == ']' ||
            prev == '{' || prev == '}' ||
            prev == ',' || prev == ';' ||
            prev == '.' || prev == ':' ||
            prev == '"' || prev == '\''
        );

        if (isDelim)
        {
            /* Step over punctuation group */
            while (x > 0)
            {
                char c = row->chars[x - 1];
                int cDelim = (
                    c == '(' || c == ')' ||
                    c == '[' || c == ']' ||
                    c == '{' || c == '}' ||
                    c == ',' || c == ';' ||
                    c == '.' || c == ':' ||
                    c == '"' || c == '\''
                );
                if (!cDelim)
                    break;
                x--;
            }
        }
        else
        {
            /* Skip word characters going left */
            while (x > 0)
            {
                char c = row->chars[x - 1];
                if (
                    c == ' ' || c == '\t' ||
                    c == '(' || c == ')' ||
                    c == '[' || c == ']' ||
                    c == '{' || c == '}' ||
                    c == ',' || c == ';' ||
                    c == '.' || c == ':' ||
                    c == '"' || c == '\''
                )
                {
                    break;
                }
                x--;
            }
        }
    }

    cursor.x = x;
}

void moveCursorWordRight(void)
{
    if (buffer.numRows == 0 || !buffer.rows)
        return;

    Row *row = &buffer.rows[cursor.y];

    if (cursor.x >= row->size)
    {
        if (cursor.y < buffer.numRows - 1)
        {
            cursor.y++;
            cursor.x = 0;
        }

        return;
    }

    int x = cursor.x;

    char curr = row->chars[x];
    int isDelim = (
        curr == '(' || curr == ')' ||
        curr == '[' || curr == ']' ||
        curr == '{' || curr == '}' ||
        curr == ',' || curr == ';' ||
        curr == '.' || curr == ':' ||
        curr == '"' || curr == '\''
    );

    if (isDelim)
    {
        while (x < row->size)
        {
            char c = row->chars[x];
            int cDelim = (
                c == '(' || c == ')' ||
                c == '[' || c == ']' ||
                c == '{' || c == '}' ||
                c == ',' || c == ';' ||
                c == '.' || c == ':' ||
                c == '"' || c == '\''
            );
            if (!cDelim)
                break;
            x++;
        }
    }
    else
    {
        /* Skip current word characters */
        while (x < row->size)
        {
            char c = row->chars[x];

            if (
                c == ' ' || c == '\t' ||
                c == '(' || c == ')' ||
                c == '[' || c == ']' ||
                c == '{' || c == '}' ||
                c == ',' || c == ';' ||
                c == '.' || c == ':' ||
                c == '"' || c == '\''
            )
            {
                break;
            }

            x++;
        }
    }

    /* Skip whitespace */
    while (
        x < row->size &&
        (row->chars[x] == ' ' ||
         row->chars[x] == '\t')
    )
    {
        x++;
    }

    cursor.x = x;
}

void jumpToBracketMatch(void)
{
    if (bracketMatchX >= 0 && bracketMatchY >= 0)
    {
        cursor.y = bracketMatchY;
        cursor.x = bracketMatchX;
    }
}

void moveCursorToStart(void)
{
    cursor.x = 0;
}

void moveCursorToEnd(void)
{
    if (cursor.y >= 0 && cursor.y < buffer.numRows && buffer.rows)
    {
        cursor.x = buffer.rows[cursor.y].size;
    }
    else
    {
        cursor.x = 0;
    }
}