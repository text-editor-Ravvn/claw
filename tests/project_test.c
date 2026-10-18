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
    assert(projectLoad("./missing-project-root") == 0);
    assert(projectIsActive() == 1);
    assert(strcmp(projectGetName(), "claw-test") == 0);
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
    assert(projectDiagnosticCount() == 0);

    projectOutput.length = 0;
    strcpy(projectOutput.text,
           "src/main.c:12:4: error: expected ';'\n"
           "include/value.h:8: warning: implicit declaration\n"
           "build complete\n"
           "src/main.c:14:2: note: declaration is here\n"
           "malformed:line\n");
    projectOutput.length = strlen(projectOutput.text);
    projectParseDiagnostics();
    assert(projectDiagnosticCount() == 3);
    assert(strcmp(projectGetDiagnostic(0)->file, "src/main.c") == 0);
    assert(projectGetDiagnostic(0)->line == 12);
    assert(projectGetDiagnostic(0)->column == 4);
    assert(projectGetDiagnostic(0)->severity == PROJECT_DIAGNOSTIC_ERROR);
    assert(strcmp(projectGetDiagnostic(0)->message, "expected ';'") == 0);
    assert(strcmp(projectGetDiagnostic(1)->file, "include/value.h") == 0);
    assert(projectGetDiagnostic(1)->line == 8);
    assert(projectGetDiagnostic(1)->column == 0);
    assert(projectGetDiagnostic(1)->severity == PROJECT_DIAGNOSTIC_WARNING);
    assert(projectGetDiagnostic(2)->severity == PROJECT_DIAGNOSTIC_NOTE);
    assert(projectGetDiagnostic(3) == NULL);

    projectDiagnosticsClear();
    assert(projectDiagnosticCount() == 0);
    assert(projectDiagnosticsIsActive() == 0);

    projectOutput.length = 0;
    projectOutput.text[0] = '\0';
    for (int i = 0; i < PROJECT_MAX_DIAGNOSTICS + 2; i++)
    {
        size_t remaining = sizeof(projectOutput.text) -
                           projectOutput.length;
        int written = snprintf(projectOutput.text + projectOutput.length,
                               remaining,
                               "src/generated.c:%d: warning: generated\n",
                               i + 1);
        assert(written > 0 && (size_t)written < remaining);
        projectOutput.length += (size_t)written;
    }
    projectParseDiagnostics();
    assert(projectDiagnosticCount() == PROJECT_MAX_DIAGNOSTICS);
    projectDiagnosticsClear();
    projectDiagnosticsOpen();
    assert(projectDiagnosticsIsActive() == 0);

    strcpy(projectOutput.text,
           "src/a.c:1: error: first\n"
           "src/b.c:2: error: second\n"
           "src/c.c:3: warning: third\n");
    projectOutput.length = strlen(projectOutput.text);
    projectParseDiagnostics();
    projectDiagnosticsOpen();
    assert(projectDiagnosticsIsActive() == 1);
    assert(projectGetSelectedDiagnostic()->line == 1);
    assert(projectDiagnosticsNext() == 1);
    assert(projectGetSelectedDiagnostic()->line == 2);
    assert(projectDiagnosticsNext() == 1);
    assert(projectGetSelectedDiagnostic()->line == 3);
    assert(projectDiagnosticsNext() == 1);
    assert(projectGetSelectedDiagnostic()->line == 1);
    assert(projectDiagnosticsPrevious() == 1);
    assert(projectGetSelectedDiagnostic()->line == 3);
    projectDiagnosticsClose();
    assert(projectDiagnosticsIsActive() == 0);

    projectDashboardClose();
    assert(projectDashboardIsActive() == 0);
    projectDashboardOpen();
    assert(projectDashboardIsActive() == 1);
    projectDashboardClose();
    assert(projectDashboardIsActive() == 0);

    remove("./.claw/project.conf");
    projectDashboardOpen();
    projectDiagnosticsOpen();
    projectOutputOpen();
    projectInit();
    assert(projectIsActive() == 0);
    assert(projectBuild() == 0);
    assert(projectDashboardIsActive() == 0);
    assert(projectDiagnosticsIsActive() == 0);
    assert(projectOutputIsActive() == 0);

    printf("project_test passed!\n");
    return 0;
}
