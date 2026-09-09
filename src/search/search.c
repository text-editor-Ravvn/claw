#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "search.h"
#include "editor.h"
#include "buffer.h"
#include "cursor.h"
#include "viewport.h"

extern Buffer buffer;
extern Cursor cursor;

SearchState searchState = {0};

void openSearchPrompt(void)
{
    searchState.active = 1;

    searchState.replaceMode = 0;

    searchState.length = 0;

    searchState.query[0] = '\0';

    editorSetStatusMessage("");
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

    /* KEEP searchState.query intact */

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

            if (searchState.matchCount < 256)
            {
                searchState.matchRows[
                    searchState.matchCount
                ] = row;

                searchState.matchCols[
                    searchState.matchCount
                ] =
                (int)(
                    match -
                    buffer.rows[row].chars
                );

                searchState.matchCount++;
            }

            start =
                match + 1;
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
    if (searchState.matchCount == 0)
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

    Row *r = &buffer.rows[row];

    char newLine[2048];

    int queryLen =
    strlen(searchState.query);

snprintf(
    newLine,
    sizeof(newLine),
    "%.*s%s%s",
    col,
    r->chars,
    searchState.replacement,
    r->chars + col + queryLen
);

    free(r->chars);

    r->chars =
        strdup(newLine);

    r->size =
        (int)strlen(newLine);

    buffer.modified = 1;

    char status[128];

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
int replaceAllMatches(void)
{
    performSearch();

    if (searchState.matchCount == 0)
    {
        editorSetStatusMessage(
            "Nothing to replace"
        );
        return 0;
    }

    int replacements = 0;

    while (1)
    {
        performSearch();

        if (searchState.matchCount == 0)
            break;

        searchState.currentMatch = 0;

        int row =
            searchState.matchRows[0];

        int col =
            searchState.matchCols[0];

        Row *r =
            &buffer.rows[row];

        char newLine[2048];

        int queryLen =
            (int)strlen(searchState.query);

        snprintf(
            newLine,
            sizeof(newLine),
            "%.*s%s%s",
            col,
            r->chars,
            searchState.replacement,
            r->chars + col + queryLen
        );

        free(r->chars);

        r->chars = strdup(newLine);

        r->size =
            (int)strlen(newLine);

        replacements++;
    }

    buffer.modified = 1;

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