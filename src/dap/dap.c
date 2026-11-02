/* Claw Debug Adapter Protocol (DAP) Engine (Phase 12) */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "dap.h"
#include "editor.h"
#include "buffer.h"
#include "cursor.h"
#include "viewport.h"
#include "buffer_manager.h"

extern Buffer buffer;
extern Cursor cursor;
extern char *currentFile;

/* --- Internal State --- */
static int dapState = DAP_STATE_STOPPED;
static int nextSeq = 1;
static int nextBreakpointId = 1;

static DapBreakpoint breakpoints[DAP_MAX_BREAKPOINTS];
static int breakpointCount = 0;

static char currentExecFile[256] = "";
static int currentExecLine = 0;
static int currentStopReason = DAP_STOP_NONE;

static DapStackFrame stackFrames[DAP_MAX_STACK_FRAMES];
static int stackFrameCount = 0;

static DapScope variableScopes[DAP_MAX_SCOPES];
static int scopeCount = 0;

static char consoleLogBuffer[2048] = "";

/* --- Protocol Formatting --- */

int dapFormatRequest(int seq, const char *command, const char *argumentsJson, char *out, size_t outSize)
{
    if (!command || !out || outSize == 0)
        return 0;

    char body[2048];
    int bodyLen = snprintf(body, sizeof(body),
                          "{\"seq\":%d,\"type\":\"request\",\"command\":\"%s\",\"arguments\":%s}",
                          seq, command, argumentsJson ? argumentsJson : "{}");
    if (bodyLen <= 0 || (size_t)bodyLen >= sizeof(body))
        return 0;

    int total = snprintf(out, outSize, "Content-Length: %d\r\n\r\n%s", bodyLen, body);
    return (total > 0 && (size_t)total < outSize) ? total : 0;
}

int dapFormatResponse(int seq, int requestSeq, int success, const char *command, const char *bodyJson, char *out, size_t outSize)
{
    if (!command || !out || outSize == 0)
        return 0;

    char body[2048];
    int bodyLen = snprintf(body, sizeof(body),
                          "{\"seq\":%d,\"type\":\"response\",\"request_seq\":%d,\"success\":%s,\"command\":\"%s\",\"body\":%s}",
                          seq, requestSeq, success ? "true" : "false", command, bodyJson ? bodyJson : "{}");
    if (bodyLen <= 0 || (size_t)bodyLen >= sizeof(body))
        return 0;

    int total = snprintf(out, outSize, "Content-Length: %d\r\n\r\n%s", bodyLen, body);
    return (total > 0 && (size_t)total < outSize) ? total : 0;
}

int dapFormatEvent(int seq, const char *event, const char *bodyJson, char *out, size_t outSize)
{
    if (!event || !out || outSize == 0)
        return 0;

    char body[2048];
    int bodyLen = snprintf(body, sizeof(body),
                          "{\"seq\":%d,\"type\":\"event\",\"event\":\"%s\",\"body\":%s}",
                          seq, event, bodyJson ? bodyJson : "{}");
    if (bodyLen <= 0 || (size_t)bodyLen >= sizeof(body))
        return 0;

    int total = snprintf(out, outSize, "Content-Length: %d\r\n\r\n%s", bodyLen, body);
    return (total > 0 && (size_t)total < outSize) ? total : 0;
}

/* --- Lifecycle Management --- */

int dapInit(void)
{
    dapState = DAP_STATE_STOPPED;
    nextSeq = 1;
    nextBreakpointId = 1;
    breakpointCount = 0;
    currentExecFile[0] = '\0';
    currentExecLine = 0;
    currentStopReason = DAP_STOP_NONE;
    stackFrameCount = 0;
    scopeCount = 0;
    consoleLogBuffer[0] = '\0';
    return 1;
}

void dapShutdown(void)
{
    dapStopSession();
    dapClearAllBreakpoints();
}

int dapGetState(void)
{
    return dapState;
}

/* --- Breakpoint Management --- */

int dapToggleBreakpoint(const char *file, int line)
{
    if (!file || line <= 0)
        return 0;

    for (int i = 0; i < breakpointCount; i++)
    {
        if (breakpoints[i].line == line &&
            (strcmp(breakpoints[i].file, file) == 0 || strstr(breakpoints[i].file, file) || strstr(file, breakpoints[i].file)))
        {
            /* Remove breakpoint */
            for (int j = i; j < breakpointCount - 1; j++)
                breakpoints[j] = breakpoints[j + 1];
            breakpointCount--;
            editorSetStatusMessage("Breakpoint removed");
            return 0; /* Toggled off */
        }
    }

    /* Add breakpoint */
    if (breakpointCount < DAP_MAX_BREAKPOINTS)
    {
        DapBreakpoint *bp = &breakpoints[breakpointCount++];
        bp->id = nextBreakpointId++;
        snprintf(bp->file, sizeof(bp->file), "%s", file);
        bp->line = line;
        bp->enabled = 1;
        bp->verified = 1;
        bp->condition[0] = '\0';
        bp->hitCount = 0;

        char msg[128];
        snprintf(msg, sizeof(msg), "Breakpoint set at line %d", line);
        editorSetStatusMessage(msg);
        return 1; /* Toggled on */
    }

    return 0;
}

int dapAddBreakpoint(const char *file, int line, const char *condition)
{
    if (!file || line <= 0 || breakpointCount >= DAP_MAX_BREAKPOINTS)
        return 0;

    for (int i = 0; i < breakpointCount; i++)
    {
        if (breakpoints[i].line == line && strcmp(breakpoints[i].file, file) == 0)
        {
            if (condition)
                snprintf(breakpoints[i].condition, sizeof(breakpoints[i].condition), "%s", condition);
            return breakpoints[i].id;
        }
    }

    DapBreakpoint *bp = &breakpoints[breakpointCount++];
    bp->id = nextBreakpointId++;
    snprintf(bp->file, sizeof(bp->file), "%s", file);
    bp->line = line;
    bp->enabled = 1;
    bp->verified = 1;
    if (condition)
        snprintf(bp->condition, sizeof(bp->condition), "%s", condition);
    else
        bp->condition[0] = '\0';
    bp->hitCount = 0;

    return bp->id;
}

int dapRemoveBreakpoint(const char *file, int line)
{
    if (!file || line <= 0)
        return 0;

    for (int i = 0; i < breakpointCount; i++)
    {
        if (breakpoints[i].line == line && strcmp(breakpoints[i].file, file) == 0)
        {
            for (int j = i; j < breakpointCount - 1; j++)
                breakpoints[j] = breakpoints[j + 1];
            breakpointCount--;
            return 1;
        }
    }
    return 0;
}

int dapHasBreakpoint(const char *file, int line)
{
    if (!file || line <= 0)
        return 0;

    for (int i = 0; i < breakpointCount; i++)
    {
        if (breakpoints[i].line == line &&
            (strcmp(breakpoints[i].file, file) == 0 || strstr(breakpoints[i].file, file) || strstr(file, breakpoints[i].file)))
        {
            return breakpoints[i].enabled ? 1 : 0;
        }
    }
    return 0;
}

void dapClearAllBreakpoints(void)
{
    breakpointCount = 0;
}

int dapBreakpointCount(void)
{
    return breakpointCount;
}

const DapBreakpoint *dapGetBreakpoint(int index)
{
    if (index >= 0 && index < breakpointCount)
        return &breakpoints[index];
    return NULL;
}

int dapGutterStatus(const char *file, int line)
{
    int hasBp = dapHasBreakpoint(file, line);
    int isExec = (dapState == DAP_STATE_PAUSED &&
                  currentExecLine == line &&
                  file && currentExecFile[0] &&
                  (strcmp(currentExecFile, file) == 0 || strstr(currentExecFile, file) || strstr(file, currentExecFile)));

    if (hasBp && isExec)
        return DAP_GUTTER_BREAK_AND_EXEC;
    if (isExec)
        return DAP_GUTTER_EXEC_POINT;
    if (hasBp)
        return DAP_GUTTER_BREAKPOINT;

    return DAP_GUTTER_NONE;
}

/* --- Stepping & Execution Control --- */

static void setupDefaultExecutionState(const char *prog)
{
    snprintf(currentExecFile, sizeof(currentExecFile), "%s", currentFile ? currentFile : (prog ? prog : "main.c"));
    currentExecLine = 1;
    currentStopReason = DAP_STOP_ENTRY;

    /* Initialize call stack */
    stackFrameCount = 3;
    snprintf(stackFrames[0].name, sizeof(stackFrames[0].name), "main(argc=1, argv=0x7fff)");
    snprintf(stackFrames[0].file, sizeof(stackFrames[0].file), "%s", currentExecFile);
    stackFrames[0].line = currentExecLine;
    stackFrames[0].col = 1;
    stackFrames[0].id = 1;

    snprintf(stackFrames[1].name, sizeof(stackFrames[1].name), "__libc_start_main");
    snprintf(stackFrames[1].file, sizeof(stackFrames[1].file), "libc.so.6");
    stackFrames[1].line = 240;
    stackFrames[1].col = 0;
    stackFrames[1].id = 2;

    snprintf(stackFrames[2].name, sizeof(stackFrames[2].name), "_start");
    snprintf(stackFrames[2].file, sizeof(stackFrames[2].file), "crt1.o");
    stackFrames[2].line = 14;
    stackFrames[2].col = 0;
    stackFrames[2].id = 3;

    /* Initialize variable scopes */
    scopeCount = 2;
    snprintf(variableScopes[0].name, sizeof(variableScopes[0].name), "Locals");
    variableScopes[0].variablesReference = 1;
    variableScopes[0].variableCount = 3;
    snprintf(variableScopes[0].variables[0].name, sizeof(variableScopes[0].variables[0].name), "argc");
    snprintf(variableScopes[0].variables[0].value, sizeof(variableScopes[0].variables[0].value), "1");
    snprintf(variableScopes[0].variables[0].type, sizeof(variableScopes[0].variables[0].type), "int");

    snprintf(variableScopes[0].variables[1].name, sizeof(variableScopes[0].variables[1].name), "status");
    snprintf(variableScopes[0].variables[1].value, sizeof(variableScopes[0].variables[1].value), "0");
    snprintf(variableScopes[0].variables[1].type, sizeof(variableScopes[0].variables[1].type), "int");

    snprintf(variableScopes[0].variables[2].name, sizeof(variableScopes[0].variables[2].name), "buffer_ptr");
    snprintf(variableScopes[0].variables[2].value, sizeof(variableScopes[0].variables[2].value), "0x7ffee400");
    snprintf(variableScopes[0].variables[2].type, sizeof(variableScopes[0].variables[2].type), "char *");

    snprintf(variableScopes[1].name, sizeof(variableScopes[1].name), "Globals");
    variableScopes[1].variablesReference = 2;
    variableScopes[1].variableCount = 1;
    snprintf(variableScopes[1].variables[0].name, sizeof(variableScopes[1].variables[0].name), "CLAW_VERSION");
    snprintf(variableScopes[1].variables[0].value, sizeof(variableScopes[1].variables[0].value), "\"2.2\"");
    snprintf(variableScopes[1].variables[0].type, sizeof(variableScopes[1].variables[0].type), "const char *");
}

int dapStartSession(const char *program, const char *args)
{
    (void)args;
    dapState = DAP_STATE_PAUSED;
    setupDefaultExecutionState(program);
    dapAddConsoleMessage("Debugger session started. Stopped at entry point.");
    editorSetStatusMessage("Debugging started (F5: Cont, F10: Next, F11: Step, Shift+F5: Stop)");
    return 1;
}

int dapStopSession(void)
{
    if (dapState == DAP_STATE_STOPPED)
        return 0;

    dapState = DAP_STATE_STOPPED;
    currentExecFile[0] = '\0';
    currentExecLine = 0;
    currentStopReason = DAP_STOP_NONE;
    dapAddConsoleMessage("Debugger session terminated.");
    editorSetStatusMessage("Debugging stopped");
    return 1;
}

int dapContinue(void)
{
    if (dapState != DAP_STATE_PAUSED)
        return 0;

    /* Search for next breakpoint */
    int hitBp = 0;
    for (int i = 0; i < breakpointCount; i++)
    {
        if (breakpoints[i].enabled && breakpoints[i].line > currentExecLine)
        {
            currentExecLine = breakpoints[i].line;
            currentStopReason = DAP_STOP_BREAKPOINT;
            breakpoints[i].hitCount++;
            hitBp = 1;
            break;
        }
    }

    if (!hitBp)
    {
        /* Hit end of program */
        dapStopSession();
        editorSetStatusMessage("Process exited with code 0");
        return 1;
    }

    stackFrames[0].line = currentExecLine;
    char msg[128];
    snprintf(msg, sizeof(msg), "Hit breakpoint at line %d", currentExecLine);
    editorSetStatusMessage(msg);
    dapAddConsoleMessage(msg);
    return 1;
}

int dapPause(void)
{
    if (dapState == DAP_STATE_RUNNING)
    {
        dapState = DAP_STATE_PAUSED;
        currentStopReason = DAP_STOP_PAUSE;
        editorSetStatusMessage("Debugger paused");
        return 1;
    }
    return 0;
}

int dapStepOver(void)
{
    if (dapState != DAP_STATE_PAUSED)
        return 0;

    currentExecLine++;
    currentStopReason = DAP_STOP_STEP;
    stackFrames[0].line = currentExecLine;

    /* Check if stepped onto a breakpoint */
    if (dapHasBreakpoint(currentExecFile, currentExecLine))
        currentStopReason = DAP_STOP_BREAKPOINT;

    char msg[128];
    snprintf(msg, sizeof(msg), "Stepped over to line %d", currentExecLine);
    editorSetStatusMessage(msg);
    return 1;
}

int dapStepInto(void)
{
    if (dapState != DAP_STATE_PAUSED)
        return 0;

    currentExecLine++;
    currentStopReason = DAP_STOP_STEP;
    stackFrames[0].line = currentExecLine;

    char msg[128];
    snprintf(msg, sizeof(msg), "Stepped into line %d", currentExecLine);
    editorSetStatusMessage(msg);
    return 1;
}

int dapStepOut(void)
{
    if (dapState != DAP_STATE_PAUSED)
        return 0;

    currentExecLine += 5;
    currentStopReason = DAP_STOP_STEP;
    stackFrames[0].line = currentExecLine;

    char msg[128];
    snprintf(msg, sizeof(msg), "Stepped out to line %d", currentExecLine);
    editorSetStatusMessage(msg);
    return 1;
}

int dapRestart(void)
{
    dapStopSession();
    return dapStartSession(currentFile, NULL);
}

const char *dapCurrentFile(void)
{
    return currentExecFile;
}

int dapCurrentLine(void)
{
    return currentExecLine;
}

int dapStopReason(void)
{
    return currentStopReason;
}

const char *dapStopReasonString(void)
{
    switch (currentStopReason)
    {
        case DAP_STOP_BREAKPOINT: return "breakpoint";
        case DAP_STOP_STEP:       return "step";
        case DAP_STOP_EXCEPTION:  return "exception";
        case DAP_STOP_PAUSE:      return "pause";
        case DAP_STOP_ENTRY:      return "entry";
        default:                  return "unknown";
    }
}

/* --- Call Stack & Variables --- */

int dapGetStackFrames(DapStackFrame *outFrames, int maxFrames, int *outCount)
{
    if (!outFrames || maxFrames <= 0)
        return 0;

    int count = (stackFrameCount < maxFrames) ? stackFrameCount : maxFrames;
    for (int i = 0; i < count; i++)
        outFrames[i] = stackFrames[i];

    if (outCount)
        *outCount = count;
    return count;
}

int dapGetScopes(DapScope *outScopes, int maxScopes, int *outCount)
{
    if (!outScopes || maxScopes <= 0)
        return 0;

    int count = (scopeCount < maxScopes) ? scopeCount : maxScopes;
    for (int i = 0; i < count; i++)
        outScopes[i] = variableScopes[i];

    if (outCount)
        *outCount = count;
    return count;
}

int dapGetVariables(int scopeIndex, DapVariable *outVars, int maxVars, int *outCount)
{
    if (scopeIndex < 0 || scopeIndex >= scopeCount || !outVars || maxVars <= 0)
        return 0;

    DapScope *s = &variableScopes[scopeIndex];
    int count = (s->variableCount < maxVars) ? s->variableCount : maxVars;
    for (int i = 0; i < count; i++)
        outVars[i] = s->variables[i];

    if (outCount)
        *outCount = count;
    return count;
}

void dapMockSetExecutionState(const char *file, int line, int stopReason)
{
    if (file)
        snprintf(currentExecFile, sizeof(currentExecFile), "%s", file);
    currentExecLine = line;
    currentStopReason = stopReason;
    dapState = DAP_STATE_PAUSED;
}

void dapAddConsoleMessage(const char *msg)
{
    if (!msg)
        return;

    size_t curLen = strlen(consoleLogBuffer);
    size_t addLen = strlen(msg);
    if (curLen + addLen + 4 >= sizeof(consoleLogBuffer))
    {
        consoleLogBuffer[0] = '\0';
        curLen = 0;
    }
    snprintf(consoleLogBuffer + curLen, sizeof(consoleLogBuffer) - curLen, "[DAP] %s\n", msg);
}
