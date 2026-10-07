#ifndef BOOKMARK_H
#define BOOKMARK_H

#define BOOKMARK_MAX 128

typedef struct
{
    int lines[BOOKMARK_MAX];
    int count;
} BookmarkList;

extern BookmarkList bookmarkList;

/* Initialize bookmark subsystem. */
void bookmarkInit(void);

/* Toggle bookmark on the given line. Returns 1 if added, 0 if removed, -1 if full. */
int bookmarkToggle(int line);

/* Check if line is bookmarked. Returns 1 if bookmarked, 0 otherwise. */
int bookmarkIsBookmarked(int line);

/* Find next bookmarked line after currentLine (wraps around). Returns -1 if no bookmarks. */
int bookmarkNext(int currentLine);

/* Find previous bookmarked line before currentLine (wraps around). Returns -1 if no bookmarks. */
int bookmarkPrev(int currentLine);

/* Clear all bookmarks in active buffer. */
void bookmarkClearAll(void);

/* Return total number of active bookmarks. */
int bookmarkCount(void);

#endif
