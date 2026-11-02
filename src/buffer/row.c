#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "buffer.h"
#include "cursor.h"
#include "history.h"
#include "config.h"
#include "search.h"

extern Buffer buffer;

/* Language-aware smart editing hooks (Phase 10) */
static const char *defaultCommentPrefix(void) { return "//"; }
static const char *defaultCommentSuffix(void) { return NULL; }
static char defaultIndentTrigger(void) { return '{'; }

const char *(*clawCommentPrefixHook)(void) = defaultCommentPrefix;
const char *(*clawCommentSuffixHook)(void) = defaultCommentSuffix;
char (*clawIndentTriggerHook)(void) = defaultIndentTrigger;

static char getLastNonWhitespaceChar(const char *chars, int len, const char *singleLineComment)
{
    if (!chars || len <= 0)
        return '\0';

    if (singleLineComment && *singleLineComment)
    {
        /* Trim trailing spaces from comment token, e.g. "#" from "# " */
        char cTok[16];
        size_t cLen = 0;
        while (singleLineComment[cLen] && !isspace((unsigned char)singleLineComment[cLen]) &&
               cLen < sizeof(cTok) - 1)
        {
            cTok[cLen] = singleLineComment[cLen];
            cLen++;
        }
        cTok[cLen] = '\0';

        if (cLen > 0)
        {
            int inQuote = 0;
            for (int i = 0; i + (int)cLen <= len; i++)
            {
                if ((chars[i] == '"' || chars[i] == '\'') && (i == 0 || chars[i - 1] != '\\'))
                {
                    if (!inQuote)
                        inQuote = chars[i];
                    else if (inQuote == chars[i])
                        inQuote = 0;
                }
                if (!inQuote && memcmp(&chars[i], cTok, cLen) == 0)
                {
                    len = i;
                    break;
                }
            }
        }
    }

    while (len > 0 && isspace((unsigned char)chars[len - 1]))
        len--;
    return (len > 0) ? chars[len - 1] : '\0';
}

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

    /* Smart closing brace alignment */
    if (c == '}')
    {
        int onlySpaces = 1;

        for (int i = 0; i < cursor.x; i++)
        {
            if (row->chars[i] != ' ')
            {
                onlySpaces = 0;
                break;
            }
        }

        if (onlySpaces)
        {
            int currentIndent = cursor.x;

            if (currentIndent >= 4)
            {
                memmove(
                    &row->chars[currentIndent - 4],
                    &row->chars[currentIndent],
                    (size_t)(row->size - currentIndent + 1)
                );

                row->size -= 4;
                cursor.x -= 4;
            }
        }
    }

    /* Skip over existing closing delimiter */
    if (
        isClosingDelimiter(c) &&
        cursor.x < row->size &&
        row->chars[cursor.x] == c
    )
    {
        cursor.x++;
        return 1;
    }

    /* Skip over existing quote */
    if (
        (c == '"' || c == '\'') &&
        cursor.x < row->size &&
        row->chars[cursor.x] == c
    )
    {
        cursor.x++;
        return 1;
    }

    /* Auto-pair opening delimiters */
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
    refreshSearchMatches();
}

static int handleSmartBackspace(void)
{
    if (cursor.y < 0 || cursor.y >= buffer.numRows)
        return 0;

    Row *row = &buffer.rows[cursor.y];

    if (cursor.x == 0)
        return 0;

    /* Only trigger when cursor is inside indentation */
    for (int i = 0; i < cursor.x; i++)
    {
        if (row->chars[i] != ' ')
            return 0;
    }

    int tabWidth = configTabWidth();
    if (tabWidth <= 0)
        tabWidth = 4;

    if (cursor.x >= tabWidth)
    {
        int removeCount = tabWidth;

        memmove(
            &row->chars[cursor.x - removeCount],
            &row->chars[cursor.x],
            (size_t)(row->size - cursor.x + 1)
        );

        row->size -= removeCount;
        cursor.x -= removeCount;

        free(row->hl);
        row->hl = NULL;
        row->hlSize = 0;

        buffer.modified = 1;
        refreshSearchMatches();

        return 1;
    }

    return 0;
}

void deleteChar(void)
{
     if (handleSmartBackspace())
        return;

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
        refreshSearchMatches();

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
    refreshSearchMatches();
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
        refreshSearchMatches();
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
        refreshSearchMatches();
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
    buffer.rows[cursor.y + 1].hlOpenState = 0;

    buffer.numRows++;

    cursor.y++;

    /* Auto indentation */
    int indent = getIndentation(buffer.rows[cursor.y - 1].chars);

    /* Smart indentation based on language trigger */
    char trig = clawIndentTriggerHook ? clawIndentTriggerHook() : 0;
    const char *cmtPrefix = clawCommentPrefixHook ? clawCommentPrefixHook() : NULL;
    char lastNonSpace = getLastNonWhitespaceChar(buffer.rows[cursor.y - 1].chars, leftSize, cmtPrefix);

    if (trig == ':')
    {
        /* Python / YAML: indent after colon or opening bracket */
        if (lastNonSpace == ':' || lastNonSpace == '{' || lastNonSpace == '(' || lastNonSpace == '[')
        {
            int tw = configTabWidth();
            indent += (tw > 0) ? tw : 4;
        }
    }
    else if (trig == '{')
    {
        /* C / JS / Rust / Go / Java: indent after opening brace or bracket */
        if (lastNonSpace == '{' || lastNonSpace == '(' || lastNonSpace == '[')
        {
            int tw = configTabWidth();
            indent += (tw > 0) ? tw : 4;
        }
    }

if (indent > 0)
{
    Row *row = &buffer.rows[cursor.y];

    char *newChars =
        realloc(
            row->chars,
            (size_t)row->size + indent + 1
        );

    if (newChars)
    {
        row->chars = newChars;

        memmove(
            &row->chars[indent],
            row->chars,
            (size_t)row->size + 1
        );

        memset(
            row->chars,
            ' ',
            (size_t)indent
        );

        row->size += indent;
    }
}

cursor.x = indent;
buffer.modified = 1;
}

void duplicateLine(void)
{
    if (cursor.y < 0 || cursor.y >= buffer.numRows)
        return;

    int lineIndex = cursor.y;

    Row source = buffer.rows[lineIndex];

    Row *newRows =
        realloc(
            buffer.rows,
            sizeof(Row) * (buffer.numRows + 1)
        );

    if (!newRows)
        return;

    buffer.rows = newRows;

    for (int i = buffer.numRows; i > lineIndex + 1; i--)
    {
        buffer.rows[i] =
            buffer.rows[i - 1];
    }

    Row *copy =
        &buffer.rows[lineIndex + 1];

    copy->size = source.size;

    copy->chars =
        malloc((size_t)source.size + 1);

    if (!copy->chars)
        return;

    memcpy(
        copy->chars,
        source.chars,
        (size_t)source.size + 1
    );

    copy->hl = NULL;
    copy->hlSize = 0;
    copy->hlOpenState = 0;

    buffer.numRows++;

    cursor.y++;

    buffer.modified = 1;
}
void moveLineUp(void)
{
    if (cursor.y <= 0)
        return;

    Row temp =
        buffer.rows[cursor.y - 1];

    buffer.rows[cursor.y - 1] =
        buffer.rows[cursor.y];

    buffer.rows[cursor.y] =
        temp;

    free(buffer.rows[cursor.y - 1].hl);
    buffer.rows[cursor.y - 1].hl = NULL;
    buffer.rows[cursor.y - 1].hlSize = 0;

    free(buffer.rows[cursor.y].hl);
    buffer.rows[cursor.y].hl = NULL;
    buffer.rows[cursor.y].hlSize = 0;

    cursor.y--;

    buffer.modified = 1;
}

void moveLineDown(void)
{
    if (
        cursor.y < 0 ||
        cursor.y >= buffer.numRows - 1
    )
        return;

    Row temp =
        buffer.rows[cursor.y + 1];

    buffer.rows[cursor.y + 1] =
        buffer.rows[cursor.y];

    buffer.rows[cursor.y] =
        temp;

    free(buffer.rows[cursor.y + 1].hl);
    buffer.rows[cursor.y + 1].hl = NULL;
    buffer.rows[cursor.y + 1].hlSize = 0;

    free(buffer.rows[cursor.y].hl);
    buffer.rows[cursor.y].hl = NULL;
    buffer.rows[cursor.y].hlSize = 0;

    cursor.y++;

    buffer.modified = 1;
}
void toggleCommentLine(void)
{
    if (cursor.y < 0 || cursor.y >= buffer.numRows)
        return;

    Row *row = &buffer.rows[cursor.y];
    int indent = 0;

    while (indent < row->size && (row->chars[indent] == ' ' || row->chars[indent] == '\t'))
        indent++;

    const char *pfx = clawCommentPrefixHook ? clawCommentPrefixHook() : "//";
    const char *sfx = clawCommentSuffixHook ? clawCommentSuffixHook() : NULL;
    if (!pfx || !*pfx)
        pfx = "//";

    /* Extract base token without trailing spaces, e.g. "//" from "// " or "#" from "# " */
    char token[16];
    size_t tokenLen = 0;
    while (pfx[tokenLen] && !isspace((unsigned char)pfx[tokenLen]) && tokenLen < sizeof(token) - 1)
    {
        token[tokenLen] = pfx[tokenLen];
        tokenLen++;
    }
    token[tokenLen] = '\0';

    if (sfx && *sfx)
    {
        /* Block comment toggle, e.g. HTML / Markdown <!-- ... --> */
        char sfxToken[16];
        size_t sfxTokenLen = 0;
        const char *sp = sfx;
        while (*sp && isspace((unsigned char)*sp))
            sp++;
        while (*sp && !isspace((unsigned char)*sp) && sfxTokenLen < sizeof(sfxToken) - 1)
        {
            sfxToken[sfxTokenLen++] = *sp++;
        }
        sfxToken[sfxTokenLen] = '\0';

        if (indent + (int)tokenLen <= row->size &&
            memcmp(&row->chars[indent], token, tokenLen) == 0)
        {
            /* Uncomment block comment */
            int removeStart = (int)tokenLen;
            if (indent + removeStart < row->size && row->chars[indent + removeStart] == ' ')
                removeStart++;

            memmove(&row->chars[indent], &row->chars[indent + removeStart],
                    (size_t)(row->size - indent - removeStart + 1));
            row->size -= removeStart;

            /* Strip suffix at end if present */
            int trailing = row->size;
            while (trailing > indent && isspace((unsigned char)row->chars[trailing - 1]))
                trailing--;
            if (trailing - indent >= (int)sfxTokenLen &&
                memcmp(&row->chars[trailing - sfxTokenLen], sfxToken, sfxTokenLen) == 0)
            {
                int endStart = trailing - (int)sfxTokenLen;
                if (endStart > indent && row->chars[endStart - 1] == ' ')
                    endStart--;
                row->chars[endStart] = '\0';
                row->size = endStart;
            }
        }
        else
        {
            /* Comment with block comment */
            char addPrefix[32];
            char addSuffix[32];
            snprintf(addPrefix, sizeof(addPrefix), "%s ", token);
            snprintf(addSuffix, sizeof(addSuffix), " %s", sfxToken);
            size_t addPreLen = strlen(addPrefix);
            size_t addSufLen = strlen(addSuffix);

            char *newChars = realloc(row->chars, (size_t)row->size + addPreLen + addSufLen + 1);
            if (!newChars)
                return;
            row->chars = newChars;

            memmove(&row->chars[indent + addPreLen], &row->chars[indent],
                    (size_t)(row->size - indent + 1));
            memcpy(&row->chars[indent], addPrefix, addPreLen);
            row->size += (int)addPreLen;

            memcpy(&row->chars[row->size], addSuffix, addSufLen);
            row->size += (int)addSufLen;
            row->chars[row->size] = '\0';
        }
    }
    else
    {
        /* Single-line comment toggle, e.g. // or # */
        if (indent + (int)tokenLen <= row->size &&
            memcmp(&row->chars[indent], token, tokenLen) == 0)
        {
            /* Uncomment */
            int removeCount = (int)tokenLen;
            if (indent + removeCount < row->size && row->chars[indent + removeCount] == ' ')
                removeCount++;

            memmove(&row->chars[indent], &row->chars[indent + removeCount],
                    (size_t)(row->size - indent - removeCount + 1));
            row->size -= removeCount;
            row->chars[row->size] = '\0';
        }
        else
        {
            /* Comment */
            char addPrefix[32];
            snprintf(addPrefix, sizeof(addPrefix), "%s ", token);
            size_t addLen = strlen(addPrefix);

            char *newChars = realloc(row->chars, (size_t)row->size + addLen + 1);
            if (!newChars)
                return;
            row->chars = newChars;

            memmove(&row->chars[indent + addLen], &row->chars[indent],
                    (size_t)(row->size - indent + 1));
            memcpy(&row->chars[indent], addPrefix, addLen);
            row->size += (int)addLen;
        }
    }

    free(row->hl);
    row->hl = NULL;
    row->hlSize = 0;
    row->hlOpenState = 0;

    buffer.modified = 1;
}
void joinLineWithNext(void)
{
    if (cursor.y >= buffer.numRows - 1)
        return;

    Row *current =
        &buffer.rows[cursor.y];

    Row *next =
        &buffer.rows[cursor.y + 1];

    int newSize =
        current->size +
        next->size;

    char *newChars =
        realloc(
            current->chars,
            newSize + 1
        );

    if (!newChars)
        return;

    current->chars = newChars;

    memcpy(
        current->chars +
        current->size,
        next->chars,
        next->size
    );

    current->size = newSize;

    current->chars[newSize] = '\0';

    free(next->chars);
    free(next->hl);

    free(current->hl);
    current->hl = NULL;
    current->hlSize = 0;

    memmove(
        &buffer.rows[cursor.y + 1],
        &buffer.rows[cursor.y + 2],
        sizeof(Row) *
        (
            buffer.numRows -
            cursor.y -
            2
        )
    );

    buffer.numRows--;

    buffer.modified = 1;
}

void splitLineAtCursor(void)
{
    insertNewLine();
}