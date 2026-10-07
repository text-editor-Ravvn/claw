#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <dirent.h>
#include <sys/stat.h>

#include "explorer.h"
#include "workspace.h"
#include "fileio.h"
#include "editor.h"
#include "recent.h"
#include "bookmark.h"

ExplorerState explorerState = {0};
extern char *currentFile;

static int entryCompare(const void *a, const void *b)
{
    const ExplorerEntry *ea = (const ExplorerEntry *)a;
    const ExplorerEntry *eb = (const ExplorerEntry *)b;

    /* Directories first */
    if (ea->isDir && !eb->isDir)
        return -1;
    if (!ea->isDir && eb->isDir)
        return 1;

    return strcasecmp(ea->name, eb->name);
}

void explorerRefresh(void)
{
    explorerState.count = 0;

    DIR *dir = opendir(explorerState.currentDir);
    if (!dir)
        return;

    struct dirent *d;
    while ((d = readdir(dir)) != NULL)
    {
        /* Ignore . and .. */
        if (strcmp(d->d_name, ".") == 0 || strcmp(d->d_name, "..") == 0)
            continue;

        /* Ignore .git directory by default */
        if (strcmp(d->d_name, ".git") == 0)
            continue;

        if (explorerState.count >= EXPLORER_MAX_ENTRIES)
            break;

        ExplorerEntry *entry = &explorerState.entries[explorerState.count];
        snprintf(entry->name, sizeof(entry->name), "%s", d->d_name);
        snprintf(entry->fullPath, sizeof(entry->fullPath), "%.1500s/%s",
                 explorerState.currentDir, d->d_name);

        struct stat st;
        if (stat(entry->fullPath, &st) == 0)
        {
#ifdef S_ISDIR
            entry->isDir = S_ISDIR(st.st_mode);
#else
            entry->isDir = (st.st_mode & _S_IFDIR) != 0;
#endif
            entry->sizeBytes = (long)st.st_size;
        }
        else
        {
            entry->isDir = 0;
            entry->sizeBytes = 0;
        }

        explorerState.count++;
    }

    closedir(dir);

    /* Sort entries: directories first, then alphabetical */
    if (explorerState.count > 1)
    {
        qsort(explorerState.entries, explorerState.count, sizeof(ExplorerEntry), entryCompare);
    }

    if (explorerState.selectedIndex >= explorerState.count)
    {
        explorerState.selectedIndex = explorerState.count > 0 ? explorerState.count - 1 : 0;
    }
}

void explorerOpen(const char *dirPath)
{
    explorerState.active = 1;
    explorerState.selectedIndex = 0;
    explorerState.scrollOffset = 0;

    if (dirPath && strlen(dirPath) > 0)
    {
        snprintf(explorerState.currentDir, sizeof(explorerState.currentDir), "%s", dirPath);
    }
    else if (workspaceIsActive())
    {
        snprintf(explorerState.currentDir, sizeof(explorerState.currentDir), "%s", workspaceGetRoot());
    }
    else
    {
        snprintf(explorerState.currentDir, sizeof(explorerState.currentDir), ".");
    }

    explorerRefresh();
}

void explorerClose(void)
{
    explorerState.active = 0;
}

void explorerSelectNext(void)
{
    if (explorerState.count == 0)
        return;

    if (explorerState.selectedIndex < explorerState.count - 1)
    {
        explorerState.selectedIndex++;
    }
}

void explorerSelectPrev(void)
{
    if (explorerState.count == 0)
        return;

    if (explorerState.selectedIndex > 0)
    {
        explorerState.selectedIndex--;
    }
}

int explorerEnter(void)
{
    if (explorerState.count == 0 || explorerState.selectedIndex >= explorerState.count)
        return 0;

    ExplorerEntry *sel = &explorerState.entries[explorerState.selectedIndex];
    if (sel->isDir)
    {
        snprintf(explorerState.currentDir, sizeof(explorerState.currentDir), "%s", sel->fullPath);
        explorerState.selectedIndex = 0;
        explorerState.scrollOffset = 0;
        explorerRefresh();
        return 0;
    }
    else
    {
        if (openFile(sel->fullPath))
        {
            if (currentFile)
                free(currentFile);
            currentFile = strdup(sel->fullPath);
            recentAddFile(sel->fullPath);
            bookmarkClearAll();
            explorerClose();
            return 1;
        }
    }
    return 0;
}

void explorerGoUp(void)
{
    char *slash = strrchr(explorerState.currentDir, '/');
    char *backslash = strrchr(explorerState.currentDir, '\\');
    char *lastSep = (slash > backslash) ? slash : backslash;

    if (lastSep && lastSep != explorerState.currentDir)
    {
        *lastSep = '\0';
        explorerState.selectedIndex = 0;
        explorerState.scrollOffset = 0;
        explorerRefresh();
    }
    else if (!lastSep && strcmp(explorerState.currentDir, ".") == 0)
    {
        snprintf(explorerState.currentDir, sizeof(explorerState.currentDir), "..");
        explorerState.selectedIndex = 0;
        explorerState.scrollOffset = 0;
        explorerRefresh();
    }
}

const char *explorerGetSelectedPath(void)
{
    if (explorerState.count == 0 || explorerState.selectedIndex >= explorerState.count)
        return NULL;
    return explorerState.entries[explorerState.selectedIndex].fullPath;
}

int explorerIsSelectedDir(void)
{
    if (explorerState.count == 0 || explorerState.selectedIndex >= explorerState.count)
        return 0;
    return explorerState.entries[explorerState.selectedIndex].isDir;
}
