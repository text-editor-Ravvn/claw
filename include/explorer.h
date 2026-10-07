#ifndef EXPLORER_H
#define EXPLORER_H

#include <stddef.h>

#define EXPLORER_MAX_ENTRIES 256
#define EXPLORER_PATH_MAX 1024
#define EXPLORER_NAME_MAX 256

typedef struct
{
    char name[EXPLORER_NAME_MAX];
    char fullPath[EXPLORER_PATH_MAX];
    int isDir;
    long sizeBytes;
} ExplorerEntry;

typedef struct
{
    int active;
    char currentDir[EXPLORER_PATH_MAX];
    ExplorerEntry entries[EXPLORER_MAX_ENTRIES];
    int count;
    int selectedIndex;
    int scrollOffset;
} ExplorerState;

extern ExplorerState explorerState;

/* Open the file explorer at the specified directory path. */
void explorerOpen(const char *dirPath);

/* Close the file explorer modal. */
void explorerClose(void);

/* Refresh directory contents. */
void explorerRefresh(void);

/* Select next entry in directory listing. */
void explorerSelectNext(void);

/* Select previous entry in directory listing. */
void explorerSelectPrev(void);

/* Enter selected entry (open file or change into directory). Returns 1 if file was opened. */
int explorerEnter(void);

/* Navigate up to parent directory. */
void explorerGoUp(void);

/* Get path of currently selected entry. */
const char *explorerGetSelectedPath(void);

/* Check if selected entry is a directory. */
int explorerIsSelectedDir(void);

#endif
