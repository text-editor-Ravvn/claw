#include <stdio.h>
#include <assert.h>
#include <string.h>

#include "workspace.h"

int main(void)
{
    workspaceInit();
    assert(workspaceIsActive() == 1);
    assert(workspaceGetRoot() != NULL);
    assert(workspaceGetName() != NULL);

    /* Test root detection on current directory */
    char detected[WORKSPACE_PATH_MAX];
    int ok = workspaceDetectRoot(".", detected, sizeof(detected));
    assert(ok == 1);
    assert(strlen(detected) > 0);

    /* Test closing workspace */
    workspaceClose();
    assert(workspaceIsActive() == 0);
    assert(workspaceGetRoot() == NULL);
    assert(strcmp(workspaceGetName(), "None") == 0);

    /* Test opening workspace */
    ok = workspaceOpen(".");
    assert(ok == 1);
    assert(workspaceIsActive() == 1);
    assert(workspaceGetRoot() != NULL);

    workspaceClose();
    printf("workspace_test passed!\n");
    return 0;
}
