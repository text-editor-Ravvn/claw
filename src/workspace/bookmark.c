#include <stdio.h>
#include <string.h>

#include "bookmark.h"

BookmarkList bookmarkList = {0};

void bookmarkInit(void)
{
    bookmarkList.count = 0;
}

int bookmarkIsBookmarked(int line)
{
    for (int i = 0; i < bookmarkList.count; i++)
    {
        if (bookmarkList.lines[i] == line)
            return 1;
    }
    return 0;
}

int bookmarkToggle(int line)
{
    if (line < 0)
        return -1;

    for (int i = 0; i < bookmarkList.count; i++)
    {
        if (bookmarkList.lines[i] == line)
        {
            /* Remove existing bookmark */
            for (int j = i; j < bookmarkList.count - 1; j++)
            {
                bookmarkList.lines[j] = bookmarkList.lines[j + 1];
            }
            bookmarkList.count--;
            return 0; /* Removed */
        }
    }

    if (bookmarkList.count >= BOOKMARK_MAX)
        return -1; /* Full */

    /* Insert in sorted order */
    int idx = bookmarkList.count;
    for (int i = 0; i < bookmarkList.count; i++)
    {
        if (bookmarkList.lines[i] > line)
        {
            idx = i;
            break;
        }
    }

    for (int j = bookmarkList.count; j > idx; j--)
    {
        bookmarkList.lines[j] = bookmarkList.lines[j - 1];
    }
    bookmarkList.lines[idx] = line;
    bookmarkList.count++;
    return 1; /* Added */
}

int bookmarkNext(int currentLine)
{
    if (bookmarkList.count == 0)
        return -1;

    for (int i = 0; i < bookmarkList.count; i++)
    {
        if (bookmarkList.lines[i] > currentLine)
            return bookmarkList.lines[i];
    }

    /* Wrap around to first */
    return bookmarkList.lines[0];
}

int bookmarkPrev(int currentLine)
{
    if (bookmarkList.count == 0)
        return -1;

    for (int i = bookmarkList.count - 1; i >= 0; i--)
    {
        if (bookmarkList.lines[i] < currentLine)
            return bookmarkList.lines[i];
    }

    /* Wrap around to last */
    return bookmarkList.lines[bookmarkList.count - 1];
}

void bookmarkClearAll(void)
{
    bookmarkList.count = 0;
}

int bookmarkCount(void)
{
    return bookmarkList.count;
}
