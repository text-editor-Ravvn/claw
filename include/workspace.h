#ifndef WORKSPACE_H
#define WORKSPACE_H

#include <stddef.h>

#define WORKSPACE_PATH_MAX 1024
#define WORKSPACE_NAME_MAX 256

typedef struct
{
    char rootPath[WORKSPACE_PATH_MAX];
    char name[WORKSPACE_NAME_MAX];
    char configPath[WORKSPACE_PATH_MAX];
    int active;
} Workspace;

extern Workspace currentWorkspace;

/* Initialize workspace subsystem and auto-detect workspace for current working directory. */
void workspaceInit(void);

/* Open or switch to a workspace directory. Returns 1 on success, 0 on failure. */
int workspaceOpen(const char *path);

/* Close the active workspace. */
void workspaceClose(void);

/* Detect workspace root by walking upward looking for .claw, .git, or fallback to current dir. */
int workspaceDetectRoot(const char *startPath, char *outRoot, size_t outSize);

/* Get root path of the active workspace, or NULL if inactive. */
const char *workspaceGetRoot(void);

/* Get display name of active workspace. */
const char *workspaceGetName(void);

/* Check if a workspace is active. */
int workspaceIsActive(void);

/* Load workspace-specific configuration (.claw/workspace.conf). Returns 1 on success, 0 otherwise. */
int workspaceLoadConfig(void);

#endif
