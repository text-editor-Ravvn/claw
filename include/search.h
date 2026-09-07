#ifndef SEARCH_H
#define SEARCH_H

typedef struct
{
    int active;
    char query[128];
    int length;
} SearchState;

extern SearchState searchState;

void openSearchPrompt(void);
void closeSearchPrompt(void);

#endif