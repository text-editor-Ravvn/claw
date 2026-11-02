/* Claw Debug Adapter Protocol (DAP) Header (Phase 12) */
#ifndef DAP_H
#define DAP_H

#include <stddef.h>

#define DAP_MAX_BREAKPOINTS 128
#define DAP_MAX_STACK_FRAMES 64
#define DAP_MAX_VARIABLES 64
#define DAP_MAX_SCOPES 4

/* Debugger State */
enum DapState
{
    DAP_STATE_STOPPED = 0,
    DAP_STATE_INITIALIZING,
    DAP_STATE_RUNNING,
    DAP_STATE_PAUSED
};

/* Stop Reason */
enum DapStopReason
{
    DAP_STOP_NONE = 0,
    DAP_STOP_BREAKPOINT,
    DAP_STOP_STEP,
    DAP_STOP_EXCEPTION,
    DAP_STOP_PAUSE,
    DAP_STOP_ENTRY
};

/* Inline Gutter Indicator Status */
enum DapGutterStatus
{
    DAP_GUTTER_NONE = 0,
    DAP_GUTTER_BREAKPOINT,
    DAP_GUTTER_EXEC_POINT,
    DAP_GUTTER_BREAK_AND_EXEC
};

/* Breakpoint representation */
typedef struct
{
    int id;
    char file[256];
    int line;      /* 1-indexed */
    int enabled;
    int verified;
    char condition[128];
    int hitCount;
} DapBreakpoint;

/* Call stack frame */
typedef struct
{
    int id;
    char name[128];
    char file[256];
    int line;
    int col;
} DapStackFrame;

/* Variable inspection item */
typedef struct
{
    char name[64];
    char value[128];
    char type[64];
    int variablesReference;
} DapVariable;

/* Variable scope (Locals, Arguments, Globals) */
typedef struct
{
    char name[64];
    int variablesReference;
    int variableCount;
    DapVariable variables[DAP_MAX_VARIABLES];
} DapScope;

/* Interactive Debug Dashboard State */
typedef struct
{
    int active;
    int selectedSection; /* 0=Controls, 1=Call Stack, 2=Variables, 3=Breakpoints */
    int selectedIndex;
    int scrollOffset;
    char consoleLog[2048];
} DapDashboardState;

extern DapDashboardState dapDashboard;

/* --- Protocol Framing & Message Formatting --- */
int dapFormatRequest(int seq, const char *command, const char *argumentsJson, char *out, size_t outSize);
int dapFormatResponse(int seq, int requestSeq, int success, const char *command, const char *bodyJson, char *out, size_t outSize);
int dapFormatEvent(int seq, const char *event, const char *bodyJson, char *out, size_t outSize);

/* --- Lifecycle Management --- */
int dapInit(void);
void dapShutdown(void);
int dapGetState(void);

/* --- Breakpoint Management --- */
int dapToggleBreakpoint(const char *file, int line);
int dapAddBreakpoint(const char *file, int line, const char *condition);
int dapRemoveBreakpoint(const char *file, int line);
int dapHasBreakpoint(const char *file, int line);
void dapClearAllBreakpoints(void);
int dapBreakpointCount(void);
const DapBreakpoint *dapGetBreakpoint(int index);
int dapGutterStatus(const char *file, int line);

/* --- Stepping & Execution Control --- */
int dapStartSession(const char *program, const char *args);
int dapStopSession(void);
int dapContinue(void);
int dapPause(void);
int dapStepOver(void);
int dapStepInto(void);
int dapStepOut(void);
int dapRestart(void);
const char *dapCurrentFile(void);
int dapCurrentLine(void);
int dapStopReason(void);
const char *dapStopReasonString(void);

/* --- Call Stack & Variables --- */
int dapGetStackFrames(DapStackFrame *outFrames, int maxFrames, int *outCount);
int dapGetScopes(DapScope *outScopes, int maxScopes, int *outCount);
int dapGetVariables(int scopeIndex, DapVariable *outVars, int maxVars, int *outCount);
void dapMockSetExecutionState(const char *file, int line, int stopReason);
void dapAddConsoleMessage(const char *msg);

/* --- Interactive UI Modal --- */
void dapDashboardOpen(void);
void dapDashboardClose(void);
int dapDashboardIsActive(void);
void dapDashboardHandleKey(int key, int modifiers);
void dapDashboardDraw(void);

#endif /* DAP_H */
