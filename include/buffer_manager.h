#ifndef BUFFER_MANAGER_H
#define BUFFER_MANAGER_H

#include "buffer.h"
#include "cursor.h"

#define BUFFER_MAX_COUNT 32
#define BUFFER_PATH_MAX 1024
#define BUFFER_NAME_MAX 256

typedef struct
{
    int id;
    char filename[BUFFER_PATH_MAX];
    char displayName[BUFFER_NAME_MAX];
    Buffer doc;
    Cursor cursor;
    int rowOffset;
    int colOffset;
    int active;
} FileBuffer;

typedef struct
{
    FileBuffer buffers[BUFFER_MAX_COUNT];
    int count;
    int activeIndex;
} BufferManager;

extern BufferManager bufferManager;

/* Initialize the buffer manager subsystem. */
void bufferManagerInit(void);

/* Get number of open buffers. */
int bufferManagerCount(void);

/* Get index of currently active buffer (0-based). */
int bufferManagerActiveIndex(void);

/* Get pointer to buffer at specified index, or NULL if invalid. */
FileBuffer *bufferManagerGet(int index);

/* Get pointer to currently active buffer, or NULL if none. */
FileBuffer *bufferManagerGetActive(void);

/* Find buffer index by filename, or -1 if not open. */
int bufferManagerFind(const char *filename);

/* Open file into a new or existing buffer and activate it. Returns index or -1 on error. */
int bufferManagerOpen(const char *filename);

/* Create a new untitled/empty buffer and activate it. Returns index or -1 on error. */
int bufferManagerNew(const char *optionalName);

/* Close buffer at specified index. Returns 1 on success, 0 on failure. */
int bufferManagerClose(int index);

/* Close currently active buffer. Returns 1 on success, 0 on failure. */
int bufferManagerCloseActive(void);

/* Cycle to the next open buffer. Returns 1 on success, 0 if only one buffer. */
int bufferManagerNext(void);

/* Cycle to the previous open buffer. Returns 1 on success, 0 if only one buffer. */
int bufferManagerPrev(void);

/* Switch directly to buffer at specified index. Returns 1 on success, 0 on failure. */
int bufferManagerSwitchTo(int index);

/* Synchronize global editor state (buffer, cursor, viewport, currentFile) into active slot. */
void bufferManagerSyncFromGlobal(void);

/* Synchronize active slot into global editor state. */
void bufferManagerSyncToGlobal(void);

/* Free all buffers in manager. */
void bufferManagerFreeAll(void);

/* Get clean display name for buffer at index (e.g. filename without directory). */
const char *bufferManagerGetDisplayName(int index);

/* Get language name for buffer at index (e.g. "C", "Python", "Rust", or "Plain Text"). */
const char *bufferManagerGetLanguage(int index);

#endif
