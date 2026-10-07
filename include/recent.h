#ifndef RECENT_H
#define RECENT_H

#define RECENT_MAX_ITEMS 16
#define RECENT_PATH_MAX 1024

typedef struct
{
    char files[RECENT_MAX_ITEMS][RECENT_PATH_MAX];
    int fileCount;
    char workspaces[RECENT_MAX_ITEMS][RECENT_PATH_MAX];
    int workspaceCount;
} RecentTracker;

extern RecentTracker recentTracker;

/* Initialize recent tracker and load stored history. */
void recentInit(void);

/* Add a file to MRU history (moves to index 0 if already present). */
void recentAddFile(const char *path);

/* Add a workspace to MRU history. */
void recentAddWorkspace(const char *path);

/* Get number of recent files. */
int recentGetFileCount(void);

/* Get recent file path at index (0 = most recent). */
const char *recentGetFile(int index);

/* Get number of recent workspaces. */
int recentGetWorkspaceCount(void);

/* Get recent workspace path at index. */
const char *recentGetWorkspace(int index);

/* Save recent entries to disk file. */
int recentSave(const char *path);

/* Load recent entries from disk file. */
int recentLoad(const char *path);

/* Save using default persistent path. */
void recentSaveDefault(void);

#endif
