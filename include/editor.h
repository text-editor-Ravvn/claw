#ifndef EDITOR_H
#define EDITOR_H
#define CLAW_VERSION "0.6"
/* Path of the file currently being edited, or NULL for a new buffer. */
extern char *currentFile;

/* Manage the editor lifecycle and interactive command loop. */
void editorInit(void);
void editorRun(void);
void editorShutdown(void);
const char *editorStatusMessage(void);
void editorSetStatusMessage(const char *message);

#endif