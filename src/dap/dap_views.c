/* Claw Debug Adapter Protocol (DAP) Interactive Views (Phase 12) */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "dap.h"
#include "editor.h"
#include "viewport.h"
#include "render.h"
#include "buffer.h"
#include "cursor.h"
#include "buffer_manager.h"

extern Buffer buffer;
extern Cursor cursor;
extern char *currentFile;

DapDashboardState dapDashboard = {0};

void dapDashboardOpen(void)
{
    dapDashboard.active = 1;
    dapDashboard.selectedSection = 0;
    dapDashboard.selectedIndex = 0;
    dapDashboard.scrollOffset = 0;
    editorSetStatusMessage("");
}

void dapDashboardClose(void)
{
    dapDashboard.active = 0;
    renderForceClear();
}

int dapDashboardIsActive(void)
{
    return dapDashboard.active;
}

void dapDashboardDraw(void)
{
    if (!dapDashboard.active)
        return;

    printf("\033[2J\033[H");
    int state = dapGetState();
    const char *stateStr = (state == DAP_STATE_STOPPED) ? "STOPPED" :
                           (state == DAP_STATE_RUNNING) ? "RUNNING" :
                           (state == DAP_STATE_PAUSED) ? "PAUSED" : "INITIALIZING";

    printf("\033[1;34m=== Claw Debugger Dashboard (DAP) ===\033[0m   Status: \033[1;%sm[%s]\033[0m",
           (state == DAP_STATE_PAUSED) ? "33" : (state == DAP_STATE_RUNNING) ? "32" : "31",
           stateStr);

    if (state == DAP_STATE_PAUSED)
    {
        printf("   Stopped at \033[1;37m%s:%d\033[0m (%s)",
               dapCurrentFile(), dapCurrentLine(), dapStopReasonString());
    }
    printf("\r\n");
    printf("\033[90m--------------------------------------------------------------------------------\033[0m\r\n");

    /* Section Tabs */
    printf(" [1] Controls   [2] Call Stack   [3] Variables   [4] Breakpoints\r\n");
    printf("\033[90m================================================================================\033[0m\r\n");

    /* Controls & Shortcuts */
    printf("\033[1;36m>> Execution Controls:\033[0m\r\n");
    printf("   [F5 / c]   Continue / Run           [F10 / n]  Step Over (Next)\r\n");
    printf("   [F6 / p]   Pause Execution          [F11 / s]  Step Into\r\n");
    printf("   [Shift+F5] Stop Debug Session       [Shift+F11] Step Out\r\n");
    printf("   [F9 / b]   Toggle Line Breakpoint   [r]        Restart Session\r\n");
    printf("\r\n");

    /* Call Stack View */
    DapStackFrame frames[16];
    int frameCount = 0;
    dapGetStackFrames(frames, 16, &frameCount);

    printf("\033[1;36m>> Call Stack (%d frames):\033[0m\r\n", frameCount);
    if (frameCount == 0)
    {
        printf("   (no active stack frames — debugger not running)\r\n");
    }
    else
    {
        for (int i = 0; i < frameCount && i < 4; i++)
        {
            printf("   #%d \033[1;37m%-30.30s\033[0m at %s:%d\r\n",
                   i, frames[i].name, frames[i].file, frames[i].line);
        }
    }
    printf("\r\n");

    /* Variables Inspection */
    DapScope scopes[2];
    int scopeNum = 0;
    dapGetScopes(scopes, 2, &scopeNum);

    printf("\033[1;36m>> Variables (Locals & Arguments):\033[0m\r\n");
    if (scopeNum == 0)
    {
        printf("   (no variable scopes available)\r\n");
    }
    else
    {
        for (int sc = 0; sc < scopeNum; sc++)
        {
            printf("   \033[1;33m[%s]\033[0m\r\n", scopes[sc].name);
            for (int v = 0; v < scopes[sc].variableCount && v < 4; v++)
            {
                DapVariable *var = &scopes[sc].variables[v];
                printf("     - \033[1;32m%-16.16s\033[0m %-8.8s = \033[1;37m%s\033[0m\r\n",
                       var->name, var->type, var->value);
            }
        }
    }
    printf("\r\n");

    /* Breakpoints List */
    int bpCount = dapBreakpointCount();
    printf("\033[1;36m>> Breakpoints (%d configured):\033[0m\r\n", bpCount);
    if (bpCount == 0)
    {
        printf("   (no breakpoints set. Press F9 in editor to set one)\r\n");
    }
    else
    {
        for (int b = 0; b < bpCount && b < 5; b++)
        {
            const DapBreakpoint *bp = dapGetBreakpoint(b);
            if (bp)
            {
                printf("   [%d] %-24.24s:%-4d  hits:%-3d %s\r\n",
                       bp->id, bp->file, bp->line, bp->hitCount,
                       bp->enabled ? "\033[32m[enabled]\033[0m" : "\033[90m[disabled]\033[0m");
            }
        }
    }

    /* Footer */
    printf("\033[%d;1H\033[90m--------------------------------------------------------------------------------\033[0m\r\n", viewport.screenRows - 1);
    printf("\033[1;34m[Tab] Section  [F5/c] Cont  [F10/n] Next  [F11/s] Step  [Esc] Back to Editor\033[0m");
}

void dapDashboardHandleKey(int key, int modifiers)
{
    (void)modifiers;

    if (key == 27) /* Esc */
    {
        dapDashboardClose();
        return;
    }

    if (key == '\t')
    {
        dapDashboard.selectedSection = (dapDashboard.selectedSection + 1) % 4;
        return;
    }

    if (key == 'c' || key == '5')
    {
        if (dapGetState() == DAP_STATE_STOPPED)
            dapStartSession(currentFile, NULL);
        else
            dapContinue();
        return;
    }

    if (key == 'n' || key == '0')
    {
        dapStepOver();
        return;
    }

    if (key == 's' || key == '1')
    {
        dapStepInto();
        return;
    }

    if (key == 'o')
    {
        dapStepOut();
        return;
    }

    if (key == 'p' || key == '6')
    {
        dapPause();
        return;
    }

    if (key == 'r')
    {
        dapRestart();
        return;
    }

    if (key == 'x')
    {
        dapStopSession();
        return;
    }

    if (key == 'b')
    {
        if (currentFile && cursor.y >= 0)
            dapToggleBreakpoint(currentFile, cursor.y + 1);
        return;
    }
}
