/* Future Git status, diff, and commit interfaces. */
#ifndef GIT_H
#define GIT_H

extern int gitTracked;
extern int gitModified;
extern char gitBranch[128];

int gitIsRepository(void);

const char *gitRepositoryRoot(void);

int gitCurrentBranch(
    char *branch,
    int size
);
const char *gitBranchName(void);
int gitFileModified(const char *filename);
int gitFileTracked(const char *filename);
void gitRefreshStatus(const char *filename);
int gitAddFile(const char *filename);
int gitRestoreFile(const char *filename);
int gitUnstageFile(const char *filename);
int gitBlameFile(const char *filename,const char *outputFile);

#endif