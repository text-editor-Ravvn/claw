#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "terminal.h"
#include "workspace.h"
#include "project.h"
#include "editor.h"

IntegratedTerminal integratedTerminal = {0};

void terminalInit(void)
{
    memset(&integratedTerminal, 0, sizeof(integratedTerminal));
    integratedTerminal.height = 10;
    integratedTerminal.historyIndex = -1;

    if (workspaceIsActive() && workspaceGetRoot())
    {
        snprintf(integratedTerminal.workingDir, sizeof(integratedTerminal.workingDir),
                 "%s", workspaceGetRoot());
    }
    else
    {
        if (!getcwd(integratedTerminal.workingDir, sizeof(integratedTerminal.workingDir)))
        {
            snprintf(integratedTerminal.workingDir, sizeof(integratedTerminal.workingDir), ".");
        }
    }

    terminalAppendOutput("=== Claw Integrated Terminal ===");
    terminalAppendOutput("Commands: help, clear, pwd, cd <dir>, build, test, exit");
    terminalAppendOutput("Press Esc or Ctrl+` to toggle terminal panel.");
}

void terminalToggle(void)
{
    if (integratedTerminal.active)
        terminalClose();
    else
        terminalOpen();
}

void terminalOpen(void)
{
    integratedTerminal.active = 1;
    if (integratedTerminal.workingDir[0] == '\0')
    {
        terminalInit();
        integratedTerminal.active = 1;
    }
    else if (workspaceIsActive() && workspaceGetRoot())
    {
        /* Keep workingDir synced with active workspace */
        snprintf(integratedTerminal.workingDir, sizeof(integratedTerminal.workingDir),
                 "%s", workspaceGetRoot());
    }
    editorSetStatusMessage("Terminal panel opened (Ctrl+` or Esc to close)");
}

void terminalClose(void)
{
    integratedTerminal.active = 0;
    editorSetStatusMessage("Terminal panel closed");
}

int terminalIsActive(void)
{
    return integratedTerminal.active;
}

void terminalClear(void)
{
    integratedTerminal.lineCount = 0;
    integratedTerminal.scrollOffset = 0;
}

void terminalAppendOutput(const char *text)
{
    if (!text)
        return;

    const char *p = text;
    while (*p)
    {
        char lineBuf[TERMINAL_LINE_WIDTH];
        int idx = 0;
        while (*p && *p != '\n' && *p != '\r' && idx < TERMINAL_LINE_WIDTH - 1)
        {
            lineBuf[idx++] = *p++;
        }
        lineBuf[idx] = '\0';

        if (*p == '\r') p++;
        if (*p == '\n') p++;

        if (integratedTerminal.lineCount >= TERMINAL_MAX_LINES)
        {
            /* Shift up */
            memmove(&integratedTerminal.lines[0], &integratedTerminal.lines[1],
                    sizeof(integratedTerminal.lines[0]) * (TERMINAL_MAX_LINES - 1));
            integratedTerminal.lineCount = TERMINAL_MAX_LINES - 1;
        }

        snprintf(integratedTerminal.lines[integratedTerminal.lineCount++],
                 TERMINAL_LINE_WIDTH, "%s", lineBuf);
    }
}

void terminalScroll(int delta)
{
    integratedTerminal.scrollOffset += delta;
    if (integratedTerminal.scrollOffset < 0)
        integratedTerminal.scrollOffset = 0;
    if (integratedTerminal.scrollOffset > integratedTerminal.lineCount - 1)
        integratedTerminal.scrollOffset = integratedTerminal.lineCount - 1;
}

void terminalExecuteCommand(const char *command)
{
    if (!command)
        return;

    /* Trim leading spaces */
    while (*command == ' ' || *command == '\t')
        command++;

    if (*command == '\0')
        return;

    /* Add to history */
    if (integratedTerminal.historyCount == 0 ||
        strcmp(integratedTerminal.history[integratedTerminal.historyCount - 1], command) != 0)
    {
        if (integratedTerminal.historyCount < TERMINAL_HISTORY_MAX)
        {
            snprintf(integratedTerminal.history[integratedTerminal.historyCount++],
                     TERMINAL_CMD_MAX, "%s", command);
        }
    }
    integratedTerminal.historyIndex = -1;

    /* Print command prompt */
    char promptLine[TERMINAL_LINE_WIDTH];
    snprintf(promptLine, sizeof(promptLine), "$ %s", command);
    terminalAppendOutput(promptLine);

    /* Builtins */
    if (strcmp(command, "clear") == 0)
    {
        terminalClear();
        return;
    }
    if (strcmp(command, "exit") == 0 || strcmp(command, "quit") == 0)
    {
        terminalClose();
        return;
    }
    if (strcmp(command, "pwd") == 0)
    {
        terminalAppendOutput(integratedTerminal.workingDir);
        return;
    }
    if (strncmp(command, "cd", 2) == 0 && (command[2] == ' ' || command[2] == '\0'))
    {
        const char *target = command + 2;
        while (*target == ' ' || *target == '\t') target++;
        if (*target == '\0')
        {
            const char *home = getenv("HOME");
            target = home ? home : ".";
        }

        char oldCwd[1024];
        if (getcwd(oldCwd, sizeof(oldCwd)))
        {
            if (chdir(integratedTerminal.workingDir) == 0)
            {
                if (chdir(target) == 0)
                {
                    if (getcwd(integratedTerminal.workingDir, sizeof(integratedTerminal.workingDir)))
                    {
                        char msg[1150];
                        snprintf(msg, sizeof(msg), "Directory changed to %s", integratedTerminal.workingDir);
                        terminalAppendOutput(msg);
                    }
                }
                else
                {
                    terminalAppendOutput("cd: directory not found");
                }
            }
            chdir(oldCwd);
        }
        return;
    }
    if (strcmp(command, "help") == 0)
    {
        terminalAppendOutput("Claw Builtin Commands:");
        terminalAppendOutput("  pwd            Show current working directory");
        terminalAppendOutput("  cd <path>      Change working directory");
        terminalAppendOutput("  build          Run workspace build command");
        terminalAppendOutput("  test           Run workspace test suite");
        terminalAppendOutput("  clear          Clear terminal scrollback");
        terminalAppendOutput("  exit           Close terminal panel");
        terminalAppendOutput("Standard shell commands (ls, git, gcc, etc.) are executed directly.");
        return;
    }
    if (strcmp(command, "build") == 0)
    {
        terminalAppendOutput("Executing project build...");
        int ok = projectBuild();
        if (projectOutput.length > 0)
            terminalAppendOutput(projectOutput.text);
        terminalAppendOutput(ok ? "[Build finished successfully]" : "[Build failed]");
        return;
    }
    if (strcmp(command, "test") == 0)
    {
        terminalAppendOutput("Executing project tests...");
        int ok = projectRunTask("test");
        if (projectOutput.length > 0)
            terminalAppendOutput(projectOutput.text);
        terminalAppendOutput(ok ? "[Tests passed]" : "[Tests failed]");
        return;
    }

    /* Execute via shell in working directory */
    char execCmd[1536];
    snprintf(execCmd, sizeof(execCmd), "cd \"%s\" && %s 2>&1",
             integratedTerminal.workingDir, command);

    FILE *pipe = popen(execCmd, "r");
    if (!pipe)
    {
        terminalAppendOutput("Failed to execute command.");
        return;
    }

    char buffer[512];
    while (fgets(buffer, sizeof(buffer), pipe))
    {
        terminalAppendOutput(buffer);
    }
    int status = pclose(pipe);
    if (status != 0)
    {
        char exitMsg[64];
        snprintf(exitMsg, sizeof(exitMsg), "[Process exited with status %d]", status);
        terminalAppendOutput(exitMsg);
    }

    integratedTerminal.scrollOffset = 0;
}

int terminalHandleKey(KeyEvent event)
{
    int key = event.key;

    if (key == 27 || (key == '`' && (event.modifiers & MOD_CTRL)))
    {
        terminalClose();
        return 1;
    }

    /* Cancel current input on Ctrl+C */
    if ((key == 'c' && (event.modifiers & MOD_CTRL)) || key == 3)
    {
        integratedTerminal.currentCommand[0] = '\0';
        integratedTerminal.commandLen = 0;
        integratedTerminal.cursorCol = 0;
        return 1;
    }

    /* Clear scrollback on Ctrl+L */
    if ((key == 'l' && (event.modifiers & MOD_CTRL)) || key == 12)
    {
        terminalClear();
        return 1;
    }

    /* Scroll terminal output on Page Up / Page Down */
    if (key == PAGE_UP)
    {
        terminalScroll(5);
        return 1;
    }
    if (key == PAGE_DOWN)
    {
        terminalScroll(-5);
        return 1;
    }

    if (key == '\r' || key == '\n')
    {
        terminalExecuteCommand(integratedTerminal.currentCommand);
        integratedTerminal.currentCommand[0] = '\0';
        integratedTerminal.commandLen = 0;
        integratedTerminal.cursorCol = 0;
        return 1;
    }

    if (key == 127 || key == 8)
    {
        if (integratedTerminal.cursorCol > 0)
        {
            memmove(&integratedTerminal.currentCommand[integratedTerminal.cursorCol - 1],
                    &integratedTerminal.currentCommand[integratedTerminal.cursorCol],
                    (size_t)(integratedTerminal.commandLen - integratedTerminal.cursorCol + 1));
            integratedTerminal.cursorCol--;
            integratedTerminal.commandLen--;
        }
        return 1;
    }

    if (key == DELETE_KEY)
    {
        if (integratedTerminal.cursorCol < integratedTerminal.commandLen)
        {
            memmove(&integratedTerminal.currentCommand[integratedTerminal.cursorCol],
                    &integratedTerminal.currentCommand[integratedTerminal.cursorCol + 1],
                    (size_t)(integratedTerminal.commandLen - integratedTerminal.cursorCol));
            integratedTerminal.commandLen--;
        }
        return 1;
    }

    if (key == ARROW_LEFT)
    {
        if (integratedTerminal.cursorCol > 0)
            integratedTerminal.cursorCol--;
        return 1;
    }

    if (key == ARROW_RIGHT)
    {
        if (integratedTerminal.cursorCol < integratedTerminal.commandLen)
            integratedTerminal.cursorCol++;
        return 1;
    }

    if (key == HOME_KEY)
    {
        integratedTerminal.cursorCol = 0;
        return 1;
    }

    if (key == END_KEY)
    {
        integratedTerminal.cursorCol = integratedTerminal.commandLen;
        return 1;
    }

    if (key == ARROW_UP)
    {
        if (integratedTerminal.historyCount > 0)
        {
            if (integratedTerminal.historyIndex == -1)
                integratedTerminal.historyIndex = integratedTerminal.historyCount - 1;
            else if (integratedTerminal.historyIndex > 0)
                integratedTerminal.historyIndex--;

            snprintf(integratedTerminal.currentCommand, sizeof(integratedTerminal.currentCommand),
                     "%s", integratedTerminal.history[integratedTerminal.historyIndex]);
            integratedTerminal.commandLen = (int)strlen(integratedTerminal.currentCommand);
            integratedTerminal.cursorCol = integratedTerminal.commandLen;
        }
        return 1;
    }

    if (key == ARROW_DOWN)
    {
        if (integratedTerminal.historyIndex >= 0)
        {
            if (integratedTerminal.historyIndex < integratedTerminal.historyCount - 1)
            {
                integratedTerminal.historyIndex++;
                snprintf(integratedTerminal.currentCommand, sizeof(integratedTerminal.currentCommand),
                         "%s", integratedTerminal.history[integratedTerminal.historyIndex]);
            }
            else
            {
                integratedTerminal.historyIndex = -1;
                integratedTerminal.currentCommand[0] = '\0';
            }
            integratedTerminal.commandLen = (int)strlen(integratedTerminal.currentCommand);
            integratedTerminal.cursorCol = integratedTerminal.commandLen;
        }
        return 1;
    }

    if (key >= 32 && key <= 126)
    {
        if (integratedTerminal.commandLen < TERMINAL_CMD_MAX - 1)
        {
            memmove(&integratedTerminal.currentCommand[integratedTerminal.cursorCol + 1],
                    &integratedTerminal.currentCommand[integratedTerminal.cursorCol],
                    (size_t)(integratedTerminal.commandLen - integratedTerminal.cursorCol + 1));
            integratedTerminal.currentCommand[integratedTerminal.cursorCol] = (char)key;
            integratedTerminal.cursorCol++;
            integratedTerminal.commandLen++;
        }
        return 1;
    }

    return 1;
}
