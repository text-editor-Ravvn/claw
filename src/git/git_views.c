/* Claw Source Control Platform UI Views & Interactive Controllers */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "git.h"
#include "editor.h"
#include "viewport.h"
#include "buffer_manager.h"
#include "input.h"
#include "render.h"

extern Viewport viewport;
extern char *currentFile;

GitDashboardState gitDashboard = {0};
GitDiffViewState gitDiffView = {0};
GitLogViewState gitLogView = {0};
GitCommitDialogState gitCommitDialog = {0};
GitBranchModalState gitBranchModal = {0};
GitStashModalState gitStashModal = {0};

/* ------------------------------------------------------------- */
/* 1. Git Dashboard Controller                                   */
/* ------------------------------------------------------------- */

void gitDashboardOpen(void)
{
    memset(&gitDashboard, 0, sizeof(gitDashboard));
    gitDashboard.active = 1;
    gitDashboard.selectedSection = 0;
    gitDashboard.selectedIndex = 0;
    gitRefreshRepo(&gitDashboard.repo);
    editorSetStatusMessage("Source Control Dashboard");
}

void gitDashboardClose(void)
{
    gitDashboard.active = 0;
    renderForceClear();
}

int gitDashboardIsActive(void)
{
    return gitDashboard.active;
}

static int gitDashboardGetSectionCount(int section)
{
    switch (section)
    {
        case 0: return gitDashboard.repo.stagedCount;
        case 1: return gitDashboard.repo.unstagedCount;
        case 2: return gitDashboard.repo.untrackedCount;
        case 3: return gitDashboard.repo.stashCount;
        default: return 0;
    }
}

static GitFileEntry *gitDashboardGetFileEntry(int section, int index)
{
    int matchIdx = 0;
    for (int i = 0; i < gitDashboard.repo.totalFiles; i++)
    {
        GitFileEntry *f = &gitDashboard.repo.files[i];
        int matches = 0;
        if (section == 0 && f->staged == 1) matches = 1;
        else if (section == 1 && f->staged == 0) matches = 1;
        else if (section == 2 && f->staged == -1) matches = 1;

        if (matches)
        {
            if (matchIdx == index)
                return f;
            matchIdx++;
        }
    }
    return NULL;
}

void gitDashboardHandleKey(int key, int modifiers)
{
    (void)modifiers;

    if (key == 27 || key == 'q')
    {
        gitDashboardClose();
        editorSetStatusMessage("Returned to editor");
        return;
    }

    if (key == '\t' || key == ARROW_RIGHT)
    {
        gitDashboard.selectedSection = (gitDashboard.selectedSection + 1) % 4;
        gitDashboard.selectedIndex = 0;
        return;
    }

    if (key == ARROW_LEFT)
    {
        gitDashboard.selectedSection = (gitDashboard.selectedSection + 3) % 4;
        gitDashboard.selectedIndex = 0;
        return;
    }

    int count = gitDashboardGetSectionCount(gitDashboard.selectedSection);

    if (key == ARROW_UP || key == 'k')
    {
        if (gitDashboard.selectedIndex > 0)
            gitDashboard.selectedIndex--;
        return;
    }

    if (key == ARROW_DOWN || key == 'j')
    {
        if (gitDashboard.selectedIndex < count - 1)
            gitDashboard.selectedIndex++;
        return;
    }

    if (key == 'r')
    {
        gitRefreshRepo(&gitDashboard.repo);
        editorSetStatusMessage("Git status refreshed");
        return;
    }

    /* Staging / Unstaging hotkey */
    if (key == 's')
    {
        if (gitDashboard.selectedSection == 0)
        {
            /* Staged section: unstage file */
            GitFileEntry *entry = gitDashboardGetFileEntry(0, gitDashboard.selectedIndex);
            if (entry)
            {
                gitUnstageFile(entry->path);
                gitRefreshRepo(&gitDashboard.repo);
                gitRefreshStatus(currentFile);
                int cnt = gitDashboardGetSectionCount(0);
                if (gitDashboard.selectedIndex >= cnt)
                    gitDashboard.selectedIndex = (cnt > 0) ? cnt - 1 : 0;
                editorSetStatusMessage("Unstaged file");
            }
        }
        else if (gitDashboard.selectedSection == 1 || gitDashboard.selectedSection == 2)
        {
            /* Unstaged or Untracked: stage file */
            GitFileEntry *entry = gitDashboardGetFileEntry(gitDashboard.selectedSection, gitDashboard.selectedIndex);
            if (entry)
            {
                gitStageFile(entry->path);
                gitRefreshRepo(&gitDashboard.repo);
                gitRefreshStatus(currentFile);
                int cnt = gitDashboardGetSectionCount(gitDashboard.selectedSection);
                if (gitDashboard.selectedIndex >= cnt)
                    gitDashboard.selectedIndex = (cnt > 0) ? cnt - 1 : 0;
                editorSetStatusMessage("Staged file");
            }
        }
        return;
    }

    /* Discard changes on selected unstaged/untracked file */
    if (key == 'x')
    {
        if (gitDashboard.selectedSection == 1)
        {
            GitFileEntry *entry = gitDashboardGetFileEntry(1, gitDashboard.selectedIndex);
            if (entry)
            {
                gitDiscardFile(entry->path);
                gitRefreshRepo(&gitDashboard.repo);
                gitRefreshStatus(currentFile);
                int cnt = gitDashboardGetSectionCount(1);
                if (gitDashboard.selectedIndex >= cnt)
                    gitDashboard.selectedIndex = (cnt > 0) ? cnt - 1 : 0;
                editorSetStatusMessage("Discarded unstaged changes");
            }
        }
        else if (gitDashboard.selectedSection == 2)
        {
            GitFileEntry *entry = gitDashboardGetFileEntry(2, gitDashboard.selectedIndex);
            if (entry)
            {
                remove(entry->path);
                gitRefreshRepo(&gitDashboard.repo);
                gitRefreshStatus(currentFile);
                int cnt = gitDashboardGetSectionCount(2);
                if (gitDashboard.selectedIndex >= cnt)
                    gitDashboard.selectedIndex = (cnt > 0) ? cnt - 1 : 0;
                editorSetStatusMessage("Removed untracked file");
            }
        }
        return;
    }

    /* Stage All */
    if (key == 'S')
    {
        gitStageAll();
        gitRefreshRepo(&gitDashboard.repo);
        gitRefreshStatus(currentFile);
        editorSetStatusMessage("Staged all changes");
        return;
    }

    /* Unstage All */
    if (key == 'U')
    {
        gitUnstageAll();
        gitRefreshRepo(&gitDashboard.repo);
        gitRefreshStatus(currentFile);
        editorSetStatusMessage("Unstaged all changes");
        return;
    }

    /* Commit dialog */
    if (key == 'c')
    {
        gitCommitDialogOpen();
        return;
    }

    /* Diff viewer */
    if (key == 'd')
    {
        GitFileEntry *entry = gitDashboardGetFileEntry(gitDashboard.selectedSection, gitDashboard.selectedIndex);
        if (entry)
            gitDiffViewerOpen(entry->path, (gitDashboard.selectedSection == 0));
        else
            gitDiffViewerOpen(NULL, 0);
        return;
    }

    /* Log viewer */
    if (key == 'l')
    {
        gitLogViewerOpen();
        return;
    }

    /* Branch modal */
    if (key == 'b')
    {
        gitBranchModalOpen();
        return;
    }

    /* Stash modal */
    if (key == 'z')
    {
        gitStashModalOpen();
        return;
    }

    /* Open file in editor */
    if (key == '\r' || key == '\n')
    {
        GitFileEntry *entry = gitDashboardGetFileEntry(gitDashboard.selectedSection, gitDashboard.selectedIndex);
        if (entry)
        {
            char path[512];
            snprintf(path, sizeof(path), "%s", entry->path);
            gitDashboardClose();
            if (bufferManagerOpen(path) >= 0)
            {
                gitRefreshStatus(currentFile);
                editorSetStatusMessage("Opened file from source control");
            }
        }
        return;
    }
}

void gitDashboardDraw(void)
{
    printf("\033[2J\033[H");

    int rows = viewport.screenRows > 0 ? viewport.screenRows : 24;

    printf("\033[1;36m=== Claw Source Control Dashboard ===\033[0m\r\n");

    if (!gitDashboard.repo.isRepo)
    {
        printf("\r\n\033[1;31m [!] Not a git repository. Run 'git init' to initialize source control.\033[0m\r\n");
        printf("\r\n\033[90m----------------------------------------------------------------------\033[0m\r\n");
        printf(" [q / Esc] Close Dashboard\r\n");
        return;
    }

    printf("\033[1;33mBranch:\033[0m \033[1;32m%s\033[0m", gitDashboard.repo.branch[0] ? gitDashboard.repo.branch : "DETACHED");
    if (gitDashboard.repo.upstream[0])
    {
        printf(" -> \033[90m%s\033[0m (\033[32m+%d\033[0m / \033[31m-%d\033[0m)",
               gitDashboard.repo.upstream, gitDashboard.repo.ahead, gitDashboard.repo.behind);
    }
    printf("  |  \033[1;33mRoot:\033[0m \033[90m%s\033[0m\r\n", gitDashboard.repo.root);
    printf("\033[90m======================================================================\033[0m\r\n");

    /* Section Tabs */
    const char *secNames[4] = {"1. Staged", "2. Unstaged", "3. Untracked", "4. Stashes"};
    int secCounts[4] = {
        gitDashboard.repo.stagedCount,
        gitDashboard.repo.unstagedCount,
        gitDashboard.repo.untrackedCount,
        gitDashboard.repo.stashCount
    };

    printf(" ");
    for (int s = 0; s < 4; s++)
    {
        if (s == gitDashboard.selectedSection)
            printf("\033[7;1;36m [%s (%d)] \033[0m ", secNames[s], secCounts[s]);
        else
            printf("\033[90m  %s (%d)  \033[0m ", secNames[s], secCounts[s]);
    }
    printf("\r\n\033[90m----------------------------------------------------------------------\033[0m\r\n");

    /* Display items for the selected section */
    int count = gitDashboardGetSectionCount(gitDashboard.selectedSection);
    if (count == 0)
    {
        printf("  \033[90m(No items in this section)\033[0m\r\n");
    }
    else
    {
        int maxItems = (rows > 12) ? rows - 12 : 4;
        int start = 0;
        if (gitDashboard.selectedIndex >= maxItems)
            start = gitDashboard.selectedIndex - maxItems + 1;

        for (int i = 0; i < maxItems && (start + i) < count; i++)
        {
            int idx = start + i;
            int isSelected = (idx == gitDashboard.selectedIndex);

            if (isSelected)
                printf("\033[7m");

            if (gitDashboard.selectedSection == 3)
            {
                /* Stashes */
                printf(" %c [STASH] stash@{%d}\033[0m\r\n", isSelected ? '>' : ' ', idx);
            }
            else
            {
                GitFileEntry *entry = gitDashboardGetFileEntry(gitDashboard.selectedSection, idx);
                if (entry)
                {
                    char icon = (entry->staged == 1) ? '+' : ((entry->staged == 0) ? '*' : '?');
                    const char *colorCode = (entry->statusChar == 'A' || entry->staged == 1) ? "\033[32m" :
                                            (entry->statusChar == 'D') ? "\033[31m" :
                                            (entry->statusChar == '?') ? "\033[36m" : "\033[33m";

                    printf(" %c %s[%c]\033[0m %-4s %-50.50s\033[0m\r\n",
                           isSelected ? '>' : ' ',
                           colorCode, icon,
                           entry->statusXY,
                           entry->path);
                }
            }

            if (isSelected)
                printf("\033[0m");
        }
    }

    printf("\033[90m----------------------------------------------------------------------\033[0m\r\n");
    printf("\033[1;37mCommands:\033[0m\r\n");
    printf(" \033[1;33m[s]\033[0m Stage/Unstage   \033[1;33m[S]\033[0m Stage All     \033[1;33m[U]\033[0m Unstage All    \033[1;33m[c]\033[0m Commit\r\n");
    printf(" \033[1;33m[d]\033[0m Diff Viewer     \033[1;33m[l]\033[0m Git Log       \033[1;33m[b]\033[0m Branches       \033[1;33m[z]\033[0m Stashes\r\n");
    printf(" \033[1;33m[Enter]\033[0m Open File   \033[1;33m[x]\033[0m Discard       \033[1;33m[Tab]\033[0m Section     \033[1;33m[r]\033[0m Refresh        \033[1;33m[q/Esc]\033[0m Close\r\n");
}

/* ------------------------------------------------------------- */
/* 2. Git Diff Viewer Controller                                 */
/* ------------------------------------------------------------- */

void gitDiffViewerOpen(const char *filename, int staged)
{
    gitDiffViewerClose();

    gitDiffView.active = 1;
    gitDiffView.staged = staged;
    gitDiffView.scrollRow = 0;
    gitDiffView.lineCount = 0;

    if (filename)
        snprintf(gitDiffView.targetFile, sizeof(gitDiffView.targetFile), "%s", filename);
    else
        gitDiffView.targetFile[0] = '\0';

    static char diffBuffer[128 * 1024];
    gitGetDiffText(gitDiffView.targetFile[0] ? gitDiffView.targetFile : NULL,
                   gitDiffView.staged,
                   diffBuffer,
                   sizeof(diffBuffer));

    char *savePtr = NULL;
    char *line = strtok_r(diffBuffer, "\n", &savePtr);
    while (line && gitDiffView.lineCount < GIT_MAX_DIFF_LINES)
    {
        gitDiffView.lines[gitDiffView.lineCount++] = strdup(line);
        line = strtok_r(NULL, "\n", &savePtr);
    }

    editorSetStatusMessage("Git Diff Viewer opened");
}

void gitDiffViewerClose(void)
{
    for (int i = 0; i < gitDiffView.lineCount; i++)
    {
        if (gitDiffView.lines[i])
        {
            free(gitDiffView.lines[i]);
            gitDiffView.lines[i] = NULL;
        }
    }
    gitDiffView.lineCount = 0;
    gitDiffView.active = 0;
    renderForceClear();
}

int gitDiffViewerIsActive(void)
{
    return gitDiffView.active;
}

void gitDiffViewerHandleKey(int key, int modifiers)
{
    (void)modifiers;

    if (key == 27 || key == 'q')
    {
        gitDiffViewerClose();
        editorSetStatusMessage("Returned from diff viewer");
        return;
    }

    int viewRows = viewport.screenRows > 6 ? viewport.screenRows - 6 : 15;

    if (key == ARROW_UP || key == 'k')
    {
        if (gitDiffView.scrollRow > 0)
            gitDiffView.scrollRow--;
        return;
    }

    if (key == ARROW_DOWN || key == 'j')
    {
        if (gitDiffView.scrollRow + viewRows < gitDiffView.lineCount)
            gitDiffView.scrollRow++;
        return;
    }

    if (key == PAGE_UP)
    {
        gitDiffView.scrollRow -= viewRows;
        if (gitDiffView.scrollRow < 0)
            gitDiffView.scrollRow = 0;
        return;
    }

    if (key == PAGE_DOWN)
    {
        gitDiffView.scrollRow += viewRows;
        if (gitDiffView.scrollRow + viewRows > gitDiffView.lineCount)
            gitDiffView.scrollRow = (gitDiffView.lineCount > viewRows) ? gitDiffView.lineCount - viewRows : 0;
        return;
    }

    if (key == HOME_KEY)
    {
        gitDiffView.scrollRow = 0;
        return;
    }

    if (key == END_KEY)
    {
        gitDiffView.scrollRow = (gitDiffView.lineCount > viewRows) ? gitDiffView.lineCount - viewRows : 0;
        return;
    }

    /* Toggle working tree vs staged diff */
    if (key == '\t' || key == 't')
    {
        int newStaged = !gitDiffView.staged;
        char file[512];
        snprintf(file, sizeof(file), "%s", gitDiffView.targetFile);
        gitDiffViewerOpen(file[0] ? file : NULL, newStaged);
        return;
    }

    /* Stage / unstage target file */
    if (key == 's' && gitDiffView.targetFile[0])
    {
        if (gitDiffView.staged)
            gitUnstageFile(gitDiffView.targetFile);
        else
            gitStageFile(gitDiffView.targetFile);

        char file[512];
        snprintf(file, sizeof(file), "%s", gitDiffView.targetFile);
        gitDiffViewerOpen(file, !gitDiffView.staged);
        return;
    }
}

void gitDiffViewerDraw(void)
{
    printf("\033[2J\033[H");

    int rows = viewport.screenRows > 0 ? viewport.screenRows : 24;
    int cols = viewport.screenCols > 0 ? viewport.screenCols : 80;

    printf("\033[1;36m=== Claw Diff Viewer ===\033[0m  ");
    printf("\033[1;33mTarget:\033[0m %s  |  \033[1;33mMode:\033[0m \033[1;%sm[%s]\033[0m  |  \033[90mLines %d/%d\033[0m\r\n",
           gitDiffView.targetFile[0] ? gitDiffView.targetFile : "Repository (All)",
           gitDiffView.staged ? "32" : "33",
           gitDiffView.staged ? "STAGED (INDEX)" : "WORKING TREE",
           gitDiffView.scrollRow + 1, gitDiffView.lineCount);
    printf("\033[90m======================================================================\033[0m\r\n");

    int displayRows = rows - 5;
    if (displayRows < 1) displayRows = 1;

    if (gitDiffView.lineCount == 0)
    {
        printf("  \033[32mNo differences detected (%s).\033[0m\r\n",
               gitDiffView.staged ? "index matches HEAD" : "working tree clean");
    }
    else
    {
        for (int i = 0; i < displayRows; i++)
        {
            int lineIdx = gitDiffView.scrollRow + i;
            if (lineIdx >= gitDiffView.lineCount)
            {
                printf("\033[K\r\n");
                continue;
            }

            const char *l = gitDiffView.lines[lineIdx];
            if (!l)
            {
                printf("\033[K\r\n");
                continue;
            }

            /* Colored unified diff rendering */
            if (strncmp(l, "+++", 3) == 0 || strncmp(l, "---", 3) == 0)
            {
                printf("\033[1;37m%-*.*s\033[0m\r\n", cols, cols, l);
            }
            else if (l[0] == '+')
            {
                printf("\033[32m%-*.*s\033[0m\r\n", cols, cols, l);
            }
            else if (l[0] == '-')
            {
                printf("\033[31m%-*.*s\033[0m\r\n", cols, cols, l);
            }
            else if (strncmp(l, "@@", 2) == 0)
            {
                printf("\033[1;36m%-*.*s\033[0m\r\n", cols, cols, l);
            }
            else if (strncmp(l, "diff --git", 10) == 0 || strncmp(l, "index ", 6) == 0)
            {
                printf("\033[1;35m%-*.*s\033[0m\r\n", cols, cols, l);
            }
            else
            {
                printf("%-*.*s\r\n", cols, cols, l);
            }
        }
    }

    printf("\033[90m----------------------------------------------------------------------\033[0m\r\n");
    printf(" \033[1;33m[j/k, Up/Down]\033[0m Scroll  \033[1;33m[Tab/t]\033[0m Staged/Working  \033[1;33m[s]\033[0m Stage/Unstage  \033[1;33m[q/Esc]\033[0m Exit\r\n");
}

/* ------------------------------------------------------------- */
/* 3. Git Log Viewer Controller                                  */
/* ------------------------------------------------------------- */

void gitLogViewerOpen(void)
{
    memset(&gitLogView, 0, sizeof(gitLogView));
    gitLogView.active = 1;
    gitGetLog(gitLogView.commits, GIT_MAX_COMMITS, &gitLogView.commitCount);
    editorSetStatusMessage("Git Log Viewer opened");
}

void gitLogViewerClose(void)
{
    gitLogView.active = 0;
    renderForceClear();
}

int gitLogViewerIsActive(void)
{
    return gitLogView.active;
}

void gitLogViewerHandleKey(int key, int modifiers)
{
    (void)modifiers;

    if (gitLogView.showingDetails)
    {
        int viewRows = viewport.screenRows > 5 ? viewport.screenRows - 5 : 15;
        if (key == 27 || key == 'q' || key == 127 || key == 8)
        {
            gitLogView.showingDetails = 0;
            return;
        }
        if (key == ARROW_UP || key == 'k')
        {
            if (gitLogView.detailsScroll > 0)
                gitLogView.detailsScroll--;
            return;
        }
        if (key == ARROW_DOWN || key == 'j')
        {
            gitLogView.detailsScroll++;
            return;
        }
        if (key == PAGE_UP)
        {
            gitLogView.detailsScroll -= viewRows;
            if (gitLogView.detailsScroll < 0) gitLogView.detailsScroll = 0;
            return;
        }
        if (key == PAGE_DOWN)
        {
            gitLogView.detailsScroll += viewRows;
            return;
        }
        return;
    }

    if (key == 27 || key == 'q')
    {
        gitLogViewerClose();
        editorSetStatusMessage("Returned from log viewer");
        return;
    }

    if (key == ARROW_UP || key == 'k')
    {
        if (gitLogView.selectedIndex > 0)
            gitLogView.selectedIndex--;
        return;
    }

    if (key == ARROW_DOWN || key == 'j')
    {
        if (gitLogView.selectedIndex < gitLogView.commitCount - 1)
            gitLogView.selectedIndex++;
        return;
    }

    if (key == '\r' || key == '\n' || key == 'd')
    {
        if (gitLogView.commitCount > 0 && gitLogView.selectedIndex < gitLogView.commitCount)
        {
            gitGetCommitDetails(gitLogView.commits[gitLogView.selectedIndex].hash,
                                gitLogView.detailsBuffer,
                                sizeof(gitLogView.detailsBuffer));
            gitLogView.showingDetails = 1;
            gitLogView.detailsScroll = 0;
        }
        return;
    }
}

void gitLogViewerDraw(void)
{
    printf("\033[2J\033[H");

    int rows = viewport.screenRows > 0 ? viewport.screenRows : 24;
    int cols = viewport.screenCols > 0 ? viewport.screenCols : 80;

    if (gitLogView.showingDetails)
    {
        const GitCommitEntry *c = &gitLogView.commits[gitLogView.selectedIndex];
        printf("\033[1;36m=== Commit Details: [%s] ===\033[0m\r\n", c->hash);
        printf("\033[1;33mAuthor:\033[0m %-25s  \033[1;33mDate:\033[0m %s\r\n", c->author, c->date);
        printf("\033[1;37mSubject:\033[0m %s\r\n", c->subject);
        printf("\033[90m======================================================================\033[0m\r\n");

        int displayRows = rows - 6;
        if (displayRows < 1) displayRows = 1;

        /* Print scrolled details lines */
        char copyBuf[8192];
        snprintf(copyBuf, sizeof(copyBuf), "%s", gitLogView.detailsBuffer);

        char *savePtr = NULL;
        char *line = strtok_r(copyBuf, "\n", &savePtr);
        int curLine = 0;
        int printed = 0;

        while (line && printed < displayRows)
        {
            if (curLine >= gitLogView.detailsScroll)
            {
                if (strncmp(line, "+", 1) == 0 && strncmp(line, "+++", 3) != 0)
                    printf("\033[32m%-*.*s\033[0m\r\n", cols, cols, line);
                else if (strncmp(line, "-", 1) == 0 && strncmp(line, "---", 3) != 0)
                    printf("\033[31m%-*.*s\033[0m\r\n", cols, cols, line);
                else if (strncmp(line, "@@", 2) == 0)
                    printf("\033[36m%-*.*s\033[0m\r\n", cols, cols, line);
                else
                    printf("%-*.*s\r\n", cols, cols, line);

                printed++;
            }
            curLine++;
            line = strtok_r(NULL, "\n", &savePtr);
        }

        while (printed < displayRows)
        {
            printf("\033[K\r\n");
            printed++;
        }

        printf("\033[90m----------------------------------------------------------------------\033[0m\r\n");
        printf(" \033[1;33m[j/k, Up/Down]\033[0m Scroll Diff   \033[1;33m[q/Esc/Backspace]\033[0m Return to commit list\r\n");
        return;
    }

    printf("\033[1;36m=== Claw Git Log — Commit History ===\033[0m  \033[90m(%d commits)\033[0m\r\n",
           gitLogView.commitCount);
    printf("\033[90m======================================================================\033[0m\r\n");

    int displayRows = rows - 5;
    if (displayRows < 1) displayRows = 1;

    if (gitLogView.commitCount == 0)
    {
        printf("  \033[90mNo commit history yet (repository has no commits).\033[0m\r\n");
        for (int i = 1; i < displayRows; i++)
            printf("\033[K\r\n");
    }
    else
    {
        int start = 0;
        if (gitLogView.selectedIndex >= displayRows)
            start = gitLogView.selectedIndex - displayRows + 1;

        for (int i = 0; i < displayRows; i++)
        {
            int idx = start + i;
            if (idx >= gitLogView.commitCount)
            {
                printf("\033[K\r\n");
                continue;
            }

            const GitCommitEntry *c = &gitLogView.commits[idx];
            int isSelected = (idx == gitLogView.selectedIndex);

            if (isSelected)
                printf("\033[7m");

            printf(" %c \033[1;33m[%-7.7s]\033[0m \033[36m%-16.16s\033[0m \033[35m%-10.10s\033[0m %-40.40s\r\n",
                   isSelected ? '>' : ' ',
                   c->hash,
                   c->author,
                   c->date,
                   c->subject);

            if (isSelected)
                printf("\033[0m");
        }
    }

    printf("\033[90m----------------------------------------------------------------------\033[0m\r\n");
    printf(" \033[1;33m[Enter / d]\033[0m View Details & Patch   \033[1;33m[j/k, Up/Down]\033[0m Navigate   \033[1;33m[q/Esc]\033[0m Close\r\n");
}

/* ------------------------------------------------------------- */
/* 4. Git Commit Dialog Controller                               */
/* ------------------------------------------------------------- */

void gitCommitDialogOpen(void)
{
    memset(&gitCommitDialog, 0, sizeof(gitCommitDialog));
    gitCommitDialog.active = 1;

    GitRepoStatus st;
    if (gitRefreshRepo(&st))
    {
        gitCommitDialog.stagedCount = st.stagedCount;
    }

    editorSetStatusMessage("Enter commit message");
}

void gitCommitDialogClose(void)
{
    gitCommitDialog.active = 0;
    renderForceClear();
}

int gitCommitDialogIsActive(void)
{
    return gitCommitDialog.active;
}

void gitCommitDialogHandleKey(int key, int modifiers)
{
    (void)modifiers;

    if (key == 27)
    {
        gitCommitDialogClose();
        editorSetStatusMessage("Commit cancelled");
        return;
    }

    if (key == 127 || key == 8)
    {
        if (gitCommitDialog.length > 0)
        {
            gitCommitDialog.message[--gitCommitDialog.length] = '\0';
        }
        return;
    }

    if (key == '\r' || key == '\n')
    {
        int hasNonSpace = 0;
        for (int i = 0; i < gitCommitDialog.length; i++)
        {
            if (!isspace((unsigned char)gitCommitDialog.message[i]))
            {
                hasNonSpace = 1;
                break;
            }
        }

        if (!hasNonSpace)
        {
            editorSetStatusMessage("Commit message cannot be empty");
            return;
        }

        if (gitCommitDialog.stagedCount == 0)
        {
            editorSetStatusMessage("No changes staged to commit (use 's' to stage)");
            gitCommitDialogClose();
            return;
        }

        if (gitCommit(gitCommitDialog.message))
        {
            gitCommitDialogClose();
            gitRefreshStatus(currentFile);
            if (gitDashboardIsActive())
                gitRefreshRepo(&gitDashboard.repo);

            editorSetStatusMessage("Git commit successful!");
        }
        else
        {
            editorSetStatusMessage("Git commit failed");
        }
        return;
    }

    if (key >= 32 && key <= 126)
    {
        if (gitCommitDialog.length < (int)sizeof(gitCommitDialog.message) - 1)
        {
            gitCommitDialog.message[gitCommitDialog.length++] = (char)key;
            gitCommitDialog.message[gitCommitDialog.length] = '\0';
        }
        return;
    }
}

void gitCommitDialogDraw(void)
{
    printf("\033[2J\033[H");
    printf("\r\n\r\n");
    printf("   \033[1;36m+-------------------------------------------------------------+\033[0m\r\n");
    printf("   \033[1;36m|                    Claw Git Commit                          |\033[0m\r\n");
    printf("   \033[1;36m+-------------------------------------------------------------+\033[0m\r\n");
    printf("   |  Staged files: \033[1;%sm%d file(s)\033[0m                                    |\r\n",
           gitCommitDialog.stagedCount > 0 ? "32" : "31",
           gitCommitDialog.stagedCount);
    if (gitCommitDialog.stagedCount == 0)
    {
        printf("   |  \033[31mWarning: No files staged! Stage files first using [s].\033[0m    |\r\n");
    }
    printf("   |                                                             |\r\n");
    printf("   |  Commit Message:                                            |\r\n");
    printf("   |  \033[1;33m> %-54.54s_\033[0m |\r\n", gitCommitDialog.message);
    printf("   |                                                             |\r\n");
    printf("   |  \033[90m[Enter] Confirm Commit         [Esc] Cancel\033[0m                |\r\n");
    printf("   \033[1;36m+-------------------------------------------------------------+\033[0m\r\n");
}

/* ------------------------------------------------------------- */
/* 5. Git Branch Modal Controller                                */
/* ------------------------------------------------------------- */

void gitBranchModalOpen(void)
{
    memset(&gitBranchModal, 0, sizeof(gitBranchModal));
    gitBranchModal.active = 1;
    gitListBranches(gitBranchModal.branches, GIT_MAX_BRANCHES, &gitBranchModal.branchCount);
    editorSetStatusMessage("Branch Management Modal");
}

void gitBranchModalClose(void)
{
    gitBranchModal.active = 0;
    renderForceClear();
}

int gitBranchModalIsActive(void)
{
    return gitBranchModal.active;
}

void gitBranchModalHandleKey(int key, int modifiers)
{
    (void)modifiers;

    if (gitBranchModal.creatingNew)
    {
        if (key == 27)
        {
            gitBranchModal.creatingNew = 0;
            return;
        }

        if (key == 127 || key == 8)
        {
            if (gitBranchModal.newBranchLen > 0)
                gitBranchModal.newBranchName[--gitBranchModal.newBranchLen] = '\0';
            return;
        }

        if (key == '\r' || key == '\n')
        {
            if (gitBranchModal.newBranchLen > 0)
            {
                if (gitCreateBranch(gitBranchModal.newBranchName, 1))
                {
                    gitBranchModalClose();
                    gitRefreshStatus(currentFile);
                    if (gitDashboardIsActive())
                        gitRefreshRepo(&gitDashboard.repo);
                    char buf[256];
                    snprintf(buf, sizeof(buf), "Created and checked out branch: %s", gitBranchModal.newBranchName);
                    editorSetStatusMessage(buf);
                }
                else
                {
                    editorSetStatusMessage("Failed to create branch");
                }
            }
            return;
        }

        if (key >= 32 && key <= 126 && key != ' ' && key != '~' && key != '^' && key != ':' &&
            key != '?' && key != '*' && key != '[' && key != '\\' && key != '@')
        {
            if (gitBranchModal.newBranchLen < (int)sizeof(gitBranchModal.newBranchName) - 1)
            {
                gitBranchModal.newBranchName[gitBranchModal.newBranchLen++] = (char)key;
                gitBranchModal.newBranchName[gitBranchModal.newBranchLen] = '\0';
            }
        }
        return;
    }

    if (key == 27 || key == 'q')
    {
        gitBranchModalClose();
        editorSetStatusMessage("Returned to editor");
        return;
    }

    if (key == ARROW_UP || key == 'k')
    {
        if (gitBranchModal.selectedIndex > 0)
            gitBranchModal.selectedIndex--;
        return;
    }

    if (key == ARROW_DOWN || key == 'j')
    {
        if (gitBranchModal.selectedIndex < gitBranchModal.branchCount - 1)
            gitBranchModal.selectedIndex++;
        return;
    }

    /* Switch to branch */
    if (key == '\r' || key == '\n')
    {
        if (gitBranchModal.branchCount > 0)
        {
            const GitBranchEntry *b = &gitBranchModal.branches[gitBranchModal.selectedIndex];
            if (gitSwitchBranch(b->name))
            {
                gitBranchModalClose();
                gitRefreshStatus(currentFile);
                if (gitDashboardIsActive())
                    gitRefreshRepo(&gitDashboard.repo);
                char buf[256];
                snprintf(buf, sizeof(buf), "Switched to branch: %s", b->name);
                editorSetStatusMessage(buf);
            }
            else
            {
                editorSetStatusMessage("Failed to switch branch");
            }
        }
        return;
    }

    /* Create new branch */
    if (key == 'n' || key == 'c')
    {
        gitBranchModal.creatingNew = 1;
        gitBranchModal.newBranchName[0] = '\0';
        gitBranchModal.newBranchLen = 0;
        return;
    }

    /* Delete branch */
    if (key == 'd')
    {
        if (gitBranchModal.branchCount > 0)
        {
            const GitBranchEntry *b = &gitBranchModal.branches[gitBranchModal.selectedIndex];
            if (b->isCurrent)
            {
                editorSetStatusMessage("Cannot delete currently checked out branch");
                return;
            }
            gitDeleteBranch(b->name, 0);
            gitListBranches(gitBranchModal.branches, GIT_MAX_BRANCHES, &gitBranchModal.branchCount);
            if (gitBranchModal.selectedIndex >= gitBranchModal.branchCount)
                gitBranchModal.selectedIndex = (gitBranchModal.branchCount > 0) ? gitBranchModal.branchCount - 1 : 0;
            editorSetStatusMessage("Deleted branch");
        }
        return;
    }
}

void gitBranchModalDraw(void)
{
    printf("\033[2J\033[H");
    int rows = viewport.screenRows > 0 ? viewport.screenRows : 24;

    printf("\033[1;36m=== Claw Branch Manager ===\033[0m\r\n");
    printf("\033[90m======================================================================\033[0m\r\n");

    if (gitBranchModal.creatingNew)
    {
        printf("\r\n \033[1;33mNew Branch Name:\033[0m %s_\r\n\r\n", gitBranchModal.newBranchName);
        printf("\033[90m----------------------------------------------------------------------\033[0m\r\n");
        printf(" [Enter] Create & Switch  [Esc] Cancel\r\n");
        return;
    }

    int maxDisplay = (rows > 9) ? rows - 9 : 4;
    int start = 0;
    if (gitBranchModal.selectedIndex >= maxDisplay)
        start = gitBranchModal.selectedIndex - maxDisplay + 1;

    for (int i = 0; i < maxDisplay; i++)
    {
        int idx = start + i;
        if (idx >= gitBranchModal.branchCount)
        {
            printf("\033[K\r\n");
            continue;
        }

        const GitBranchEntry *b = &gitBranchModal.branches[idx];
        int isSelected = (idx == gitBranchModal.selectedIndex);

        if (isSelected)
            printf("\033[7m");

        if (b->isCurrent)
        {
            printf(" %c \033[1;32m* %-30.30s\033[0m (current branch)\r\n",
                   isSelected ? '>' : ' ', b->name);
        }
        else if (b->isRemote)
        {
            printf(" %c \033[35m  %-30.30s\033[0m [remote]\r\n",
                   isSelected ? '>' : ' ', b->name);
        }
        else
        {
            printf(" %c   %-30.30s\r\n",
                   isSelected ? '>' : ' ', b->name);
        }

        if (isSelected)
            printf("\033[0m");
    }

    printf("\033[90m----------------------------------------------------------------------\033[0m\r\n");
    printf(" \033[1;33m[Enter]\033[0m Switch Branch  \033[1;33m[n/c]\033[0m New Branch  \033[1;33m[d]\033[0m Delete Branch  \033[1;33m[q/Esc]\033[0m Close\r\n");
}

/* ------------------------------------------------------------- */
/* 6. Git Stash Modal Controller                                 */
/* ------------------------------------------------------------- */

void gitStashModalOpen(void)
{
    memset(&gitStashModal, 0, sizeof(gitStashModal));
    gitStashModal.active = 1;
    gitListStashes(gitStashModal.stashes, GIT_MAX_STASHES, &gitStashModal.stashCount);
    editorSetStatusMessage("Stash Management Modal");
}

void gitStashModalClose(void)
{
    gitStashModal.active = 0;
    renderForceClear();
}

int gitStashModalIsActive(void)
{
    return gitStashModal.active;
}

void gitStashModalHandleKey(int key, int modifiers)
{
    (void)modifiers;

    if (gitStashModal.savingNew)
    {
        if (key == 27)
        {
            gitStashModal.savingNew = 0;
            return;
        }

        if (key == 127 || key == 8)
        {
            if (gitStashModal.saveMsgLen > 0)
                gitStashModal.saveMessage[--gitStashModal.saveMsgLen] = '\0';
            return;
        }

        if (key == '\r' || key == '\n')
        {
            gitStashSave(gitStashModal.saveMessage[0] ? gitStashModal.saveMessage : NULL);
            gitStashModal.savingNew = 0;
            gitListStashes(gitStashModal.stashes, GIT_MAX_STASHES, &gitStashModal.stashCount);
            gitRefreshStatus(currentFile);
            if (gitDashboardIsActive())
                gitRefreshRepo(&gitDashboard.repo);
            editorSetStatusMessage("Saved changes to stash");
            return;
        }

        if (key >= 32 && key <= 126)
        {
            if (gitStashModal.saveMsgLen < (int)sizeof(gitStashModal.saveMessage) - 1)
            {
                gitStashModal.saveMessage[gitStashModal.saveMsgLen++] = (char)key;
                gitStashModal.saveMessage[gitStashModal.saveMsgLen] = '\0';
            }
        }
        return;
    }

    if (key == 27 || key == 'q')
    {
        gitStashModalClose();
        editorSetStatusMessage("Returned to editor");
        return;
    }

    if (key == ARROW_UP || key == 'k')
    {
        if (gitStashModal.selectedIndex > 0)
            gitStashModal.selectedIndex--;
        return;
    }

    if (key == ARROW_DOWN || key == 'j')
    {
        if (gitStashModal.selectedIndex < gitStashModal.stashCount - 1)
            gitStashModal.selectedIndex++;
        return;
    }

    /* Pop stash */
    if (key == 'p')
    {
        if (gitStashModal.stashCount > 0)
        {
            gitStashPop(gitStashModal.selectedIndex);
            gitListStashes(gitStashModal.stashes, GIT_MAX_STASHES, &gitStashModal.stashCount);
            if (gitStashModal.selectedIndex >= gitStashModal.stashCount)
                gitStashModal.selectedIndex = (gitStashModal.stashCount > 0) ? gitStashModal.stashCount - 1 : 0;
            gitRefreshStatus(currentFile);
            if (gitDashboardIsActive())
                gitRefreshRepo(&gitDashboard.repo);
            editorSetStatusMessage("Popped stash");
        }
        return;
    }

    /* Apply stash */
    if (key == 'a')
    {
        if (gitStashModal.stashCount > 0)
        {
            gitStashApply(gitStashModal.selectedIndex);
            gitRefreshStatus(currentFile);
            if (gitDashboardIsActive())
                gitRefreshRepo(&gitDashboard.repo);
            editorSetStatusMessage("Applied stash");
        }
        return;
    }

    /* Drop stash */
    if (key == 'd')
    {
        if (gitStashModal.stashCount > 0)
        {
            gitStashDrop(gitStashModal.selectedIndex);
            gitListStashes(gitStashModal.stashes, GIT_MAX_STASHES, &gitStashModal.stashCount);
            if (gitStashModal.selectedIndex >= gitStashModal.stashCount)
                gitStashModal.selectedIndex = (gitStashModal.stashCount > 0) ? gitStashModal.stashCount - 1 : 0;
            if (gitDashboardIsActive())
                gitRefreshRepo(&gitDashboard.repo);
            editorSetStatusMessage("Dropped stash");
        }
        return;
    }

    /* Save new stash */
    if (key == 's' || key == 'n')
    {
        gitStashModal.savingNew = 1;
        gitStashModal.saveMessage[0] = '\0';
        gitStashModal.saveMsgLen = 0;
        return;
    }
}

void gitStashModalDraw(void)
{
    printf("\033[2J\033[H");
    int rows = viewport.screenRows > 0 ? viewport.screenRows : 24;

    printf("\033[1;36m=== Claw Stash Manager ===\033[0m\r\n");
    printf("\033[90m======================================================================\033[0m\r\n");

    if (gitStashModal.savingNew)
    {
        printf("\r\n \033[1;33mStash Message (optional):\033[0m %s_\r\n\r\n", gitStashModal.saveMessage);
        printf("\033[90m----------------------------------------------------------------------\033[0m\r\n");
        printf(" [Enter] Save Stash  [Esc] Cancel\r\n");
        return;
    }

    if (gitStashModal.stashCount == 0)
    {
        printf("  \033[90mNo stashes found.\033[0m\r\n");
    }
    else
    {
        int maxDisplay = (rows > 9) ? rows - 9 : 4;
        int start = 0;
        if (gitStashModal.selectedIndex >= maxDisplay)
            start = gitStashModal.selectedIndex - maxDisplay + 1;

        for (int i = 0; i < maxDisplay; i++)
        {
            int idx = start + i;
            if (idx >= gitStashModal.stashCount)
            {
                printf("\033[K\r\n");
                continue;
            }

            const GitStashEntry *st = &gitStashModal.stashes[idx];
            int isSelected = (idx == gitStashModal.selectedIndex);

            if (isSelected)
                printf("\033[7m");

            printf(" %c \033[1;33m%-10.10s\033[0m %-40.40s \033[90m(%s)\033[0m\r\n",
                   isSelected ? '>' : ' ',
                   st->name,
                   st->message,
                   st->date);

            if (isSelected)
                printf("\033[0m");
        }
    }

    printf("\033[90m----------------------------------------------------------------------\033[0m\r\n");
    printf(" \033[1;33m[p]\033[0m Pop Stash   \033[1;33m[a]\033[0m Apply Stash   \033[1;33m[d]\033[0m Drop Stash   \033[1;33m[s/n]\033[0m Save Stash   \033[1;33m[q/Esc]\033[0m Close\r\n");
}
