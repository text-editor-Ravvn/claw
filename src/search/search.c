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
    if (searchState.length == 0)
        return;

    for (int row = 0; row < buffer.numRows; row++)
    {
        char *match =
            strstr(
                buffer.rows[row].chars,
                searchState.query
            );

        if (match)
        {
            cursor.y = row;

            cursor.x =
                (int)(
                    match -
                    buffer.rows[row].chars
                );

            scrollEditor();

            editorSetStatusMessage(
                "Match found"
            );

            return;
        }
    }

    editorSetStatusMessage(
        "No matches found"
    );
}