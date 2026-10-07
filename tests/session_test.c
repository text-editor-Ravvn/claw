#include <stdio.h>
#include <assert.h>
#include <string.h>

#include "recent.h"
#include "session.h"
#include "workspace.h"

int main(void)
{
    /* Test recent tracker */
    recentTracker.fileCount = 0;
    recentTracker.workspaceCount = 0;

    recentAddFile("fileA.txt");
    recentAddFile("fileB.txt");
    recentAddFile("fileA.txt"); /* MRU deduplication */

    assert(recentGetFileCount() == 2);
    assert(strcmp(recentGetFile(0), "fileA.txt") == 0);
    assert(strcmp(recentGetFile(1), "fileB.txt") == 0);

    recentAddWorkspace("/path/one");
    recentAddWorkspace("/path/two");
    assert(recentGetWorkspaceCount() == 2);
    assert(strcmp(recentGetWorkspace(0), "/path/two") == 0);

    /* Test session serialization */
    int ok = sessionSave("tests/test_session.tmp");
    assert(ok == 1);

    /* Verify file exists and has content */
    FILE *fp = fopen("tests/test_session.tmp", "r");
    assert(fp != NULL);
    char buf[128];
    assert(fgets(buf, sizeof(buf), fp) != NULL);
    fclose(fp);
    remove("tests/test_session.tmp");

    printf("session_test passed!\n");
    return 0;
}
