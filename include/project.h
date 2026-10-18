#ifndef PROJECT_H
#define PROJECT_H

#include <stddef.h>

#define PROJECT_NAME_MAX 128
#define PROJECT_TYPE_MAX 64
#define PROJECT_COMMAND_MAX 512
#define PROJECT_PATH_MAX 1024
#define PROJECT_TASK_NAME_MAX 64
#define PROJECT_MAX_TASKS 32
#define PROJECT_OUTPUT_MAX 16384
#define PROJECT_DIAGNOSTIC_PATH_MAX 512
#define PROJECT_DIAGNOSTIC_MESSAGE_MAX 512
#define PROJECT_MAX_DIAGNOSTICS 128

typedef struct
{
    char name[PROJECT_TASK_NAME_MAX];
    char command[PROJECT_COMMAND_MAX];
} ProjectTask;

typedef struct
{
    char name[PROJECT_NAME_MAX];
    char type[PROJECT_TYPE_MAX];
    char buildCommand[PROJECT_COMMAND_MAX];
    char runCommand[PROJECT_COMMAND_MAX];
    char testCommand[PROJECT_COMMAND_MAX];
    char rootPath[PROJECT_PATH_MAX];
    char configPath[PROJECT_PATH_MAX];
    ProjectTask tasks[PROJECT_MAX_TASKS];
    size_t taskCount;
    int active;
} ProjectConfig;

extern ProjectConfig currentProject;

typedef struct
{
    char text[PROJECT_OUTPUT_MAX];
    size_t length;
    int active;
    int exitCode;
} ProjectOutput;

extern ProjectOutput projectOutput;

typedef struct
{
    int active;
    size_t selected;
} ProjectDiagnosticsView;

extern ProjectDiagnosticsView projectDiagnosticsView;

typedef struct
{
    int active;
} ProjectDashboardView;

extern ProjectDashboardView projectDashboardView;

typedef enum
{
    PROJECT_DIAGNOSTIC_ERROR,
    PROJECT_DIAGNOSTIC_WARNING,
    PROJECT_DIAGNOSTIC_NOTE
} ProjectDiagnosticSeverity;

typedef struct
{
    char file[PROJECT_DIAGNOSTIC_PATH_MAX];
    int line;
    int column;
    ProjectDiagnosticSeverity severity;
    char message[PROJECT_DIAGNOSTIC_MESSAGE_MAX];
} ProjectDiagnostic;

/* Reset project state and discover the active workspace project configuration. */
void projectInit(void);

/* Discover and load .claw/project.conf from a workspace root. */
int projectLoad(const char *workspaceRoot);

/* Execute the configured build command from the project root. */
int projectBuild(void);

/* Execute a named task from the project root. */
int projectRunTask(const char *name);

/* Open and close the captured output view. */
void projectOutputOpen(void);
void projectOutputClose(void);
int projectOutputIsActive(void);

/* Parse compiler-style diagnostics from the latest captured output. */
void projectDiagnosticsClear(void);
void projectParseDiagnostics(void);
size_t projectDiagnosticCount(void);
const ProjectDiagnostic *projectGetDiagnostic(size_t index);
const ProjectDiagnostic *projectGetSelectedDiagnostic(void);
int projectDiagnosticsNext(void);
int projectDiagnosticsPrevious(void);
void projectDiagnosticsOpen(void);
void projectDiagnosticsClose(void);
int projectDiagnosticsIsActive(void);

void projectDashboardOpen(void);
void projectDashboardClose(void);
int projectDashboardIsActive(void);

/* Inspect explicitly declared task entries. */
size_t projectTaskCount(void);
const ProjectTask *projectGetTask(size_t index);

/* Getters for the active project configuration. */
const char *projectGetName(void);
const char *projectGetType(void);
const char *projectGetBuildCommand(void);
const char *projectGetRunCommand(void);
const char *projectGetTestCommand(void);
const char *projectGetRoot(void);
const char *projectGetConfigPath(void);
int projectIsActive(void);

#endif
