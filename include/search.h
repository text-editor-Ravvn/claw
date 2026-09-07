#ifndef SEARCH_H
#define SEARCH_H

typedef struct
{
    int active;

    char query[128];

    int length;

    int matchRows[256];
    int matchCols[256];

    int matchCount;
    int currentMatch;

} SearchState;

extern SearchState searchState;

void openSearchPrompt(void);
void closeSearchPrompt(void);
void performSearch(void);
void nextMatch(void);
void previousMatch(void);

#endif