#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "workspace.h"
#include "recent.h"
#include "session.h"
#include "bookmark.h"
#include "explorer.h"
#include "file_switcher.h"
#include "cursor.h"
#include "viewport.h"
#include "buffer.h"
#include "config.h"

/* Stubs for editor globals needed by session and fileio */
Cursor cursor = {0};
Viewport viewport = {0};
Buffer buffer = {0};
char *currentFile = NULL;

int openFile(const char *path)
{
    (void)path;
    return 1;
}

static void test_workspace_subsystem(void)
{
    printf("[1/6] Testing Workspace Core & Detection...\n");

    workspaceInit();
    assert(workspaceIsActive() == 1);
    assert(workspaceGetRoot() != NULL);
    assert(strlen(workspaceGetName()) > 0);

    /* Detect root from current directory */
    char detected[WORKSPACE_PATH_MAX];
    int ok = workspaceDetectRoot(".", detected, sizeof(detected));
    assert(ok == 1);
    assert(strlen(detected) > 0);

    /* Test workspace close */
    workspaceClose();
    assert(workspaceIsActive() == 0);
    assert(workspaceGetRoot() == NULL);
    assert(strcmp(workspaceGetName(), "None") == 0);

    /* Test workspace open */
    ok = workspaceOpen(".");
    assert(ok == 1);
    assert(workspaceIsActive() == 1);
    assert(strcmp(workspaceGetName(), "None") != 0);

    printf("  -> Workspace Core tests PASSED.\n");
}

static void test_recent_subsystem(void)
{
    printf("[2/6] Testing Recent Tracker (MRU & Persistence)...\n");

    recentTracker.fileCount = 0;
    recentTracker.workspaceCount = 0;

    recentAddFile("src/main.c");
    recentAddFile("include/editor.h");
    recentAddFile("src/buffer/buffer.c");
    assert(recentGetFileCount() == 3);
    assert(strcmp(recentGetFile(0), "src/buffer/buffer.c") == 0);

    /* Re-add already present file: must move to MRU index 0 */
    recentAddFile("src/main.c");
    assert(recentGetFileCount() == 3);
    assert(strcmp(recentGetFile(0), "src/main.c") == 0);
    assert(strcmp(recentGetFile(1), "src/buffer/buffer.c") == 0);
    assert(strcmp(recentGetFile(2), "include/editor.h") == 0);

    /* Test workspace tracking */
    recentAddWorkspace("/home/user/project1");
    recentAddWorkspace("/home/user/project2");
    recentAddWorkspace("/home/user/project1"); /* Move to top */
    assert(recentGetWorkspaceCount() == 2);
    assert(strcmp(recentGetWorkspace(0), "/home/user/project1") == 0);
    assert(strcmp(recentGetWorkspace(1), "/home/user/project2") == 0);

    /* Test persistence save and load */
    const char *testFile = "tests/test_recent_tmp.txt";
    assert(recentSave(testFile) == 1);

    /* Clear and reload */
    recentTracker.fileCount = 0;
    recentTracker.workspaceCount = 0;
    assert(recentLoad(testFile) == 1);
    assert(recentGetFileCount() == 3);
    assert(strcmp(recentGetFile(0), "src/main.c") == 0);
    assert(recentGetWorkspaceCount() == 2);
    assert(strcmp(recentGetWorkspace(0), "/home/user/project1") == 0);

    remove(testFile);
    printf("  -> Recent Tracker tests PASSED.\n");
}

static void test_session_subsystem(void)
{
    printf("[3/6] Testing Session Serialization & Recovery...\n");

    cursor.x = 15;
    cursor.y = 88;
    viewport.rowOffset = 70;
    viewport.colOffset = 0;
    currentFile = "src/editor/editor.c";

    const char *sessFile = "tests/test_session_tmp.claw";
    assert(sessionSave(sessFile) == 1);

    /* Verify file output */
    FILE *fp = fopen(sessFile, "r");
    assert(fp != NULL);
    char buf[512];
    int foundFile = 0, foundY = 0, foundX = 0, foundRowOff = 0;
    while (fgets(buf, sizeof(buf), fp))
    {
        if (strstr(buf, "file=src/editor/editor.c")) foundFile = 1;
        if (strstr(buf, "cursor_x=15")) foundX = 1;
        if (strstr(buf, "cursor_y=88")) foundY = 1;
        if (strstr(buf, "row_offset=70")) foundRowOff = 1;
    }
    fclose(fp);
    assert(foundFile == 1);
    assert(foundX == 1);
    assert(foundY == 1);
    assert(foundRowOff == 1);

    remove(sessFile);
    printf("  -> Session tests PASSED.\n");
}

static void test_bookmark_subsystem(void)
{
    printf("[4/6] Testing Bookmark Navigation System...\n");

    bookmarkInit();
    assert(bookmarkCount() == 0);

    /* Toggle on empty */
    assert(bookmarkToggle(100) == 1);
    assert(bookmarkCount() == 1);
    assert(bookmarkIsBookmarked(100) == 1);
    assert(bookmarkIsBookmarked(50) == 0);

    /* Toggle again removes */
    assert(bookmarkToggle(100) == 0);
    assert(bookmarkCount() == 0);
    assert(bookmarkIsBookmarked(100) == 0);

    /* Add out of order: 50, 10, 80, 25 */
    bookmarkToggle(50);
    bookmarkToggle(10);
    bookmarkToggle(80);
    bookmarkToggle(25);
    assert(bookmarkCount() == 4);

    /* Verify sorted order: 10, 25, 50, 80 */
    assert(bookmarkNext(0) == 10);
    assert(bookmarkNext(10) == 25);
    assert(bookmarkNext(25) == 50);
    assert(bookmarkNext(50) == 80);
    assert(bookmarkNext(80) == 10);  /* Cyclic wrap to start */
    assert(bookmarkNext(100) == 10); /* Wrap to start */

    /* Backward navigation */
    assert(bookmarkPrev(90) == 80);
    assert(bookmarkPrev(80) == 50);
    assert(bookmarkPrev(50) == 25);
    assert(bookmarkPrev(25) == 10);
    assert(bookmarkPrev(10) == 80); /* Cyclic wrap to end */
    assert(bookmarkPrev(5) == 80);  /* Wrap to end */

    /* Clear all */
    bookmarkClearAll();
    assert(bookmarkCount() == 0);
    assert(bookmarkNext(10) == -1);
    assert(bookmarkPrev(10) == -1);

    printf("  -> Bookmark tests PASSED.\n");
}

static void test_file_switcher_subsystem(void)
{
    printf("[5/6] Testing Quick File Switcher Filter...\n");

    /* Populate recent tracker for switcher */
    recentAddFile("src/main.c");
    recentAddFile("src/workspace/workspace.c");
    recentAddFile("include/workspace.h");
    recentAddFile("README.md");

    fileSwitcherOpen();
    assert(fileSwitcherState.active == 1);
    assert(fileSwitcherState.filteredCount > 0);

    /* Search for "workspace" */
    fileSwitcherAddChar('w');
    fileSwitcherAddChar('o');
    fileSwitcherAddChar('r');
    fileSwitcherAddChar('k');
    assert(fileSwitcherState.filteredCount > 0);

    /* Every filtered item must match "work" */
    for (int i = 0; i < fileSwitcherState.filteredCount; i++)
    {
        const char *item = fileSwitcherState.allFiles[fileSwitcherState.filteredIndices[i]];
        assert(strstr(item, "work") != NULL || strstr(item, "Work") != NULL);
    }

    /* Test selection navigation */
    fileSwitcherSelectNext();
    assert(fileSwitcherState.selectedIndex >= 0);
    fileSwitcherSelectPrev();
    assert(fileSwitcherState.selectedIndex == 0);

    /* Backspace and clear */
    fileSwitcherBackspace();
    fileSwitcherBackspace();
    fileSwitcherBackspace();
    fileSwitcherBackspace();
    assert(fileSwitcherState.queryLen == 0);

    fileSwitcherClose();
    assert(fileSwitcherState.active == 0);

    printf("  -> Quick File Switcher tests PASSED.\n");
}

static void test_file_explorer_subsystem(void)
{
    printf("[6/6] Testing Workspace File Explorer...\n");

    explorerOpen(".");
    assert(explorerState.active == 1);
    assert(explorerState.count > 0);

    /* Directories must appear before files */
    int sawFile = 0;
    for (int i = 0; i < explorerState.count; i++)
    {
        if (!explorerState.entries[i].isDir)
        {
            sawFile = 1;
        }
        else
        {
            /* If we see a dir after a file, sort is broken */
            assert(sawFile == 0);
        }
    }

    /* Test navigation */
    explorerSelectNext();
    assert(explorerState.selectedIndex == 1);
    explorerSelectPrev();
    assert(explorerState.selectedIndex == 0);

    const char *sel = explorerGetSelectedPath();
    assert(sel != NULL);
    assert(strlen(sel) > 0);

    explorerClose();
    assert(explorerState.active == 0);

    printf("  -> File Explorer tests PASSED.\n");
}

int main(void)
{
    printf("=========================================\n");
    printf("  Claw Phase 5 Integration Test Suite    \n");
    printf("=========================================\n\n");

    test_workspace_subsystem();
    test_recent_subsystem();
    test_session_subsystem();
    test_bookmark_subsystem();
    test_file_switcher_subsystem();
    test_file_explorer_subsystem();

    printf("\n=========================================\n");
    printf("  ALL PHASE 5 TESTS COMPLETED & PASSED!  \n");
    printf("=========================================\n");
    return 0;
}
