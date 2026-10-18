#include <assert.h>
#include <stdio.h>
#include <string.h>

#include "project.h"

int workspaceIsActive(void)
{
    return 0;
}

const char *workspaceGetRoot(void)
{
    return NULL;
}

static void writeProjectConfig(const char *root)
{
    char path[PROJECT_PATH_MAX];
    FILE *file;

    snprintf(path, sizeof(path), "%s/.claw/project.conf", root);
    file = fopen(path, "w");
    assert(file != NULL);
    fputs("# Project metadata\n"
          "name = claw-test\n"
          "type = c\n"
          "build = echo project-build-test\n"
          "run = ./claw\n"
          "test = make test\n"
          "unknown = ignored\n", file);
    assert(fclose(file) == 0);
}

int main(void)
{
    const char *root = ".";

    projectInit();
    assert(projectIsActive() == 0);
    assert(projectGetName() == NULL);

    writeProjectConfig(root);
    assert(projectLoad(root) == 1);
    assert(projectIsActive() == 1);
    assert(strcmp(projectGetName(), "claw-test") == 0);
    assert(strcmp(projectGetType(), "c") == 0);
    assert(strcmp(projectGetBuildCommand(), "echo project-build-test") == 0);
    assert(strcmp(projectGetRunCommand(), "./claw") == 0);
    assert(strcmp(projectGetTestCommand(), "make test") == 0);
    assert(strcmp(projectGetRoot(), root) == 0);
    assert(strcmp(projectGetConfigPath(), "./.claw/project.conf") == 0);
    assert(projectBuild() == 1);

    remove("./.claw/project.conf");
    projectInit();
    assert(projectIsActive() == 0);
    assert(projectBuild() == 0);

    printf("project_test passed!\n");
    return 0;
}
