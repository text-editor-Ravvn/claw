#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

#include "recent.h"
#include "utils.h"

RecentTracker recentTracker = {0};

static void getDefaultRecentPath(char *outPath, size_t outSize)
{
    const char *xdg = getenv("XDG_CONFIG_HOME");
    if (xdg && strlen(xdg) > 0)
    {
        snprintf(outPath, outSize, "%s/claw/recent.txt", xdg);
        return;
    }

    const char *home = getenv("HOME");
    if (home && strlen(home) > 0)
    {
        snprintf(outPath, outSize, "%s/.config/claw/recent.txt", home);
        return;
    }

    snprintf(outPath, outSize, "config/recent.txt");
}

void recentAddFile(const char *path)
{
    if (!path || !*path)
        return;

    int found = -1;
    for (int i = 0; i < recentTracker.fileCount; i++)
    {
        if (strcmp(recentTracker.files[i], path) == 0)
        {
            found = i;
            break;
        }
    }

    int limit = recentTracker.fileCount;
    if (found != -1)
    {
        limit = found;
    }
    else if (recentTracker.fileCount < RECENT_MAX_ITEMS)
    {
        recentTracker.fileCount++;
        limit = recentTracker.fileCount - 1;
    }
    else
    {
        limit = RECENT_MAX_ITEMS - 1;
    }

    for (int i = limit; i > 0; i--)
    {
        snprintf(recentTracker.files[i], sizeof(recentTracker.files[i]), "%s", recentTracker.files[i - 1]);
    }

    snprintf(recentTracker.files[0], sizeof(recentTracker.files[0]), "%s", path);
    recentSaveDefault();
}

void recentAddWorkspace(const char *path)
{
    if (!path || !*path)
        return;

    int found = -1;
    for (int i = 0; i < recentTracker.workspaceCount; i++)
    {
        if (strcmp(recentTracker.workspaces[i], path) == 0)
        {
            found = i;
            break;
        }
    }

    int limit = recentTracker.workspaceCount;
    if (found != -1)
    {
        limit = found;
    }
    else if (recentTracker.workspaceCount < RECENT_MAX_ITEMS)
    {
        recentTracker.workspaceCount++;
        limit = recentTracker.workspaceCount - 1;
    }
    else
    {
        limit = RECENT_MAX_ITEMS - 1;
    }

    for (int i = limit; i > 0; i--)
    {
        snprintf(recentTracker.workspaces[i], sizeof(recentTracker.workspaces[i]), "%s", recentTracker.workspaces[i - 1]);
    }

    snprintf(recentTracker.workspaces[0], sizeof(recentTracker.workspaces[0]), "%s", path);
    recentSaveDefault();
}

int recentGetFileCount(void)
{
    return recentTracker.fileCount;
}

const char *recentGetFile(int index)
{
    if (index < 0 || index >= recentTracker.fileCount)
        return NULL;
    return recentTracker.files[index];
}

int recentGetWorkspaceCount(void)
{
    return recentTracker.workspaceCount;
}

const char *recentGetWorkspace(int index)
{
    if (index < 0 || index >= recentTracker.workspaceCount)
        return NULL;
    return recentTracker.workspaces[index];
}

int recentSave(const char *path)
{
    if (!path || !*path)
        return 0;

    FILE *fp = fopen(path, "w");
    if (!fp)
        return 0;

    fprintf(fp, "[files]\n");
    for (int i = 0; i < recentTracker.fileCount; i++)
    {
        fprintf(fp, "%s\n", recentTracker.files[i]);
    }

    fprintf(fp, "[workspaces]\n");
    for (int i = 0; i < recentTracker.workspaceCount; i++)
    {
        fprintf(fp, "%s\n", recentTracker.workspaces[i]);
    }

    fclose(fp);
    return 1;
}

int recentLoad(const char *path)
{
    if (!path || !*path)
        return 0;

    FILE *fp = fopen(path, "r");
    if (!fp)
        return 0;

    recentTracker.fileCount = 0;
    recentTracker.workspaceCount = 0;

    char line[RECENT_PATH_MAX];
    int section = 0; /* 1 = files, 2 = workspaces */

    while (fgets(line, sizeof(line), fp))
    {
        line[strcspn(line, "\r\n")] = '\0';
        char *trimmed = trimWhitespace(line);
        if (*trimmed == '\0' || *trimmed == '#')
            continue;

        if (strcmp(trimmed, "[files]") == 0)
        {
            section = 1;
            continue;
        }
        else if (strcmp(trimmed, "[workspaces]") == 0)
        {
            section = 2;
            continue;
        }

        if (section == 1 && recentTracker.fileCount < RECENT_MAX_ITEMS)
        {
            snprintf(recentTracker.files[recentTracker.fileCount++],
                     sizeof(recentTracker.files[0]), "%s", trimmed);
        }
        else if (section == 2 && recentTracker.workspaceCount < RECENT_MAX_ITEMS)
        {
            snprintf(recentTracker.workspaces[recentTracker.workspaceCount++],
                     sizeof(recentTracker.workspaces[0]), "%s", trimmed);
        }
    }

    fclose(fp);
    return 1;
}

void recentSaveDefault(void)
{
    char defPath[512];
    getDefaultRecentPath(defPath, sizeof(defPath));
    recentSave(defPath);
}

void recentInit(void)
{
    memset(&recentTracker, 0, sizeof(recentTracker));
    char defPath[512];
    getDefaultRecentPath(defPath, sizeof(defPath));
    recentLoad(defPath);
}
