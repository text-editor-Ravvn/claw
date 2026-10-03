#include "brackets.h"
#include "buffer.h"
#include "cursor.h"

extern Buffer buffer;

int isBracket(char c)
{
    return (
        c == '(' ||
        c == ')' ||
        c == '[' ||
        c == ']' ||
        c == '{' ||
        c == '}'
    );
}

char getMatchingBracket(char c)
{
    switch (c)
    {
        case '(':
            return ')';

        case ')':
            return '(';

        case '[':
            return ']';

        case ']':
            return '[';

        case '{':
            return '}';

        case '}':
            return '{';

        default:
            return '\0';
    }
}

void updateBracketMatch(void)
{
    bracketMatchX = -1;
    bracketMatchY = -1;

    if (
        cursor.y < 0 ||
        cursor.y >= buffer.numRows
    )
        return;

    Row *row = &buffer.rows[cursor.y];

    if (
        cursor.x < 0 ||
        cursor.x >= row->size
    )
        return;

    char current =
        row->chars[cursor.x];

    if (!isBracket(current))
        return;

    char target =
        getMatchingBracket(current);

    int direction =
        (
            current == '(' ||
            current == '[' ||
            current == '{'
        )
        ? 1
        : -1;

    int depth = 0;

    for (
        int y = cursor.y;
        y >= 0 &&
        y < buffer.numRows;
        y += direction
    )
    {
        Row *r = &buffer.rows[y];

        int start =
            (y == cursor.y)
            ? cursor.x
            : (
                direction > 0
                ? 0
                : r->size - 1
            );

        for (
            int x = start;
            x >= 0 &&
            x < r->size;
            x += direction
        )
        {
            char c = r->chars[x];

            if (c == current)
                depth++;

            if (c == target)
            {
                depth--;

                if (depth == 0)
                {
                    bracketMatchY = y;
                    bracketMatchX = x;
                    return;
                }
            }
        }
    }
}