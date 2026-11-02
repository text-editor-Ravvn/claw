/* Claw Intelligent IDE Services Header (Phase 11) */
#ifndef LSP_H
#define LSP_H

#include <stddef.h>

#define LSP_MAX_SERVERS 16
#define LSP_MAX_DIAGNOSTICS 512
#define LSP_MAX_COMPLETIONS 128
#define LSP_MAX_LOCATIONS 128
#define LSP_MAX_CODE_ACTIONS 32

/* Diagnostic Severity Levels */
enum LspSeverity
{
    LSP_SEVERITY_NONE = 0,
    LSP_SEVERITY_ERROR = 1,
    LSP_SEVERITY_WARNING = 2,
    LSP_SEVERITY_INFO = 3,
    LSP_SEVERITY_HINT = 4
};

/* Completion Item Kinds (matching LSP specification) */
enum LspCompletionKind
{
    LSP_KIND_TEXT = 1,
    LSP_KIND_METHOD = 2,
    LSP_KIND_FUNCTION = 3,
    LSP_KIND_CONSTRUCTOR = 4,
    LSP_KIND_FIELD = 5,
    LSP_KIND_VARIABLE = 6,
    LSP_KIND_CLASS = 7,
    LSP_KIND_INTERFACE = 8,
    LSP_KIND_MODULE = 9,
    LSP_KIND_PROPERTY = 10,
    LSP_KIND_UNIT = 11,
    LSP_KIND_VALUE = 12,
    LSP_KIND_ENUM = 13,
    LSP_KIND_KEYWORD = 14,
    LSP_KIND_SNIPPET = 15,
    LSP_KIND_COLOR = 16,
    LSP_KIND_FILE = 17,
    LSP_KIND_REFERENCE = 18,
    LSP_KIND_FOLDER = 19,
    LSP_KIND_CONSTANT = 21,
    LSP_KIND_STRUCT = 22,
    LSP_KIND_EVENT = 23,
    LSP_KIND_OPERATOR = 24,
    LSP_KIND_TYPE_PARAM = 25
};

/* Diagnostic representation */
typedef struct
{
    char uri[256];
    int line;    /* 1-indexed line */
    int column;  /* 1-indexed column */
    int severity;/* LSP_SEVERITY_* */
    char message[256];
    char source[64];
    char code[32];
} LspDiagnostic;

/* Completion item representation */
typedef struct
{
    char label[128];
    int kind;
    char detail[128];
    char documentation[256];
    char insertText[128];
} LspCompletionItem;

/* Code location for Definition / References */
typedef struct
{
    char file[256];
    int line;   /* 1-indexed */
    int col;    /* 1-indexed */
    char preview[128];
} LspLocation;

/* Code Action representation */
typedef struct
{
    char title[128];
    char kind[64]; /* "quickfix", "refactor.extract", "refactor.inline", "refactor.rename" */
    char targetFile[256];
    int line;
    int col;
    char replacementText[256];
    int deleteOld;
} LspCodeAction;

/* LSP Server status */
enum LspServerStatus
{
    LSP_STATUS_STOPPED = 0,
    LSP_STATUS_STARTING,
    LSP_STATUS_RUNNING,
    LSP_STATUS_SHUTDOWN,
    LSP_STATUS_ERROR
};

/* Registered language server description */
typedef struct
{
    char langId[32];
    char command[128];
    char rootUri[256];
    int status;
    int initialized;
    int nextRequestId;
    int hasCompletion;
    int hasHover;
    int hasDefinition;
    int hasReferences;
    int hasCodeAction;
} LspServer;

/* Interactive Modals States */
typedef struct
{
    int active;
    char query[64];
    int queryLen;
    int triggerRow;
    int triggerCol;
    int selectedIndex;
    int count;
    LspCompletionItem items[LSP_MAX_COMPLETIONS];
} LspCompletionState;

typedef struct
{
    int active;
    int line;
    int col;
    char symbol[128];
    char signature[256];
    char documentation[512];
} LspHoverState;

typedef struct
{
    int active;
    char symbol[128];
    int count;
    int selectedIndex;
    int scrollOffset;
    LspLocation locations[LSP_MAX_LOCATIONS];
} LspReferencesState;

typedef struct
{
    int active;
    int count;
    int selectedIndex;
    LspCodeAction actions[LSP_MAX_CODE_ACTIONS];
} LspCodeActionsState;

extern LspCompletionState lspCompletionMenu;
extern LspHoverState lspHoverTooltip;
extern LspReferencesState lspReferencesOverlay;
extern LspCodeActionsState lspCodeActionsModal;

/* --- Protocol Framing & Formatting --- */
int lspFormatHeader(size_t bodyLength, char *out, size_t outSize);
int lspParseHeader(const char *buffer, size_t *outContentLen, size_t *outHeaderLen);
int lspFormatRequest(int id, const char *method, const char *paramsJson, char *out, size_t outSize);
int lspFormatNotification(const char *method, const char *paramsJson, char *out, size_t outSize);
int lspFormatResponse(int id, const char *resultJson, char *out, size_t outSize);

/* --- Server Lifecycle & Registration --- */
int lspInit(void);
void lspShutdown(void);
int lspRegisterServer(const char *langId, const char *command);
int lspStartServer(const char *langId, const char *rootPath);
int lspStopServer(const char *langId);
void lspStopAll(void);
LspServer *lspGetServerForLanguage(const char *langId);
int lspServerCount(void);

/* --- Document Synchronization Notifications --- */
void lspNotifyDidOpen(const char *filename, const char *langId, const char *text);
void lspNotifyDidChange(const char *filename, const char *text);
void lspNotifyDidSave(const char *filename);
void lspNotifyDidClose(const char *filename);

/* --- Diagnostics Engine --- */
void lspAddDiagnostic(const char *uri, int line, int col, int severity, const char *msg, const char *source, const char *code);
void lspClearDiagnostics(const char *uri);
void lspClearAllDiagnostics(void);
int lspGetDiagnosticsForLine(const char *uri, int line, LspDiagnostic *outDiags, int maxDiags);
int lspGutterStatus(const char *uri, int line);
int lspTotalDiagnosticsCount(const char *uri);
const LspDiagnostic *lspGetFirstDiagnosticAt(const char *uri, int line);

/* --- Code Completion Engine & Interactive Menu --- */
void lspCompletionOpen(void);
void lspCompletionClose(void);
int lspCompletionIsActive(void);
void lspCompletionHandleKey(int key, int modifiers);
void lspCompletionDraw(void);
int lspRequestCompletion(const char *uri, int line, int col, const char *prefix, LspCompletionItem *outItems, int maxItems);

/* --- Go-to-Definition Provider --- */
int lspGotoDefinition(void);
int lspLookupDefinition(const char *uri, int line, int col, char *outTargetFile, int *outLine, int *outCol);

/* --- Hover Information Provider & Tooltip --- */
void lspHoverOpen(void);
void lspHoverClose(void);
int lspHoverIsActive(void);
void lspHoverHandleKey(int key, int modifiers);
void lspHoverDraw(void);
int lspRequestHover(const char *uri, int line, int col, char *outSignature, size_t sigSize, char *outDoc, size_t docSize);

/* --- Find References Provider & Overlay --- */
void lspReferencesOpen(void);
void lspReferencesClose(void);
int lspReferencesIsActive(void);
void lspReferencesHandleKey(int key, int modifiers);
void lspReferencesDraw(void);
int lspRequestReferences(const char *uri, int line, int col, LspLocation *outLocs, int maxLocs);

/* --- Code Actions & Refactoring Framework --- */
void lspCodeActionsOpen(void);
void lspCodeActionsClose(void);
int lspCodeActionsIsActive(void);
void lspCodeActionsHandleKey(int key, int modifiers);
void lspCodeActionsDraw(void);
int lspRequestCodeActions(const char *uri, int line, int col, LspCodeAction *outActions, int maxActions);
int lspApplyCodeAction(int index);

#endif /* LSP_H */
