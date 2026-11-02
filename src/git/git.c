/* Claw Source Control Platform & Git Engine Implementation */

#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <limits.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <ctype.h>

#include "git.h"

static char repositoryRoot[PATH_MAX] = "";
static char currentBranch[128] = "";

int gitTracked = 0;
int gitModified = 0;
char gitBranch[128] = "";

/* Gutter change indicators table */
static char gutterStatus[GIT_MAX_GUTTER_LINES] = {0};

/* Shell argument escaping helper: safely wraps in single-quotes and escapes inner single-quotes */
void gitEscapeShellArg(const char *src, char *dst, size_t dstSize)
{
    if (!dst || dstSize < 3)
    {
        if (dst && dstSize > 0) dst[0] = '\0';
        return;
    }

    if (!src)
    {
        snprintf(dst, dstSize, "''");
        return;
    }

    size_t outIdx = 0;
    dst[outIdx++] = '\'';

    for (size_t i = 0; src[i] != '\0' && outIdx + 5 < dstSize; i++)
    {
        if (src[i] == '\'')
        {
            dst[outIdx++] = '\'';
            dst[outIdx++] = '\\';
            dst[outIdx++] = '\'';
            dst[outIdx++] = '\'';
        }
        else
        {
            dst[outIdx++] = src[i];
        }
    }

    if (outIdx < dstSize - 1)
    {
        dst[outIdx++] = '\'';
        dst[outIdx] = '\0';
    }
    else
    {
        dst[dstSize - 1] = '\0';
    }
}

/* Helper to strip surrounding quotes and escape characters from git porcelain paths */
void gitUnescapePath(char *path)
{
    if (!path) return;
    size_t len = strlen(path);
    if (len >= 2 && path[0] == '"' && path[len - 1] == '"')
    {
        size_t r = 1, w = 0;
        while (r < len - 1)
        {
            if (path[r] == '\\' && r + 1 < len - 1)
            {
                r++;
                if (path[r] == '"') path[w++] = '"';
                else if (path[r] == '\\') path[w++] = '\\';
                else if (path[r] == 'n') path[w++] = '\n';
                else if (path[r] == 't') path[w++] = '\t';
                else path[w++] = path[r];
            }
            else
            {
                path[w++] = path[r];
            }
            r++;
        }
        path[w] = '\0';
    }
}

/* Helper to strip trailing newline/carriage return */
static void stripNewline(char *str)
{
    if (!str) return;
    size_t len = strlen(str);
    while (len > 0 && (str[len - 1] == '\r' || str[len - 1] == '\n'))
    {
        str[--len] = '\0';
    }
}

int gitIsRepository(void)
{
    char current[PATH_MAX];

    if (!getcwd(current, sizeof(current)))
        return 0;

    while (1)
    {
        char gitPath[PATH_MAX + 32];

        snprintf(gitPath, sizeof(gitPath), "%s/.git", current);

        struct stat st;
        if (stat(gitPath, &st) == 0)
        {
            if (S_ISDIR(st.st_mode) || S_ISREG(st.st_mode))
            {
                snprintf(repositoryRoot, sizeof(repositoryRoot), "%s", current);
                return 1;
            }
        }

        char *slash = strrchr(current, '/');
#ifdef _WIN32
        char *bslash = strrchr(current, '\\');
        if (bslash && (!slash || bslash > slash))
            slash = bslash;
#endif

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

        if (strcmp(current, "/") == 0 || (strlen(current) == 2 && current[1] == ':'))
        {
            snprintf(gitPath, sizeof(gitPath), "%s/.git", current);
            if (stat(gitPath, &st) == 0 && (S_ISDIR(st.st_mode) || S_ISREG(st.st_mode)))
            {
                snprintf(repositoryRoot, sizeof(repositoryRoot), "%s", current);
                return 1;
            }
            break;
        }
    }

    /* Fallback to git rev-parse */
    FILE *fp = popen("git rev-parse --show-toplevel 2>/dev/null", "r");
    if (fp)
    {
        if (fgets(repositoryRoot, sizeof(repositoryRoot), fp))
        {
            stripNewline(repositoryRoot);
            pclose(fp);
            if (strlen(repositoryRoot) > 0)
                return 1;
        }
        else
        {
            pclose(fp);
        }
    }

    repositoryRoot[0] = '\0';
    return 0;
}

const char *gitRepositoryRoot(void)
{
    return repositoryRoot;
}

int gitCurrentBranch(char *branch, int size)
{
    if (!gitIsRepository())
        return 0;

    char headPath[PATH_MAX + 64];
    struct stat st;
    char gitPath[PATH_MAX + 32];
    snprintf(gitPath, sizeof(gitPath), "%s/.git", repositoryRoot);

    if (stat(gitPath, &st) == 0 && S_ISREG(st.st_mode))
    {
        /* Submodule or git worktree where .git is a file pointing to gitdir */
        headPath[0] = '\0';
        FILE *gfp = fopen(gitPath, "r");
        if (gfp)
        {
            char gline[PATH_MAX];
            if (fgets(gline, sizeof(gline), gfp))
            {
                stripNewline(gline);
                if (strncmp(gline, "gitdir:", 7) == 0)
                {
                    char *d = gline + 7;
                    while (*d == ' ' || *d == '\t') d++;
                    if (d[0] == '/')
                        snprintf(headPath, sizeof(headPath), "%s/HEAD", d);
                    else
                        snprintf(headPath, sizeof(headPath), "%s/%s/HEAD", repositoryRoot, d);
                }
            }
            fclose(gfp);
        }
        if (headPath[0] == '\0')
            snprintf(headPath, sizeof(headPath), "%s/.git/HEAD", repositoryRoot);
    }
    else
    {
        snprintf(headPath, sizeof(headPath), "%s/.git/HEAD", repositoryRoot);
    }

    FILE *fp = fopen(headPath, "r");
    if (fp)
    {
        char line[256];
        if (fgets(line, sizeof(line), fp))
        {
            fclose(fp);
            stripNewline(line);

            if (strncmp(line, "ref: refs/heads/", 16) == 0)
            {
                snprintf(currentBranch, sizeof(currentBranch), "%s", line + 16);
                if (branch && size > 0)
                    snprintf(branch, (size_t)size, "%s", currentBranch);
                return 1;
            }

            char *lastSlash = strrchr(line, '/');
            if (lastSlash)
            {
                lastSlash++;
                snprintf(currentBranch, sizeof(currentBranch), "%s", lastSlash);
                if (branch && size > 0)
                    snprintf(branch, (size_t)size, "%s", currentBranch);
                return 1;
            }

            /* Detached HEAD: line contains commit SHA */
            size_t slen = strlen(line);
            if (slen >= 7)
            {
                int isHex = 1;
                for (size_t i = 0; i < 7; i++)
                {
                    if (!isxdigit((unsigned char)line[i])) { isHex = 0; break; }
                }
                if (isHex)
                {
                    snprintf(currentBranch, sizeof(currentBranch), "detached (%.7s)", line);
                    if (branch && size > 0)
                        snprintf(branch, (size_t)size, "%s", currentBranch);
                    return 1;
                }
            }
        }
        else
        {
            fclose(fp);
        }
    }

    /* Fallback to git rev-parse */
    FILE *pfp = popen("git rev-parse --abbrev-ref HEAD 2>/dev/null", "r");
    if (pfp)
    {
        if (fgets(currentBranch, sizeof(currentBranch), pfp))
        {
            stripNewline(currentBranch);
            pclose(pfp);
            if (strlen(currentBranch) > 0)
            {
                if (branch && size > 0)
                    snprintf(branch, (size_t)size, "%s", currentBranch);
                return 1;
            }
        }
        else
        {
            pclose(pfp);
        }
    }

    return 0;
}

const char *gitBranchName(void)
{
    return currentBranch;
}

int gitFileTracked(const char *filename)
{
    if (!filename || !gitIsRepository())
        return 0;

    char escaped[1024];
    gitEscapeShellArg(filename, escaped, sizeof(escaped));
    char command[1200];
    snprintf(command, sizeof(command), "git ls-files --error-unmatch -- %s > /dev/null 2>&1", escaped);
    return system(command) == 0;
}

int gitFileModified(const char *filename)
{
    if (!filename || !gitIsRepository())
        return 0;

    char escaped[1024];
    gitEscapeShellArg(filename, escaped, sizeof(escaped));
    char command[1200];
    snprintf(command, sizeof(command), "git diff --quiet -- %s", escaped);
    return system(command) != 0;
}

void gitRefreshStatus(const char *filename)
{
    gitTracked = 0;
    gitModified = 0;
    gitBranch[0] = '\0';

    if (!gitCurrentBranch(gitBranch, sizeof(gitBranch)))
    {
        gitGutterClear();
        return;
    }

    if (!filename)
    {
        gitGutterClear();
        return;
    }

    gitTracked = gitFileTracked(filename);
    gitModified = gitFileModified(filename);
    gitGutterRefresh(filename);
}

int gitAddFile(const char *filename)
{
    if (!filename)
        return 0;

    char escaped[1024];
    gitEscapeShellArg(filename, escaped, sizeof(escaped));
    char command[1200];
    snprintf(command, sizeof(command), "git add -- %s > /dev/null 2>&1", escaped);
    return system(command) == 0;
}

int gitRestoreFile(const char *filename)
{
    if (!filename)
        return 0;

    char escaped[1024];
    gitEscapeShellArg(filename, escaped, sizeof(escaped));
    char command[1200];
    snprintf(command, sizeof(command), "git restore -- %s > /dev/null 2>&1", escaped);
    return system(command) == 0;
}

int gitUnstageFile(const char *filename)
{
    if (!filename || !gitIsRepository())
        return 0;

    char escaped[1024];
    gitEscapeShellArg(filename, escaped, sizeof(escaped));
    char command[1200];
    snprintf(command, sizeof(command), "git restore --staged -- %s > /dev/null 2>&1", escaped);
    return system(command) == 0;
}

int gitBlameFile(const char *filename, const char *outputFile)
{
    if (!filename || !outputFile || !gitIsRepository())
        return 0;

    char escFile[1024];
    char escOut[1024];
    gitEscapeShellArg(filename, escFile, sizeof(escFile));
    gitEscapeShellArg(outputFile, escOut, sizeof(escOut));
    char command[2200];
    snprintf(command, sizeof(command), "git blame -- %s > %s 2>/dev/null", escFile, escOut);
    return system(command) == 0;
}

/* ------------------------------------------------------------- */
/* Phase 9 Additions                                             */
/* ------------------------------------------------------------- */

int gitRefreshRepo(GitRepoStatus *outStatus)
{
    if (!outStatus)
        return 0;

    memset(outStatus, 0, sizeof(GitRepoStatus));
    outStatus->isRepo = gitIsRepository();
    if (!outStatus->isRepo)
        return 0;

    strncpy(outStatus->root, repositoryRoot, sizeof(outStatus->root) - 1);
    outStatus->root[sizeof(outStatus->root) - 1] = '\0';
    gitCurrentBranch(outStatus->branch, sizeof(outStatus->branch));

    /* Check upstream and ahead/behind count */
    FILE *upFp = popen("git rev-parse --abbrev-ref --symbolic-full-name @{u} 2>/dev/null", "r");
    if (upFp)
    {
        if (fgets(outStatus->upstream, sizeof(outStatus->upstream), upFp))
        {
            stripNewline(outStatus->upstream);
        }
        pclose(upFp);
    }

    if (outStatus->upstream[0])
    {
        FILE *cntFp = popen("git rev-list --left-right --count HEAD...@{u} 2>/dev/null", "r");
        if (cntFp)
        {
            int ah = 0, bh = 0;
            if (fscanf(cntFp, "%d %d", &ah, &bh) == 2)
            {
                outStatus->ahead = ah;
                outStatus->behind = bh;
            }
            pclose(cntFp);
        }
    }

    /* Stash count */
    FILE *stFp = popen("git stash list 2>/dev/null", "r");
    if (stFp)
    {
        char sLine[256];
        while (fgets(sLine, sizeof(sLine), stFp))
        {
            outStatus->stashCount++;
        }
        pclose(stFp);
    }

    /* Changed files via git -c core.quotepath=false status --porcelain=v1 -u */
    FILE *statFp = popen("git -c core.quotepath=false status --porcelain=v1 -u 2>/dev/null", "r");
    if (statFp)
    {
        char line[1024];
        while (fgets(line, sizeof(line), statFp) && outStatus->totalFiles < GIT_MAX_FILES)
        {
            stripNewline(line);
            if (strlen(line) < 4)
                continue;

            char x = line[0];
            char y = line[1];

            char *pathPtr = line + 3;
            /* In case of rename "R  old -> new", take the destination or full string */
            char *arrow = strstr(pathPtr, " -> ");
            if (arrow)
            {
                pathPtr = arrow + 4;
            }

            char cleanPath[512];
            snprintf(cleanPath, sizeof(cleanPath), "%s", pathPtr);
            gitUnescapePath(cleanPath);

            if (x == '?' && y == '?')
            {
                /* Untracked file */
                GitFileEntry *entry = &outStatus->files[outStatus->totalFiles++];
                entry->statusXY[0] = '?';
                entry->statusXY[1] = '?';
                entry->statusXY[2] = '\0';
                entry->staged = -1;
                entry->statusChar = '?';
                snprintf(entry->path, sizeof(entry->path), "%s", cleanPath);
                outStatus->untrackedCount++;
            }
            else if (x == 'U' || y == 'U' || (x == 'A' && y == 'A') || (x == 'D' && y == 'D'))
            {
                /* Merge conflict / unmerged file: categorize in working tree (unstaged) */
                GitFileEntry *entry = &outStatus->files[outStatus->totalFiles++];
                entry->statusXY[0] = x;
                entry->statusXY[1] = y;
                entry->statusXY[2] = '\0';
                entry->staged = 0;
                entry->statusChar = 'U';
                snprintf(entry->path, sizeof(entry->path), "%s", cleanPath);
                outStatus->unstagedCount++;
            }
            else
            {
                /* Check if there are staged changes */
                if (x != ' ' && x != '?' && outStatus->totalFiles < GIT_MAX_FILES)
                {
                    GitFileEntry *entry = &outStatus->files[outStatus->totalFiles++];
                    entry->statusXY[0] = x;
                    entry->statusXY[1] = ' ';
                    entry->statusXY[2] = '\0';
                    entry->staged = 1;
                    entry->statusChar = x;
                    snprintf(entry->path, sizeof(entry->path), "%s", cleanPath);
                    outStatus->stagedCount++;
                }

                /* Check if there are ALSO unstaged working-tree changes (e.g. MM, AM, MD) */
                if (y != ' ' && y != '?' && outStatus->totalFiles < GIT_MAX_FILES)
                {
                    GitFileEntry *entry = &outStatus->files[outStatus->totalFiles++];
                    entry->statusXY[0] = ' ';
                    entry->statusXY[1] = y;
                    entry->statusXY[2] = '\0';
                    entry->staged = 0;
                    entry->statusChar = y;
                    snprintf(entry->path, sizeof(entry->path), "%s", cleanPath);
                    outStatus->unstagedCount++;
                }
            }
        }
        pclose(statFp);
    }

    return 1;
}

int gitStageFile(const char *filename)
{
    return gitAddFile(filename);
}

int gitStageAll(void)
{
    if (!gitIsRepository())
        return 0;

    return system("git add -A > /dev/null 2>&1") == 0;
}

int gitUnstageAll(void)
{
    if (!gitIsRepository())
        return 0;

    return system("git restore --staged . > /dev/null 2>&1") == 0;
}

int gitDiscardFile(const char *filename)
{
    return gitRestoreFile(filename);
}

int gitCommit(const char *message)
{
    if (!message || strlen(message) == 0 || !gitIsRepository())
        return 0;

    char tmpPath[PATH_MAX + 64];
    if (repositoryRoot[0])
        snprintf(tmpPath, sizeof(tmpPath), "%s/.git/CLAW_COMMIT_MSG", repositoryRoot);
    else
        snprintf(tmpPath, sizeof(tmpPath), ".claw_commit.tmp");

    FILE *tmp = fopen(tmpPath, "w");
    if (!tmp)
        return 0;

    fprintf(tmp, "%s\n", message);
    fclose(tmp);

    char escPath[PATH_MAX + 128];
    gitEscapeShellArg(tmpPath, escPath, sizeof(escPath));
    char cmd[PATH_MAX + 256];
    snprintf(cmd, sizeof(cmd), "git commit -F %s > /dev/null 2>&1", escPath);

    int res = system(cmd);
    remove(tmpPath);
    return (res == 0);
}

int gitListBranches(GitBranchEntry *branches, int maxBranches, int *count)
{
    if (!branches || maxBranches <= 0 || !count)
        return 0;

    *count = 0;
    if (!gitIsRepository())
        return 0;

    FILE *fp = popen("git branch -a --format=\"%(HEAD)%(refname:short)|%(upstream:short)\" 2>/dev/null", "r");
    if (!fp)
        return 0;

    char line[256];
    while (fgets(line, sizeof(line), fp) && *count < maxBranches)
    {
        stripNewline(line);
        if (strlen(line) == 0)
            continue;

        GitBranchEntry *b = &branches[*count];
        memset(b, 0, sizeof(GitBranchEntry));

        char *ptr = line;
        while (*ptr == ' ' || *ptr == '\t') ptr++;
        if (*ptr == '*')
        {
            b->isCurrent = 1;
            ptr++;
            while (*ptr == ' ' || *ptr == '\t') ptr++;
        }
        else
        {
            b->isCurrent = 0;
        }
        while (*ptr == ' ' || *ptr == '\t') ptr++;

        char *pipePos = strchr(ptr, '|');
        if (pipePos)
        {
            *pipePos = '\0';
            strncpy(b->name, ptr, sizeof(b->name) - 1);
            b->name[sizeof(b->name) - 1] = '\0';
            strncpy(b->upstream, pipePos + 1, sizeof(b->upstream) - 1);
            b->upstream[sizeof(b->upstream) - 1] = '\0';
        }
        else
        {
            strncpy(b->name, ptr, sizeof(b->name) - 1);
            b->name[sizeof(b->name) - 1] = '\0';
        }

        if (strncmp(b->name, "origin/", 7) == 0 || strncmp(b->name, "remotes/", 8) == 0)
        {
            b->isRemote = 1;
        }

        (*count)++;
    }

    pclose(fp);
    return 1;
}

int gitSwitchBranch(const char *branchName)
{
    if (!branchName || !gitIsRepository())
        return 0;

    char escaped[384];
    gitEscapeShellArg(branchName, escaped, sizeof(escaped));
    char command[512];
    snprintf(command, sizeof(command), "git checkout %s > /dev/null 2>&1", escaped);
    return system(command) == 0;
}

int gitCreateBranch(const char *branchName, int checkout)
{
    if (!branchName || !gitIsRepository())
        return 0;

    char escaped[384];
    gitEscapeShellArg(branchName, escaped, sizeof(escaped));
    char command[512];
    if (checkout)
        snprintf(command, sizeof(command), "git checkout -b %s > /dev/null 2>&1", escaped);
    else
        snprintf(command, sizeof(command), "git branch %s > /dev/null 2>&1", escaped);

    return system(command) == 0;
}

int gitDeleteBranch(const char *branchName, int force)
{
    if (!branchName || !gitIsRepository())
        return 0;

    char escaped[384];
    gitEscapeShellArg(branchName, escaped, sizeof(escaped));
    char command[512];
    snprintf(command, sizeof(command), "git branch %s %s > /dev/null 2>&1", force ? "-D" : "-d", escaped);
    return system(command) == 0;
}

int gitListStashes(GitStashEntry *stashes, int maxStashes, int *count)
{
    if (!stashes || maxStashes <= 0 || !count)
        return 0;

    *count = 0;
    if (!gitIsRepository())
        return 0;

    FILE *fp = popen("git stash list --format=\"%gd|%gs|%cr\" 2>/dev/null", "r");
    if (!fp)
        return 0;

    char line[512];
    while (fgets(line, sizeof(line), fp) && *count < maxStashes)
    {
        stripNewline(line);
        if (strlen(line) == 0)
            continue;

        GitStashEntry *st = &stashes[*count];
        memset(st, 0, sizeof(GitStashEntry));
        st->index = *count;

        char *firstPipe = strchr(line, '|');
        if (firstPipe)
        {
            *firstPipe = '\0';
            strncpy(st->name, line, sizeof(st->name) - 1);
            st->name[sizeof(st->name) - 1] = '\0';

            char *secondPipe = strchr(firstPipe + 1, '|');
            if (secondPipe)
            {
                *secondPipe = '\0';
                strncpy(st->message, firstPipe + 1, sizeof(st->message) - 1);
                st->message[sizeof(st->message) - 1] = '\0';
                strncpy(st->date, secondPipe + 1, sizeof(st->date) - 1);
                st->date[sizeof(st->date) - 1] = '\0';
            }
            else
            {
                strncpy(st->message, firstPipe + 1, sizeof(st->message) - 1);
                st->message[sizeof(st->message) - 1] = '\0';
            }
        }
        else
        {
            strncpy(st->name, line, sizeof(st->name) - 1);
            st->name[sizeof(st->name) - 1] = '\0';
        }

        (*count)++;
    }

    pclose(fp);
    return 1;
}

int gitStashSave(const char *message)
{
    if (!gitIsRepository())
        return 0;

    char command[1200];
    if (message && strlen(message) > 0)
    {
        char escaped[1024];
        gitEscapeShellArg(message, escaped, sizeof(escaped));
        snprintf(command, sizeof(command), "git stash push -m %s > /dev/null 2>&1", escaped);
    }
    else
    {
        snprintf(command, sizeof(command), "git stash push > /dev/null 2>&1");
    }

    return system(command) == 0;
}

int gitStashPop(int index)
{
    if (!gitIsRepository() || index < 0)
        return 0;

    char command[256];
    snprintf(command, sizeof(command), "git stash pop \"stash@{%d}\" > /dev/null 2>&1", index);
    return system(command) == 0;
}

int gitStashApply(int index)
{
    if (!gitIsRepository() || index < 0)
        return 0;

    char command[256];
    snprintf(command, sizeof(command), "git stash apply \"stash@{%d}\" > /dev/null 2>&1", index);
    return system(command) == 0;
}

int gitStashDrop(int index)
{
    if (!gitIsRepository() || index < 0)
        return 0;

    char command[256];
    snprintf(command, sizeof(command), "git stash drop \"stash@{%d}\" > /dev/null 2>&1", index);
    return system(command) == 0;
}

int gitGetLog(GitCommitEntry *entries, int maxEntries, int *count)
{
    if (!entries || maxEntries <= 0 || !count)
        return 0;

    *count = 0;
    if (!gitIsRepository())
        return 0;

    char cmd[256];
    snprintf(cmd, sizeof(cmd), "git log -n %d --format=\"%%h|%%an|%%ad|%%s\" --date=short 2>/dev/null", maxEntries);
    FILE *fp = popen(cmd, "r");
    if (!fp)
        return 0;

    char line[512];
    while (fgets(line, sizeof(line), fp) && *count < maxEntries)
    {
        stripNewline(line);
        if (strlen(line) == 0)
            continue;

        GitCommitEntry *c = &entries[*count];
        memset(c, 0, sizeof(GitCommitEntry));

        char *p1 = strchr(line, '|');
        if (p1)
        {
            *p1 = '\0';
            strncpy(c->hash, line, sizeof(c->hash) - 1);
            c->hash[sizeof(c->hash) - 1] = '\0';

            char *p2 = strchr(p1 + 1, '|');
            if (p2)
            {
                *p2 = '\0';
                strncpy(c->author, p1 + 1, sizeof(c->author) - 1);
                c->author[sizeof(c->author) - 1] = '\0';

                char *p3 = strchr(p2 + 1, '|');
                if (p3)
                {
                    *p3 = '\0';
                    strncpy(c->date, p2 + 1, sizeof(c->date) - 1);
                    c->date[sizeof(c->date) - 1] = '\0';
                    strncpy(c->subject, p3 + 1, sizeof(c->subject) - 1);
                    c->subject[sizeof(c->subject) - 1] = '\0';
                }
                else
                {
                    strncpy(c->date, p2 + 1, sizeof(c->date) - 1);
                    c->date[sizeof(c->date) - 1] = '\0';
                }
            }
            else
            {
                strncpy(c->author, p1 + 1, sizeof(c->author) - 1);
                c->author[sizeof(c->author) - 1] = '\0';
            }
        }
        else
        {
            strncpy(c->hash, line, sizeof(c->hash) - 1);
            c->hash[sizeof(c->hash) - 1] = '\0';
        }

        (*count)++;
    }

    pclose(fp);
    return 1;
}

int gitGetCommitDetails(const char *hash, char *detailsBuf, size_t bufSize)
{
    if (!hash || !detailsBuf || bufSize == 0 || !gitIsRepository())
        return 0;

    detailsBuf[0] = '\0';
    char escHash[256];
    gitEscapeShellArg(hash, escHash, sizeof(escHash));
    char cmd[512];
    snprintf(cmd, sizeof(cmd), "git show --stat -p %s 2>/dev/null", escHash);

    FILE *fp = popen(cmd, "r");
    if (!fp)
        return 0;

    size_t total = 0;
    char line[512];
    while (fgets(line, sizeof(line), fp) && total + sizeof(line) < bufSize)
    {
        size_t len = strlen(line);
        memcpy(detailsBuf + total, line, len);
        total += len;
        detailsBuf[total] = '\0';
    }

    pclose(fp);
    return 1;
}

int gitGetDiffText(const char *filename, int staged, char *diffBuf, size_t bufSize)
{
    if (!diffBuf || bufSize == 0 || !gitIsRepository())
        return 0;

    diffBuf[0] = '\0';
    char cmd[1200];

    if (filename && strlen(filename) > 0)
    {
        char escFile[1024];
        gitEscapeShellArg(filename, escFile, sizeof(escFile));
        snprintf(cmd, sizeof(cmd), "git diff %s -- %s 2>/dev/null",
                 staged ? "--cached" : "", escFile);
    }
    else
    {
        snprintf(cmd, sizeof(cmd), "git diff %s 2>/dev/null",
                 staged ? "--cached" : "");
    }

    FILE *fp = popen(cmd, "r");
    if (!fp)
        return 0;

    size_t total = 0;
    char line[512];
    while (fgets(line, sizeof(line), fp) && total + sizeof(line) < bufSize)
    {
        size_t len = strlen(line);
        memcpy(diffBuf + total, line, len);
        total += len;
        diffBuf[total] = '\0';
    }

    pclose(fp);
    return 1;
}

/* ------------------------------------------------------------- */
/* Inline Gutter Indicators Implementation                       */
/* ------------------------------------------------------------- */

void gitGutterClear(void)
{
    memset(gutterStatus, GIT_GUTTER_NONE, sizeof(gutterStatus));
}

void gitGutterRefresh(const char *filename)
{
    gitGutterClear();
    if (!filename || !gitIsRepository())
        return;

    char escFile[1024];
    gitEscapeShellArg(filename, escFile, sizeof(escFile));
    char cmd[1200];
    snprintf(cmd, sizeof(cmd), "git diff -U0 -- %s 2>/dev/null", escFile);

    FILE *fp = popen(cmd, "r");
    if (!fp)
        return;

    char line[512];
    while (fgets(line, sizeof(line), fp))
    {
        if (strncmp(line, "@@ -", 4) == 0)
        {
            int oldStart = 0, oldCount = 1, newStart = 0, newCount = 1;

            if (sscanf(line, "@@ -%d,%d +%d,%d @@", &oldStart, &oldCount, &newStart, &newCount) == 4)
            {
                /* standard hunk */
            }
            else if (sscanf(line, "@@ -%d,%d +%d @@", &oldStart, &oldCount, &newStart) == 3)
            {
                newCount = 1;
            }
            else if (sscanf(line, "@@ -%d +%d,%d @@", &oldStart, &newStart, &newCount) == 3)
            {
                oldCount = 1;
            }
            else if (sscanf(line, "@@ -%d +%d @@", &oldStart, &newStart) == 2)
            {
                oldCount = 1;
                newCount = 1;
            }
            else
            {
                continue;
            }

            if (oldCount == 0 && newCount > 0)
            {
                /* Pure added lines */
                for (int l = newStart; l < newStart + newCount && l < GIT_MAX_GUTTER_LINES; l++)
                {
                    if (l > 0)
                        gutterStatus[l] = GIT_GUTTER_ADDED;
                }
            }
            else if (newCount == 0 && oldCount > 0)
            {
                /* Deleted lines: if deleted at beginning of file (newStart == 0), mark line 1 */
                int targetLine = (newStart == 0) ? 1 : newStart;
                if (targetLine > 0 && targetLine < GIT_MAX_GUTTER_LINES)
                {
                    gutterStatus[targetLine] = GIT_GUTTER_DELETED;
                }
            }
            else
            {
                /* Modified lines */
                for (int l = newStart; l < newStart + newCount && l < GIT_MAX_GUTTER_LINES; l++)
                {
                    if (l > 0)
                        gutterStatus[l] = GIT_GUTTER_MODIFIED;
                }
            }
        }
    }

    pclose(fp);
}

int gitGutterGetLineStatus(int line1Indexed)
{
    if (line1Indexed <= 0 || line1Indexed >= GIT_MAX_GUTTER_LINES)
        return GIT_GUTTER_NONE;

    return (int)gutterStatus[line1Indexed];
}