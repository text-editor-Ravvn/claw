#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>

#include "search.h"
#include "editor.h"
#include "buffer.h"
#include "cursor.h"
#include "viewport.h"
#include "plugin_api.h"

extern Buffer buffer;
extern Cursor cursor;

static int isWordCharacter(unsigned char c)
{
    return isalnum(c) || c == '_';
}

static int isWholeWordMatch(
    const char *line,
    const char *match,
    size_t queryLength)
{
    if (!line || !match || queryLength == 0)
        return 0;

    int leftBoundary = 1;
    if (isWordCharacter((unsigned char)match[0]))
    {
        leftBoundary = (match == line) || !isWordCharacter((unsigned char)match[-1]);
    }

    int rightBoundary = 1;
    if (isWordCharacter((unsigned char)match[queryLength - 1]))
    {
        rightBoundary = (match[queryLength] == '\0') || !isWordCharacter((unsigned char)match[queryLength]);
    }

    return leftBoundary && rightBoundary;
}

SearchState searchState = {0};

void clearSearchResults(void)
{
    searchState.active = 0;

    searchState.replaceMode = 0;

    searchState.length = 0;
    searchState.query[0] = '\0';

    searchState.replacementLength = 0;
    searchState.replacement[0] = '\0';

    searchState.matchCount = 0;
    searchState.currentMatch = 0;
}

void openSearchPrompt(void)
{
    pluginTriggerEvent(
        PLUGIN_EVENT_SEARCH
    );

    searchState.active = 1;
    searchState.replaceMode = 0;

    searchState.length = 0;
    searchState.query[0] = '\0';
}

void openReplacePrompt(void)
{
    if (searchState.matchCount == 0)
    {
        editorSetStatusMessage(
            "Search something first"
        );
        return;
    }

    searchState.active = 1;
    searchState.replaceMode = 1;

    /* KEEP search query */
    searchState.replacementLength = 0;
    searchState.replacement[0] = '\0';

    editorSetStatusMessage("");
}

void closeSearchPrompt(void)
{
    searchState.active = 0;
}

void performSearch(void)
{
    searchState.matchCount = 0;
    searchState.currentMatch = 0;

    if (searchState.length == 0)
        return;

    for (int row = 0;
         row < buffer.numRows;
         row++)
    {
        if (!buffer.rows || !buffer.rows[row].chars)
            continue;

        char *start =
            buffer.rows[row].chars;

        while (1)
        {
            char *match =
                strstr(
                    start,
                    searchState.query
                );

            if (!match)
                break;

            
            if (isWholeWordMatch(
                    buffer.rows[row].chars,
                    match,
                    (size_t)searchState.length))
            {
                if (searchState.matchCount < 256)
                {
                    int i = searchState.matchCount++;

                    searchState.matchRows[i] = row;
                    searchState.matchCols[i] =
                        (int)(match - buffer.rows[row].chars);
                }
            }

            start = match + 1;
        }
    }

    if (searchState.matchCount == 0)
    {
        editorSetStatusMessage(
            "No matches found"
        );
        return;
    }

    cursor.y =
        searchState.matchRows[0];

    cursor.x =
        searchState.matchCols[0];

    scrollEditor();

    char status[64];

    snprintf(
        status,
        sizeof(status),
        "%d matches found",
        searchState.matchCount
    );

    editorSetStatusMessage(status);

    pluginTriggerEvent(
        PLUGIN_EVENT_SEARCH
    );
}

void nextMatch(void)
{
    if (searchState.matchCount == 0)
        return;

    searchState.currentMatch++;

    if (searchState.currentMatch >=
        searchState.matchCount)
    {
        searchState.currentMatch = 0;
    }

    cursor.y =
        searchState.matchRows[
            searchState.currentMatch
        ];

    cursor.x =
        searchState.matchCols[
            searchState.currentMatch
        ];

    scrollEditor();

    char status[64];

    snprintf(
        status,
        sizeof(status),
        "Match %d/%d",
        searchState.currentMatch + 1,
        searchState.matchCount
    );

    editorSetStatusMessage(status);
}

void previousMatch(void)
{
    if (searchState.matchCount == 0)
        return;

    searchState.currentMatch--;

    if (searchState.currentMatch < 0)
    {
        searchState.currentMatch =
            searchState.matchCount - 1;
    }

    cursor.y =
        searchState.matchRows[
            searchState.currentMatch
        ];

    cursor.x =
        searchState.matchCols[
            searchState.currentMatch
        ];

    scrollEditor();

    char status[64];

    snprintf(
        status,
        sizeof(status),
        "Match %d/%d",
        searchState.currentMatch + 1,
        searchState.matchCount
    );

    editorSetStatusMessage(status);
}

void replaceCurrentMatch(void)
{
    if (searchState.matchCount == 0 || searchState.currentMatch < 0 ||
        searchState.currentMatch >= searchState.matchCount)
    {
        editorSetStatusMessage(
            "Nothing to replace"
        );
        return;
    }

    int row =
        searchState.matchRows[
            searchState.currentMatch
        ];

    int col =
        searchState.matchCols[
            searchState.currentMatch
        ];

    if (!buffer.rows || row < 0 || row >= buffer.numRows)
    {
        editorSetStatusMessage(
            "Nothing to replace"
        );
        return;
    }

    Row *r = &buffer.rows[row];
    if (!r->chars)
        return;

    int queryLen =
        (int)strlen(
            searchState.query
        );
    int repLen =
        (int)strlen(
            searchState.replacement
        );

    if (queryLen <= 0 || col < 0 || col + queryLen > r->size)
    {
        editorSetStatusMessage(
            "Nothing to replace"
        );
        return;
    }

    size_t newSize = (size_t)r->size - (size_t)queryLen + (size_t)repLen;
    char *newChars = malloc(newSize + 1);
    if (!newChars)
        return;

    memcpy(newChars, r->chars, col);
    memcpy(newChars + col, searchState.replacement, repLen);
    memcpy(newChars + col + repLen, r->chars + col + queryLen, (size_t)(r->size - col - queryLen));
    newChars[newSize] = '\0';

    free(r->chars);
    r->chars = newChars;
    r->size = (int)newSize;

    free(r->hl);
    r->hl = NULL;
    r->hlSize = 0;

    buffer.modified = 1;

    char status[320];

    performSearch();

    if (searchState.matchCount > 0)
    {
        cursor.y =
            searchState.matchRows[0];

        cursor.x =
            searchState.matchCols[0];

        scrollEditor();

        snprintf(
            status,
            sizeof(status),
            "Replaced \"%s\" with \"%s\"",
            searchState.query,
            searchState.replacement
        );
    }
    else
    {
        snprintf(
            status,
            sizeof(status),
            "All matches replaced"
        );
    }

    editorSetStatusMessage(status);
}

void refreshSearchMatches(void)
{
    if (searchState.length <= 0)
        return;

    int savedX = cursor.x;
    int savedY = cursor.y;
    int oldRow = -1;
    int oldCol = -1;

    if (searchState.currentMatch >= 0 &&
        searchState.currentMatch < searchState.matchCount)
    {
        oldRow = searchState.matchRows[searchState.currentMatch];
        oldCol = searchState.matchCols[searchState.currentMatch];
    }

    searchState.matchCount = 0;
    searchState.currentMatch = 0;

    for (int row = 0; row < buffer.numRows; row++)
    {
        if (!buffer.rows || !buffer.rows[row].chars)
            continue;

        char *start = buffer.rows[row].chars;

        while (1)
        {
            char *match = strstr(start, searchState.query);
            
            
            if (!match)
                break;

            if (isWholeWordMatch(
                    buffer.rows[row].chars,
                    match,
                    (size_t)searchState.length))
            {
                if (searchState.matchCount < 256)
                {
                    searchState.matchRows[
                        searchState.matchCount
                    ] = row;

                    searchState.matchCols[
                        searchState.matchCount
                    ] = (int)(
                        match - buffer.rows[row].chars
                    );

                    searchState.matchCount++;
                }
            }

            start = match + 1;
        }
    }


    for (int i = 0; i < searchState.matchCount; i++)
    {
        if (searchState.matchRows[i] == oldRow &&
            searchState.matchCols[i] == oldCol)
        {
            searchState.currentMatch = i;
            break;
        }
    }

    cursor.x = savedX;
    cursor.y = savedY;
}

int replaceAllMatches(void)
{
    if (searchState.length <= 0)
    {
        editorSetStatusMessage(
            "Nothing to replace"
        );
        return 0;
    }

    int replacements = 0;
    size_t qLen = (size_t)searchState.length;
    size_t repLen = strlen(searchState.replacement);

    for (int row = 0; row < buffer.numRows; row++)
    {
        Row *r = &buffer.rows[row];
        if (!r->chars || r->size <= 0)
            continue;

        int rowMatches = 0;
        const char *p = r->chars;
        while ((p = strstr(p, searchState.query)) != NULL)
        {
            if (isWholeWordMatch(r->chars, p, qLen))
            {
                rowMatches++;
                p += qLen;
            }
            else
            {
                p += 1;
            }
        }

        if (rowMatches == 0)
            continue;

        size_t extraPerMatch = (repLen > qLen) ? (repLen - qLen) : 0;
        size_t newAlloc = (size_t)r->size + (size_t)rowMatches * extraPerMatch + 1;
        char *newChars = malloc(newAlloc);
        if (!newChars)
            continue;

        size_t newLen = 0;
        size_t srcIdx = 0;
        while (srcIdx < (size_t)r->size)
        {
            char *m = strstr(r->chars + srcIdx, searchState.query);
            if (!m)
            {
                size_t tail = (size_t)r->size - srcIdx;
                memcpy(newChars + newLen, r->chars + srcIdx, tail);
                newLen += tail;
                break;
            }

            if (isWholeWordMatch(r->chars, m, qLen))
            {
                size_t prefixLen = (size_t)(m - (r->chars + srcIdx));
                memcpy(newChars + newLen, r->chars + srcIdx, prefixLen);
                newLen += prefixLen;
                memcpy(newChars + newLen, searchState.replacement, repLen);
                newLen += repLen;
                srcIdx = (size_t)(m - r->chars) + qLen;
                replacements++;
            }
            else
            {
                size_t copyLen = (size_t)(m - (r->chars + srcIdx)) + 1;
                memcpy(newChars + newLen, r->chars + srcIdx, copyLen);
                newLen += copyLen;
                srcIdx += copyLen;
            }
        }

        newChars[newLen] = '\0';
        free(r->chars);
        r->chars = newChars;
        r->size = (int)newLen;

        free(r->hl);
        r->hl = NULL;
        r->hlSize = 0;
    }

    if (replacements > 0)
    {
        buffer.modified = 1;
        refreshSearchMatches();
    }

    char status[64];
    snprintf(
        status,
        sizeof(status),
        "%d replacements made",
        replacements
    );

    editorSetStatusMessage(status);

    return replacements;
}