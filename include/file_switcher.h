#ifndef FILE_SWITCHER_H
#define FILE_SWITCHER_H

#define SWITCHER_MAX_ITEMS 64
#define SWITCHER_QUERY_MAX 128
#define SWITCHER_PATH_MAX 512

typedef struct
{
    int active;
    char query[SWITCHER_QUERY_MAX];
    int queryLen;
    int selectedIndex;

    char allFiles[SWITCHER_MAX_ITEMS][SWITCHER_PATH_MAX];
    int allCount;

    int filteredIndices[SWITCHER_MAX_ITEMS];
    int filteredCount;
} FileSwitcherState;

extern FileSwitcherState fileSwitcherState;

/* Open the quick file switcher modal populated with recent and workspace files. */
void fileSwitcherOpen(void);

/* Close the file switcher modal. */
void fileSwitcherClose(void);

/* Filter candidate files based on active query string. */
void fileSwitcherFilter(void);

/* Navigate selection down. */
void fileSwitcherSelectNext(void);

/* Navigate selection up. */
void fileSwitcherSelectPrev(void);

/* Get path of currently selected item, or NULL if none. */
const char *fileSwitcherGetSelected(void);

/* Append character to query filter. */
void fileSwitcherAddChar(char c);

/* Delete character from query filter. */
void fileSwitcherBackspace(void);

#endif
