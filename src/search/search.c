#include <string.h>

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

    searchState.length = 0;

    searchState.query[0] = '\0';

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

    editorSetStatusMessage(
        "Match found"
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

    editorSetStatusMessage(
        "Next match"
    );
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

    editorSetStatusMessage(
        "Previous match"
    );
}