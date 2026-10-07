#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <dirent.h>

#include "file_switcher.h"
#include "recent.h"
#include "workspace.h"
#include "editor.h"
#include "fileio.h"

FileSwitcherState fileSwitcherState = {0};
extern char *currentFile;

static int containsCaseInsensitive(const char *haystack, const char *needle)
{
    if (!needle || !*needle)
        return 1;
    if (!haystack || !*haystack)
        return 0;

    size_t hlen = strlen(haystack);
    size_t nlen = strlen(needle);
    if (nlen > hlen)
        return 0;

    for (size_t i = 0; i <= hlen - nlen; i++)
    {
        size_t j = 0;
        while (j < nlen && tolower((unsigned char)haystack[i + j]) == tolower((unsigned char)needle[j]))
        {
            j++;
        }
        if (j == nlen)
            return 1;
    }
    return 0;
}

static void addCandidate(const char *path)
{
    if (!path || !*path || fileSwitcherState.allCount >= SWITCHER_MAX_ITEMS)
        return;

    for (int i = 0; i < fileSwitcherState.allCount; i++)
    {
        if (strcmp(fileSwitcherState.allFiles[i], path) == 0)
            return;
    }

    snprintf(fileSwitcherState.allFiles[fileSwitcherState.allCount],
             sizeof(fileSwitcherState.allFiles[0]), "%s", path);
    fileSwitcherState.allCount++;
}

void fileSwitcherFilter(void)
{
    fileSwitcherState.filteredCount = 0;

    for (int i = 0; i < fileSwitcherState.allCount; i++)
    {
        if (fileSwitcherState.queryLen == 0 ||
            containsCaseInsensitive(fileSwitcherState.allFiles[i], fileSwitcherState.query))
        {
            if (fileSwitcherState.filteredCount < SWITCHER_MAX_ITEMS)
            {
                fileSwitcherState.filteredIndices[fileSwitcherState.filteredCount++] = i;
            }
        }
    }

    if (fileSwitcherState.selectedIndex >= fileSwitcherState.filteredCount)
    {
        fileSwitcherState.selectedIndex = fileSwitcherState.filteredCount > 0 ? fileSwitcherState.filteredCount - 1 : 0;
    }
}

void fileSwitcherOpen(void)
{
    fileSwitcherState.active = 1;
    fileSwitcherState.query[0] = '\0';
    fileSwitcherState.queryLen = 0;
    fileSwitcherState.selectedIndex = 0;
    fileSwitcherState.allCount = 0;

    /* 1. Add current file */
    if (currentFile && strlen(currentFile) > 0)
    {
        addCandidate(currentFile);
    }

    /* 2. Add recent files */
    int rCount = recentGetFileCount();
    for (int i = 0; i < rCount; i++)
    {
        addCandidate(recentGetFile(i));
    }

    /* 3. Add workspace files from root */
    const char *wsRoot = workspaceIsActive() ? workspaceGetRoot() : ".";
    DIR *dir = opendir(wsRoot);
    if (dir)
    {
        struct dirent *d;
        while ((d = readdir(dir)) != NULL)
        {
            if (d->d_name[0] == '.')
                continue;

            char filePath[SWITCHER_PATH_MAX];
            snprintf(filePath, sizeof(filePath), "%s/%s", wsRoot, d->d_name);
            addCandidate(filePath);

            if (fileSwitcherState.allCount >= SWITCHER_MAX_ITEMS)
                break;
        }
        closedir(dir);
    }

    fileSwitcherFilter();
}

void fileSwitcherClose(void)
{
    fileSwitcherState.active = 0;
}

void fileSwitcherSelectNext(void)
{
    if (fileSwitcherState.filteredCount == 0)
        return;

    if (fileSwitcherState.selectedIndex < fileSwitcherState.filteredCount - 1)
    {
        fileSwitcherState.selectedIndex++;
    }
}

void fileSwitcherSelectPrev(void)
{
    if (fileSwitcherState.filteredCount == 0)
        return;

    if (fileSwitcherState.selectedIndex > 0)
    {
        fileSwitcherState.selectedIndex--;
    }
}

const char *fileSwitcherGetSelected(void)
{
    if (fileSwitcherState.filteredCount == 0 ||
        fileSwitcherState.selectedIndex >= fileSwitcherState.filteredCount)
    {
        return NULL;
    }

    int realIdx = fileSwitcherState.filteredIndices[fileSwitcherState.selectedIndex];
    return fileSwitcherState.allFiles[realIdx];
}

void fileSwitcherAddChar(char c)
{
    if (fileSwitcherState.queryLen < SWITCHER_QUERY_MAX - 1)
    {
        fileSwitcherState.query[fileSwitcherState.queryLen++] = c;
        fileSwitcherState.query[fileSwitcherState.queryLen] = '\0';
        fileSwitcherFilter();
    }
}

void fileSwitcherBackspace(void)
{
    if (fileSwitcherState.queryLen > 0)
    {
        fileSwitcherState.query[--fileSwitcherState.queryLen] = '\0';
        fileSwitcherFilter();
    }
}
