#ifndef EDITOR_H
#define EDITOR_H
#define CLAW_VERSION "1.3"
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

/* Manage the editor lifecycle and interactive command loop. */
void editorInit(void);
void editorRun(void);
void editorShutdown(void);
const char *editorStatusMessage(void);
void editorSetStatusMessage(const char *message);

/* Open and close the "Save as" filename prompt. */
void openSavePrompt(void);
void closeSavePrompt(void);

#endif