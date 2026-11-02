#ifndef TERMINAL_H
#define TERMINAL_H

#include "input.h"

#define TERMINAL_MAX_LINES 500
#define TERMINAL_LINE_WIDTH 512
#define TERMINAL_HISTORY_MAX 64
#define TERMINAL_CMD_MAX 256

typedef struct
{
    int active;
    int height;
    char currentCommand[TERMINAL_CMD_MAX];
    int commandLen;
    int cursorCol;
    char history[TERMINAL_HISTORY_MAX][TERMINAL_CMD_MAX];
    int historyCount;
    int historyIndex;
    char lines[TERMINAL_MAX_LINES][TERMINAL_LINE_WIDTH];
    int lineCount;
    int scrollOffset;
    char workingDir[1024];
} IntegratedTerminal;

extern IntegratedTerminal integratedTerminal;

/* Initialize terminal subsystem */
void terminalInit(void);

/* Toggle terminal panel visibility */
void terminalToggle(void);

/* Open integrated terminal panel */
void terminalOpen(void);

/* Close integrated terminal panel */
void terminalClose(void);

/* Check if integrated terminal panel is open */
int terminalIsActive(void);

/* Append a line of text to terminal output buffer */
void terminalAppendOutput(const char *text);

/* Clear terminal output */
void terminalClear(void);

/* Execute a shell command inside the terminal environment */
void terminalExecuteCommand(const char *command);

/* Handle keyboard input while terminal has focus */
int terminalHandleKey(KeyEvent event);

/* Scroll terminal output */
void terminalScroll(int delta);

#endif
