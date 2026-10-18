#ifndef PROJECT_H
#define PROJECT_H

#include <stddef.h>

#define PROJECT_NAME_MAX 128
#define PROJECT_TYPE_MAX 64
#define PROJECT_COMMAND_MAX 512
#define PROJECT_PATH_MAX 1024

typedef struct
{
    char name[PROJECT_NAME_MAX];
    char type[PROJECT_TYPE_MAX];
    char buildCommand[PROJECT_COMMAND_MAX];
    char runCommand[PROJECT_COMMAND_MAX];
    char testCommand[PROJECT_COMMAND_MAX];
    char rootPath[PROJECT_PATH_MAX];
    char configPath[PROJECT_PATH_MAX];
    int active;
} ProjectConfig;

extern ProjectConfig currentProject;

/* Reset project state and discover the active workspace project configuration. */
void projectInit(void);

/* Discover and load .claw/project.conf from a workspace root. */
int projectLoad(const char *workspaceRoot);

/* Execute the configured build command from the project root. */
int projectBuild(void);

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
