#ifndef SESSION_H
#define SESSION_H

#define SESSION_PATH_MAX 1024

typedef struct
{
    char workspaceRoot[SESSION_PATH_MAX];
    char activeFile[SESSION_PATH_MAX];
    int cursorX;
    int cursorY;
    int rowOffset;
    int colOffset;
    int valid;
} EditorSession;

extern EditorSession currentSession;

/* Initialize session subsystem. */
void sessionInit(void);

/* Save current editor session (workspace, active file, cursor, scroll) to file. */
int sessionSave(const char *path);

/* Restore editor session from file. Returns 1 on success, 0 on failure. */
int sessionRestore(const char *path);

/* Automatically save session to workspace .claw or default session location. */
int sessionAutoSave(void);

/* Automatically restore session from workspace .claw or default session location. */
int sessionAutoRestore(void);

#endif
