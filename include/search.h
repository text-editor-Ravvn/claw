#ifndef SEARCH_H
#define SEARCH_H

typedef struct
{
    int active;

    int replaceMode;

    char query[128];
    int length;

    char replacement[128];
    int replacementLength;

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
void openReplacePrompt(void);
void replaceCurrentMatch(void);
int replaceAllMatches(void);

#endif