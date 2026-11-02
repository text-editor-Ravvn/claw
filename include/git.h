/* Claw Source Control Platform & Git Engine Header */
#ifndef GIT_H
#define GIT_H

#include <stddef.h>

#define GIT_MAX_BRANCHES 128
#define GIT_MAX_STASHES 64
#define GIT_MAX_COMMITS 100
#define GIT_MAX_FILES 256
#define GIT_MAX_DIFF_LINES 2048
#define GIT_MAX_GUTTER_LINES 8192

/* Inline Gutter change status */
enum GitGutterStatus
{
    GIT_GUTTER_NONE = 0,
    GIT_GUTTER_ADDED,
    GIT_GUTTER_MODIFIED,
    GIT_GUTTER_DELETED
};

/* Individual changed file entry */
typedef struct
{
    char path[512];
    char statusXY[4]; /* e.g. " M", "M ", "??", "A ", "D " */
    int staged;       /* 1 = staged, 0 = unstaged, -1 = untracked */
    char statusChar;  /* 'M', 'A', 'D', '?', 'R' */
} GitFileEntry;

/* Branch representation */
typedef struct
{
    char name[128];
    int isCurrent;
    int isRemote;
    char upstream[128];
} GitBranchEntry;

/* Stash entry */
typedef struct
{
    int index;
    char name[32];     /* "stash@{0}" */
    char message[256]; /* stash message */
    char date[64];
} GitStashEntry;

/* Commit history entry */
typedef struct
{
    char hash[16];
    char author[64];
    char date[32];
    char subject[160];
} GitCommitEntry;

/* Comprehensive repository overview */
typedef struct
{
    int isRepo;
    char root[512];
    char branch[128];
    char upstream[128];
    int ahead;
    int behind;
    int stagedCount;
    int unstagedCount;
    int untrackedCount;
    int stashCount;
    int totalFiles;
    GitFileEntry files[GIT_MAX_FILES];
} GitRepoStatus;

/* Core global state variables (backward-compatible) */
extern int gitTracked;
extern int gitModified;
extern char gitBranch[128];

/* Core status & detection */
void gitEscapeShellArg(const char *src, char *dst, size_t dstSize);
void gitUnescapePath(char *path);
int gitIsRepository(void);
const char *gitRepositoryRoot(void);
int gitCurrentBranch(char *branch, int size);
const char *gitBranchName(void);
int gitFileModified(const char *filename);
int gitFileTracked(const char *filename);
void gitRefreshStatus(const char *filename);

/* File staging & workspace operations */
int gitAddFile(const char *filename);
int gitRestoreFile(const char *filename);
int gitUnstageFile(const char *filename);
int gitBlameFile(const char *filename, const char *outputFile);

/* Phase 9 Git Platform Additions */
int gitRefreshRepo(GitRepoStatus *outStatus);
int gitStageFile(const char *filename);
int gitStageAll(void);
int gitUnstageAll(void);
int gitDiscardFile(const char *filename);
int gitCommit(const char *message);

/* Branch Management */
int gitListBranches(GitBranchEntry *branches, int maxBranches, int *count);
int gitSwitchBranch(const char *branchName);
int gitCreateBranch(const char *branchName, int checkout);
int gitDeleteBranch(const char *branchName, int force);

/* Stash Management */
int gitListStashes(GitStashEntry *stashes, int maxStashes, int *count);
int gitStashSave(const char *message);
int gitStashPop(int index);
int gitStashApply(int index);
int gitStashDrop(int index);

/* Commit Log & Details */
int gitGetLog(GitCommitEntry *entries, int maxEntries, int *count);
int gitGetCommitDetails(const char *hash, char *detailsBuf, size_t bufSize);

/* Diff Engine */
int gitGetDiffText(const char *filename, int staged, char *diffBuf, size_t bufSize);

/* Inline Gutter Indicators */
void gitGutterRefresh(const char *filename);
int gitGutterGetLineStatus(int line1Indexed);
void gitGutterClear(void);

/* ------------------------------------------------------------- */
/* Interactive UI Subsystems                                     */
/* ------------------------------------------------------------- */

/* 1. Git Dashboard State */
typedef struct
{
    int active;
    int selectedSection; /* 0=Staged, 1=Unstaged, 2=Untracked, 3=Stashes */
    int selectedIndex;
    GitRepoStatus repo;
} GitDashboardState;

extern GitDashboardState gitDashboard;
void gitDashboardOpen(void);
void gitDashboardClose(void);
int gitDashboardIsActive(void);
void gitDashboardHandleKey(int key, int modifiers);
void gitDashboardDraw(void);

/* 2. Git Diff Viewer State */
typedef struct
{
    int active;
    char targetFile[512];
    int staged;
    char *lines[GIT_MAX_DIFF_LINES];
    int lineCount;
    int scrollRow;
} GitDiffViewState;

extern GitDiffViewState gitDiffView;
void gitDiffViewerOpen(const char *filename, int staged);
void gitDiffViewerClose(void);
int gitDiffViewerIsActive(void);
void gitDiffViewerHandleKey(int key, int modifiers);
void gitDiffViewerDraw(void);

/* 3. Git Log Viewer State */
typedef struct
{
    int active;
    GitCommitEntry commits[GIT_MAX_COMMITS];
    int commitCount;
    int selectedIndex;
    int scrollRow;
    int showingDetails;
    char detailsBuffer[8192];
    int detailsScroll;
} GitLogViewState;

extern GitLogViewState gitLogView;
void gitLogViewerOpen(void);
void gitLogViewerClose(void);
int gitLogViewerIsActive(void);
void gitLogViewerHandleKey(int key, int modifiers);
void gitLogViewerDraw(void);

/* 4. Git Commit Dialog Modal State */
typedef struct
{
    int active;
    char message[512];
    int length;
    int stagedCount;
} GitCommitDialogState;

extern GitCommitDialogState gitCommitDialog;
void gitCommitDialogOpen(void);
void gitCommitDialogClose(void);
int gitCommitDialogIsActive(void);
void gitCommitDialogHandleKey(int key, int modifiers);
void gitCommitDialogDraw(void);

/* 5. Git Branch Manager Modal State */
typedef struct
{
    int active;
    GitBranchEntry branches[GIT_MAX_BRANCHES];
    int branchCount;
    int selectedIndex;
    int creatingNew;
    char newBranchName[128];
    int newBranchLen;
} GitBranchModalState;

extern GitBranchModalState gitBranchModal;
void gitBranchModalOpen(void);
void gitBranchModalClose(void);
int gitBranchModalIsActive(void);
void gitBranchModalHandleKey(int key, int modifiers);
void gitBranchModalDraw(void);

/* 6. Git Stash Manager Modal State */
typedef struct
{
    int active;
    GitStashEntry stashes[GIT_MAX_STASHES];
    int stashCount;
    int selectedIndex;
    int savingNew;
    char saveMessage[256];
    int saveMsgLen;
} GitStashModalState;

extern GitStashModalState gitStashModal;
void gitStashModalOpen(void);
void gitStashModalClose(void);
int gitStashModalIsActive(void);
void gitStashModalHandleKey(int key, int modifiers);
void gitStashModalDraw(void);

#endif