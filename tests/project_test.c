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
          "test = echo project-test-task\n"
          "task.check = echo task-check-test\n"
#ifdef _WIN32
          "task.fail = cmd /c exit 7\n"
#else
          "task.fail = false\n"
#endif
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
    assert(strcmp(projectGetTestCommand(), "echo project-test-task") == 0);
    assert(projectTaskCount() == 2);
    assert(strcmp(projectGetTask(0)->name, "check") == 0);
    assert(strcmp(projectGetTask(0)->command, "echo task-check-test") == 0);
    assert(strcmp(projectGetRoot(), root) == 0);
    assert(strcmp(projectGetConfigPath(), "./.claw/project.conf") == 0);
    assert(projectBuild() == 1);
    assert(projectOutput.exitCode == 0);
    assert(strstr(projectOutput.text, "project-build-test") != NULL);
    projectOutputOpen();
    assert(projectOutputIsActive() == 1);
    projectOutputClose();
    assert(projectOutputIsActive() == 0);
    assert(projectRunTask("check") == 1);
    assert(strstr(projectOutput.text, "task-check-test") != NULL);
    assert(projectRunTask("test") == 1);
    assert(strstr(projectOutput.text, "project-test-task") != NULL);
    assert(projectRunTask("fail") == 0);
    assert(projectOutput.length > 0);
    assert(projectOutput.length < PROJECT_OUTPUT_MAX);
    assert(projectRunTask("missing") == 0);
    assert(projectOutput.exitCode != 0);

    remove("./.claw/project.conf");
    projectInit();
    assert(projectIsActive() == 0);
    assert(projectBuild() == 0);

    printf("project_test passed!\n");
    return 0;
}
