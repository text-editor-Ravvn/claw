/* Placeholder for shared Git repository operations. */
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <limits.h>
#include <sys/stat.h>

#include "git.h"

static char repositoryRoot[PATH_MAX];

int gitIsRepository(void)
{
    char current[PATH_MAX];

    if (!getcwd(current, sizeof(current)))
        return 0;

    while (1)
    {
        char gitPath[PATH_MAX];

        snprintf(
            gitPath,
            sizeof(gitPath),
            "%s/.git",
            current
        );

        struct stat st;

        if (stat(gitPath, &st) == 0 &&
            S_ISDIR(st.st_mode))
        {
            strcpy(repositoryRoot, current);
            return 1;
        }

        char *slash =
            strrchr(current, '/');

        if (!slash)
            break;

        if (slash == current)
        {
            current[1] = '\0';
        }
        else
        {
            *slash = '\0';
        }

        if (strcmp(current, "/") == 0)
        {
            snprintf(
                gitPath,
                sizeof(gitPath),
                "/.git"
            );

            if (stat(gitPath, &st) == 0)
            {
                strcpy(repositoryRoot, "/");
                return 1;
            }

            break;
        }
    }

    repositoryRoot[0] = '\0';

    return 0;
}

const char *gitRepositoryRoot(void)
{
    return repositoryRoot;
}

int gitCurrentBranch(
    char *branch,
    int size
)
{
    if (!gitIsRepository())
        return 0;

    char headPath[PATH_MAX];

    snprintf(
        headPath,
        sizeof(headPath),
        "%s/.git/HEAD",
        repositoryRoot
    );

    FILE *fp =
        fopen(headPath, "r");

    if (!fp)
        return 0;

    char line[256];

    if (!fgets(
            line,
            sizeof(line),
            fp))
    {
        fclose(fp);
        return 0;
    }

    fclose(fp);

    char *lastSlash =
        strrchr(line, '/');

    if (!lastSlash)
        return 0;

    lastSlash++;

    line[
        strcspn(line, "\n")
    ] = '\0';

    snprintf(
        branch,
        size,
        "%s",
        lastSlash
    );

    return 1;
}