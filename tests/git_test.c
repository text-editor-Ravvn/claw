#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "git.h"
#include "keymap.h"
#include "input.h"
#include "editor.h"
#include "viewport.h"
#include "buffer_manager.h"

/* Mock global dependencies when testing git subsystem in isolation */
Viewport viewport = {.screenRows = 24, .screenCols = 80};
char *currentFile = NULL;
SavePrompt savePrompt = {0};
GotoLinePrompt gotoPrompt = {0};
GitViewState gitView = {0};

static char testStatusMsg[256] = "";

const char *editorStatusMessage(void)
{
    return testStatusMsg;
}

void editorSetStatusMessage(const char *message)
{
    if (message)
        snprintf(testStatusMsg, sizeof(testStatusMsg), "%s", message);
}

void renderForceClear(void)
{
}

void highlightInit(void)
{
}

int openFile(const char *filename)
{
    (void)filename;
    return 1;
}

int saveFile(const char *filename)
{
    (void)filename;
    return 1;
}

int main(void)
{
    printf("\n==================================================\n");
    printf("     Claw Phase 9 Test Suite (Source Control)     \n");
    printf("==================================================\n");

    /* -------------------------------------------------- */
    /* 1. Repository Detection & Root Resolution          */
    /* -------------------------------------------------- */
    printf("[1/8] Testing Repository Detection & Root Resolution...\n");

    assert(gitIsRepository() == 1);
    const char *root = gitRepositoryRoot();
    assert(root != NULL);
    assert(strlen(root) > 0);

    char branch[128] = "";
    int branchOk = gitCurrentBranch(branch, sizeof(branch));
    assert(branchOk == 1);
    assert(strlen(branch) > 0);
    assert(strcmp(branch, gitBranchName()) == 0);

    printf("  -> Repository root: %s\n", root);
    printf("  -> Current branch: %s\n", branch);
    printf("  -> Detection tests PASSED.\n");

    /* -------------------------------------------------- */
    /* 2. File Tracking & Status Queries                  */
    /* -------------------------------------------------- */
    printf("[2/8] Testing File Tracking & Status Queries...\n");

    /* Makefile should be tracked in this repo */
    assert(gitFileTracked("Makefile") == 1);

    /* A non-existent file should not be tracked */
    assert(gitFileTracked("non_existent_claw_file_9999.xyz") == 0);

    /* Null guards */
    assert(gitFileTracked(NULL) == 0);
    assert(gitFileModified(NULL) == 0);

    /* Refresh status on Makefile */
    gitRefreshStatus("Makefile");
    assert(gitTracked == 1);
    assert(strlen(gitBranch) > 0);

    /* Refresh status on NULL */
    gitRefreshStatus(NULL);
    assert(gitTracked == 0);

    /* Shell argument escaping tests */
    char escBuf[256];
    gitEscapeShellArg("hello world.txt", escBuf, sizeof(escBuf));
    assert(strcmp(escBuf, "'hello world.txt'") == 0);

    gitEscapeShellArg("foo'bar", escBuf, sizeof(escBuf));
    assert(strcmp(escBuf, "'foo'\\''bar'") == 0);

    gitEscapeShellArg("; rm -rf / ;", escBuf, sizeof(escBuf));
    assert(strcmp(escBuf, "'; rm -rf / ;'") == 0);

    gitEscapeShellArg(NULL, escBuf, sizeof(escBuf));
    assert(strcmp(escBuf, "''") == 0);

    /* Path unquoting tests */
    char unescPath[128] = "\"quoted path.txt\"";
    gitUnescapePath(unescPath);
    assert(strcmp(unescPath, "quoted path.txt") == 0);

    char unescQuotes[128] = "\"file\\\"with\\\"quotes.txt\"";
    gitUnescapePath(unescQuotes);
    assert(strcmp(unescQuotes, "file\"with\"quotes.txt") == 0);

    char plainPath[128] = "src/git/git.c";
    gitUnescapePath(plainPath);
    assert(strcmp(plainPath, "src/git/git.c") == 0);

    printf("  -> Shell argument escaping & path unquoting PASSED.\n");
    printf("  -> File tracking & status queries PASSED.\n");

    /* -------------------------------------------------- */
    /* 3. Repository Overview & Status Structure          */
    /* -------------------------------------------------- */
    printf("[3/8] Testing Repository Status Model (gitRefreshRepo)...\n");

    /* Create temporary file with dual staged + unstaged changes (MM status) */
    const char *testDualFile = "claw_test_dual_state.tmp";
    FILE *tf = fopen(testDualFile, "w");
    assert(tf != NULL);
    fputs("line 1\n", tf);
    fclose(tf);

    /* Stage it */
    assert(gitStageFile(testDualFile) == 1);

    /* Now modify it in worktree so it has both staged and unstaged edits */
    tf = fopen(testDualFile, "a");
    assert(tf != NULL);
    fputs("line 2 modified\n", tf);
    fclose(tf);

    GitRepoStatus repoStatus;
    int repoOk = gitRefreshRepo(&repoStatus);
    assert(repoOk == 1);
    assert(repoStatus.isRepo == 1);
    assert(strcmp(repoStatus.root, root) == 0);
    assert(strcmp(repoStatus.branch, branch) == 0);
    assert(repoStatus.stagedCount >= 1);
    assert(repoStatus.unstagedCount >= 1);
    assert(repoStatus.untrackedCount >= 0);
    assert(repoStatus.totalFiles == (repoStatus.stagedCount + repoStatus.unstagedCount + repoStatus.untrackedCount));

    /* Clean up temporary test file */
    gitUnstageFile(testDualFile);
    remove(testDualFile);

    /* Re-verify status model cleanly */
    repoOk = gitRefreshRepo(&repoStatus);
    assert(repoOk == 1);

    /* Null guard */
    assert(gitRefreshRepo(NULL) == 0);

    printf("  -> Found %d total changed files (%d staged, %d unstaged, %d untracked)\n",
           repoStatus.totalFiles, repoStatus.stagedCount, repoStatus.unstagedCount, repoStatus.untrackedCount);
    printf("  -> Dual-state (MM) tracking & status model tests PASSED.\n");

    /* -------------------------------------------------- */
    /* 4. Branch Management API                           */
    /* -------------------------------------------------- */
    printf("[4/8] Testing Branch Management API...\n");

    GitBranchEntry branches[GIT_MAX_BRANCHES];
    int branchCount = 0;
    int listOk = gitListBranches(branches, GIT_MAX_BRANCHES, &branchCount);
    assert(listOk == 1);
    assert(branchCount > 0);

    int foundCurrent = 0;
    for (int i = 0; i < branchCount; i++)
    {
        assert(strlen(branches[i].name) > 0);
        if (branches[i].isCurrent)
        {
            foundCurrent = 1;
            assert(strstr(branches[i].name, branch) != NULL || strcmp(branches[i].name, branch) == 0);
        }
    }
    assert(foundCurrent == 1);

    /* Test branch creation and deletion */
    const char *tempBranchName = "claw-test-temp-branch";
    gitDeleteBranch(tempBranchName, 1);
    int createOk = gitCreateBranch(tempBranchName, 0);
    assert(createOk == 1);

    /* Verify newly created branch appears in branch list */
    int newBranchCount = 0;
    gitListBranches(branches, GIT_MAX_BRANCHES, &newBranchCount);
    int foundNew = 0;
    for (int i = 0; i < newBranchCount; i++)
    {
        if (strcmp(branches[i].name, tempBranchName) == 0)
            foundNew = 1;
    }
    assert(foundNew == 1);

    /* Clean up newly created branch */
    int deleteOk = gitDeleteBranch(tempBranchName, 1);
    assert(deleteOk == 1);

    /* Null guards */
    assert(gitListBranches(NULL, 10, &branchCount) == 0);
    assert(gitSwitchBranch(NULL) == 0);
    assert(gitCreateBranch(NULL, 0) == 0);
    assert(gitDeleteBranch(NULL, 0) == 0);

    printf("  -> Branch list (%d branches), create, and delete PASSED.\n", branchCount);

    /* -------------------------------------------------- */
    /* 5. Stash Management & Commit History Log           */
    /* -------------------------------------------------- */
    printf("[5/8] Testing Stash API & Commit History Log...\n");

    GitStashEntry stashes[GIT_MAX_STASHES];
    int stashCount = 0;
    int stashListOk = gitListStashes(stashes, GIT_MAX_STASHES, &stashCount);
    assert(stashListOk == 1);
    assert(stashCount >= 0);

    /* Commit log queries */
    GitCommitEntry commits[GIT_MAX_COMMITS];
    int commitCount = 0;
    int logOk = gitGetLog(commits, 20, &commitCount);
    assert(logOk == 1);
    assert(commitCount > 0);

    assert(strlen(commits[0].hash) > 0);
    assert(strlen(commits[0].author) > 0);
    assert(strlen(commits[0].date) > 0);
    assert(strlen(commits[0].subject) > 0);

    /* Commit details */
    char detailsBuf[8192];
    int detailsOk = gitGetCommitDetails(commits[0].hash, detailsBuf, sizeof(detailsBuf));
    assert(detailsOk == 1);
    assert(strlen(detailsBuf) > 0);
    assert(strstr(detailsBuf, commits[0].hash) != NULL || strstr(detailsBuf, "commit") != NULL);

    /* Null guards */
    assert(gitGetLog(NULL, 10, &commitCount) == 0);
    assert(gitGetCommitDetails(NULL, detailsBuf, sizeof(detailsBuf)) == 0);
    assert(gitGetCommitDetails(commits[0].hash, NULL, 0) == 0);
    assert(gitListStashes(NULL, 10, &stashCount) == 0);

    printf("  -> Retrieved %d commits. Head commit: [%s] by %s (%s)\n",
           commitCount, commits[0].hash, commits[0].author, commits[0].date);
    printf("  -> Stash and Commit Log tests PASSED.\n");

    /* -------------------------------------------------- */
    /* 6. Diff Engine & Inline Gutter Indicators          */
    /* -------------------------------------------------- */
    printf("[6/8] Testing Diff Engine & Inline Gutter Indicators...\n");

    /* Diff queries */
    char diffBuf[16384];
    int diffOk = gitGetDiffText("Makefile", 0, diffBuf, sizeof(diffBuf));
    assert(diffOk == 1);

    int repoDiffOk = gitGetDiffText(NULL, 0, diffBuf, sizeof(diffBuf));
    assert(repoDiffOk == 1);

    /* Gutter indicator tests */
    gitGutterClear();
    assert(gitGutterGetLineStatus(1) == GIT_GUTTER_NONE);
    assert(gitGutterGetLineStatus(50) == GIT_GUTTER_NONE);

    /* Out of bounds checks */
    assert(gitGutterGetLineStatus(-5) == GIT_GUTTER_NONE);
    assert(gitGutterGetLineStatus(0) == GIT_GUTTER_NONE);
    assert(gitGutterGetLineStatus(GIT_MAX_GUTTER_LINES + 10) == GIT_GUTTER_NONE);

    /* Refresh gutter on Makefile */
    gitGutterRefresh("Makefile");
    /* Clearing should reset */
    gitGutterClear();
    assert(gitGutterGetLineStatus(1) == GIT_GUTTER_NONE);

    printf("  -> Diff text and gutter line status tests PASSED.\n");

    /* -------------------------------------------------- */
    /* 7. Interactive UI View Controllers                 */
    /* -------------------------------------------------- */
    printf("[7/8] Testing Interactive UI View Controllers...\n");

    /* A. Git Dashboard */
    assert(gitDashboardIsActive() == 0);
    gitDashboardOpen();
    assert(gitDashboardIsActive() == 1);

    /* Switch sections: 0 -> 1 -> 2 -> 3 -> 0 */
    gitDashboardHandleKey('\t', 0);
    assert(gitDashboard.selectedSection == 1);
    gitDashboardHandleKey('\t', 0);
    assert(gitDashboard.selectedSection == 2);
    gitDashboardHandleKey('\t', 0);
    assert(gitDashboard.selectedSection == 3);
    gitDashboardHandleKey('\t', 0);
    assert(gitDashboard.selectedSection == 0);

    /* Refresh key */
    gitDashboardHandleKey('r', 0);
    assert(gitDashboard.repo.isRepo == 1);

    /* Test discard key 'x' in dashboard */
    gitDashboardOpen();
    gitDashboardHandleKey('x', 0);
    assert(gitDashboardIsActive() == 1);
    gitDashboardHandleKey('q', 0);
    assert(gitDashboardIsActive() == 0);

    /* B. Git Diff Viewer */
    assert(gitDiffViewerIsActive() == 0);
    gitDiffViewerOpen("Makefile", 0);
    assert(gitDiffViewerIsActive() == 1);
    assert(gitDiffView.staged == 0);

    /* Toggle staged mode */
    gitDiffViewerHandleKey('\t', 0);
    assert(gitDiffView.staged == 1);

    /* Test stage toggle via 's' */
    gitDiffViewerHandleKey('s', 0);
    assert(gitDiffView.staged == 0); /* toggles view to the other mode */

    /* Scroll navigation */
    gitDiffViewerHandleKey(ARROW_DOWN, 0);
    gitDiffViewerHandleKey(ARROW_UP, 0);

    /* Close diff viewer */
    gitDiffViewerHandleKey(27, 0);
    assert(gitDiffViewerIsActive() == 0);

    /* C. Git Log Viewer */
    assert(gitLogViewerIsActive() == 0);
    gitLogViewerOpen();
    assert(gitLogViewerIsActive() == 1);
    assert(gitLogView.commitCount > 0);
    assert(gitLogView.showingDetails == 0);

    /* Open commit details */
    gitLogViewerHandleKey('\r', 0);
    assert(gitLogView.showingDetails == 1);
    assert(strlen(gitLogView.detailsBuffer) > 0);

    /* Return to commit list */
    gitLogViewerHandleKey(27, 0);
    assert(gitLogView.showingDetails == 0);

    /* Close log viewer */
    gitLogViewerHandleKey('q', 0);
    assert(gitLogViewerIsActive() == 0);

    /* D. Git Commit Dialog Modal */
    assert(gitCommitDialogIsActive() == 0);
    gitCommitDialogOpen();
    assert(gitCommitDialogIsActive() == 1);

    /* Whitespace-only commit message should be rejected */
    gitCommitDialogHandleKey(' ', 0);
    gitCommitDialogHandleKey(' ', 0);
    gitCommitDialogHandleKey('\r', 0);
    assert(gitCommitDialogIsActive() == 1); /* must remain open, not committed */
    gitCommitDialogHandleKey(127, 0);
    gitCommitDialogHandleKey(127, 0);

    /* Type commit message */
    gitCommitDialogHandleKey('f', 0);
    gitCommitDialogHandleKey('e', 0);
    gitCommitDialogHandleKey('a', 0);
    gitCommitDialogHandleKey('t', 0);
    assert(strcmp(gitCommitDialog.message, "feat") == 0);
    assert(gitCommitDialog.length == 4);

    /* Backspace */
    gitCommitDialogHandleKey(127, 0);
    assert(strcmp(gitCommitDialog.message, "fea") == 0);
    assert(gitCommitDialog.length == 3);

    /* Esc to cancel */
    gitCommitDialogHandleKey(27, 0);
    assert(gitCommitDialogIsActive() == 0);

    /* E. Git Branch Modal */
    assert(gitBranchModalIsActive() == 0);
    gitBranchModalOpen();
    assert(gitBranchModalIsActive() == 1);
    assert(gitBranchModal.branchCount > 0);

    /* Toggle new branch input */
    gitBranchModalHandleKey('n', 0);
    assert(gitBranchModal.creatingNew == 1);

    /* Filter out illegal git branch characters */
    gitBranchModalHandleKey('~', 0);
    gitBranchModalHandleKey('^', 0);
    gitBranchModalHandleKey(':', 0);
    gitBranchModalHandleKey('?', 0);
    gitBranchModalHandleKey('*', 0);
    assert(gitBranchModal.newBranchLen == 0);

    gitBranchModalHandleKey('m', 0);
    gitBranchModalHandleKey('y', 0);
    assert(strcmp(gitBranchModal.newBranchName, "my") == 0);
    gitBranchModalHandleKey(27, 0);
    assert(gitBranchModal.creatingNew == 0);

    /* Close branch modal */
    gitBranchModalHandleKey(27, 0);
    assert(gitBranchModalIsActive() == 0);

    /* F. Git Stash Modal */
    assert(gitStashModalIsActive() == 0);
    gitStashModalOpen();
    assert(gitStashModalIsActive() == 1);

    /* Toggle new stash input */
    gitStashModalHandleKey('s', 0);
    assert(gitStashModal.savingNew == 1);
    gitStashModalHandleKey(27, 0);
    assert(gitStashModal.savingNew == 0);

    /* Close stash modal */
    gitStashModalHandleKey('q', 0);
    assert(gitStashModalIsActive() == 0);

    printf("  -> All 6 Git View Controllers (Dashboard, Diff, Log, Commit, Branch, Stash) PASSED.\n");

    /* -------------------------------------------------- */
    /* 8. Keymap & Command Decoder Integration            */
    /* -------------------------------------------------- */
    printf("[8/8] Testing Keymap & Command Decoder Integration...\n");

    keymapInit();

    /* Keymap lookup for Phase 9 commands */
    assert(keymapLookup('g', MOD_CTRL | MOD_SHIFT) == CMD_GIT_DASHBOARD);
    assert(keymapLookup('5', MOD_ALT) == CMD_GIT_DASHBOARD);
    assert(keymapLookup('k', MOD_CTRL | MOD_SHIFT) == CMD_GIT_DIFF);
    assert(keymapLookup('l', MOD_CTRL | MOD_SHIFT) == CMD_GIT_LOG);
    assert(keymapLookup('c', MOD_CTRL | MOD_SHIFT) == CMD_GIT_COMMIT);
    assert(keymapLookup('r', MOD_CTRL | MOD_SHIFT) == CMD_GIT_BRANCH);
    assert(keymapLookup('z', MOD_CTRL | MOD_SHIFT) == CMD_GIT_STASH);

    /* Existing git commands continue to work */
    assert(keymapLookup('g', MOD_CTRL) == CMD_GIT_ADD);
    assert(keymapLookup('t', MOD_CTRL) == CMD_GIT_RESTORE);
    assert(keymapLookup('u', MOD_CTRL) == CMD_GIT_UNSTAGE);
    assert(keymapLookup('b', MOD_CTRL) == CMD_GIT_BLAME);

    printf("  -> Phase 9 keymap and command decoder bindings PASSED.\n");

    printf("\n==================================================\n");
    printf("   ALL PHASE 9 TESTS COMPLETED & PASSED!         \n");
    printf("==================================================\n");

    return 0;
}
