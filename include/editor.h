#ifndef EDITOR_H
#define EDITOR_H

#define CLAW_VERSION "1.4"

/* Path of the file currently being edited, or NULL for a new buffer. */
extern char *currentFile;

/* Interactive "Save as" prompt state for unnamed buffers. */
typedef struct
{
    int active;
    char filename[256];
    int length;
} SavePrompt;

extern SavePrompt savePrompt;

/* Temporary Git viewer state (Blame, Diff, etc.) */
typedef struct
{
    int active;
    char originalFile[512];
} GitViewState;

extern GitViewState gitView;

/* Manage the editor lifecycle and interactive command loop. */
void editorInit(void);
void editorRun(void);
void editorShutdown(void);

const char *editorStatusMessage(void);
void editorSetStatusMessage(const char *message);

/* Save prompt helpers */
void openSavePrompt(void);
void closeSavePrompt(void);

/* Temporary Git view helpers */
void openGitView(const char *tempFile);
void closeGitView(void);

#endif