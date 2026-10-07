#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
#include <limits.h>

#include "workspace.h"
#include "config.h"

Workspace currentWorkspace = {0};

static int directoryExists(const char *path)
{
    struct stat st;
    if (stat(path, &st) == 0)
    {
#ifdef S_ISDIR
        return S_ISDIR(st.st_mode);
#else
        return (st.st_mode & _S_IFDIR) != 0;
#endif
    }
    return 0;
}

static void extractWorkspaceName(const char *path, char *outName, size_t outSize)
{
    if (!path || !*path || outSize == 0)
    {
        if (outSize > 0)
            outName[0] = '\0';
        return;
    }

    size_t len = strlen(path);
    while (len > 1 && (path[len - 1] == '/' || path[len - 1] == '\\'))
    {
        len--;
    }

    const char *lastSlash = NULL;
    for (size_t i = 0; i < len; i++)
    {
        if (path[i] == '/' || path[i] == '\\')
            lastSlash = &path[i];
    }

    const char *start = lastSlash ? lastSlash + 1 : path;
    size_t nameLen = (path + len) - start;
    if (nameLen >= outSize)
        nameLen = outSize - 1;

    memcpy(outName, start, nameLen);
    outName[nameLen] = '\0';
}

int workspaceDetectRoot(const char *startPath, char *outRoot, size_t outSize)
{
    if (!startPath || !outRoot || outSize == 0)
        return 0;

    char current[WORKSPACE_PATH_MAX];
    snprintf(current, sizeof(current), "%s", startPath);

    /* Normalize trailing slashes */
    size_t len = strlen(current);
    while (len > 1 && (current[len - 1] == '/' || current[len - 1] == '\\'))
    {
        current[--len] = '\0';
    }

    char candidate[WORKSPACE_PATH_MAX];

    while (1)
    {
        /* 1. Check for .claw directory or marker */
        snprintf(candidate, sizeof(candidate), "%s/.claw", current);
        if (directoryExists(candidate))
        {
            snprintf(outRoot, outSize, "%s", current);
            return 1;
        }

        /* 2. Check for .git directory */
        snprintf(candidate, sizeof(candidate), "%s/.git", current);
        if (directoryExists(candidate))
        {
            snprintf(outRoot, outSize, "%s", current);
            return 1;
        }

        /* Find parent directory */
        char *slash = strrchr(current, '/');
        char *backslash = strrchr(current, '\\');
        char *lastSep = (slash > backslash) ? slash : backslash;

        if (!lastSep)
            break;

        if (lastSep == current)
        {
            current[1] = '\0';
            break;
        }

        *lastSep = '\0';
    }

    /* Fallback to startPath */
    snprintf(outRoot, outSize, "%s", startPath);
    return 1;
}

int workspaceOpen(const char *path)
{
    if (!path || !*path)
        return 0;

    char detectedRoot[WORKSPACE_PATH_MAX];
    if (!workspaceDetectRoot(path, detectedRoot, sizeof(detectedRoot)))
    {
        snprintf(detectedRoot, sizeof(detectedRoot), "%s", path);
    }

    snprintf(currentWorkspace.rootPath, sizeof(currentWorkspace.rootPath), "%s", detectedRoot);
    extractWorkspaceName(detectedRoot, currentWorkspace.name, sizeof(currentWorkspace.name));

    /* Check for .claw configuration file */
    snprintf(currentWorkspace.configPath, sizeof(currentWorkspace.configPath),
             "%s/.claw/workspace.conf", detectedRoot);

    struct stat st;
    if (stat(currentWorkspace.configPath, &st) != 0)
    {
        currentWorkspace.configPath[0] = '\0';
    }

    currentWorkspace.active = 1;
    return 1;
}

void workspaceClose(void)
{
    memset(&currentWorkspace, 0, sizeof(currentWorkspace));
}

void workspaceInit(void)
{
    workspaceClose();

    char cwd[WORKSPACE_PATH_MAX];
    if (getcwd(cwd, sizeof(cwd)))
    {
        workspaceOpen(cwd);
    }
}

const char *workspaceGetRoot(void)
{
    return currentWorkspace.active ? currentWorkspace.rootPath : NULL;
}

const char *workspaceGetName(void)
{
    return currentWorkspace.active ? currentWorkspace.name : "None";
}

int workspaceIsActive(void)
{
    return currentWorkspace.active;
}

int workspaceLoadConfig(void)
{
    if (!currentWorkspace.active || currentWorkspace.configPath[0] == '\0')
        return 0;

    return configLoad(currentWorkspace.configPath);
}
