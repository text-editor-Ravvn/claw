#include "search.h"
#include "editor.h"

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