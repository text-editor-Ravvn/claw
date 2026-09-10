/* Future Git status, diff, and commit interfaces. */
#ifndef GIT_H
#define GIT_H

int gitIsRepository(void);

const char *gitRepositoryRoot(void);

int gitCurrentBranch(
    char *branch,
    int size
);

#endif