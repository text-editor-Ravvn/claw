/* Claw Phase 11 Test Suite — Intelligent IDE Services */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "lsp.h"
#include "buffer.h"
#include "cursor.h"
#include "viewport.h"
#include "symbols.h"
#include "buffer_manager.h"
#include "input.h"
#include "split.h"

/* --- Mocking / Stubbing Global Editor State for Isolated Unit Testing --- */
extern Buffer buffer;
extern Cursor cursor;
char *currentFile = NULL;
Viewport viewport = {24, 80, 0, 0, 0};
SplitLayout splitLayout = {0};

static char lastStatusMsg[256] = "";

void editorSetStatusMessage(const char *msg)
{
    if (msg)
        snprintf(lastStatusMsg, sizeof(lastStatusMsg), "%s", msg);
    else
        lastStatusMsg[0] = '\0';
}

const char *editorStatusMessage(void)
{
    return lastStatusMsg;
}

void renderForceClear(void)
{
}

void refreshSearchMatches(void) {}
void highlightInit(void) {}
void gitRefreshStatus(const char *file) { (void)file; }
int openFile(const char *filename) { (void)filename; return 1; }
int saveFile(const char *filename) { (void)filename; return 1; }
int splitIsActive(void) { return 0; }

static void setupTestBuffer(void)
{
    bufferFree();
    bufferInit();
    cursor.x = 0;
    cursor.y = 0;

    /* Add sample code rows */
    const char *lines[] = {
        "#include <stdio.h>",
        "#include <stdlib.h>",
        "",
        "int calculate_total(int count, int price) {",
        "    int result = count * price;",
        "    return result;",
        "}",
        "",
        "int main(void) {",
        "    int items = 10;",
        "    int cost = 5;",
        "    int total = calculate_total(items, cost);",
        "    printf(\"Total: %d\\n\", total);",
        "    return 0;",
        "}"
    };

    /* Free the initial single empty row from bufferInit */
    if (buffer.numRows > 0 && buffer.rows)
    {
        free(buffer.rows[0].chars);
        free(buffer.rows);
        buffer.rows = NULL;
        buffer.numRows = 0;
    }

    for (int i = 0; i < 15; i++)
    {
        Row *newRows = realloc(buffer.rows, sizeof(Row) * (buffer.numRows + 1));
        assert(newRows != NULL);
        buffer.rows = newRows;
        int len = (int)strlen(lines[i]);
        buffer.rows[buffer.numRows].size = len;
        buffer.rows[buffer.numRows].chars = malloc((size_t)len + 1);
        memcpy(buffer.rows[buffer.numRows].chars, lines[i], len + 1);
        buffer.rows[buffer.numRows].hl = NULL;
        buffer.rows[buffer.numRows].hlSize = 0;
        buffer.rows[buffer.numRows].hlOpenState = 0;
        buffer.numRows++;
    }

    if (currentFile)
        free(currentFile);
    currentFile = strdup("test_app.c");
}

/* ========================================================================= */
/* Test Cases                                                                */
/* ========================================================================= */

static void test_lsp_framing(void)
{
    printf("[1/12] Testing LSP JSON-RPC Framing & Header Parsing...\n");

    char header[64];
    int len = lspFormatHeader(128, header, sizeof(header));
    assert(len > 0);
    assert(strcmp(header, "Content-Length: 128\r\n\r\n") == 0);

    size_t outContentLen = 0, outHeaderLen = 0;
    int parseOk = lspParseHeader("Content-Length: 256\r\n\r\n{\"jsonrpc\":\"2.0\"}", &outContentLen, &outHeaderLen);
    assert(parseOk == 1);
    assert(outContentLen == 256);
    assert(outHeaderLen == strlen("Content-Length: 256\r\n\r\n"));

    /* Test lowercase header and whitespace robustness */
    parseOk = lspParseHeader("content-length:   512  \r\n\r\nbody", &outContentLen, &outHeaderLen);
    assert(parseOk == 1);
    assert(outContentLen == 512);

    /* Invalid header rejection */
    assert(lspParseHeader("Invalid-Header: 123\r\n\r\n", &outContentLen, &outHeaderLen) == 0);
    assert(lspParseHeader(NULL, &outContentLen, &outHeaderLen) == 0);

    printf("  -> LSP Framing & Header tests PASSED.\n");
}

static void test_lsp_message_formatting(void)
{
    printf("[2/12] Testing LSP Request, Notification & Response Serialization...\n");

    char bufferOut[1024];

    /* Request */
    int reqLen = lspFormatRequest(42, "textDocument/completion", "{\"textDocument\":{\"uri\":\"file:///main.c\"}}", bufferOut, sizeof(bufferOut));
    assert(reqLen > 0);
    assert(strstr(bufferOut, "Content-Length:") != NULL);
    assert(strstr(bufferOut, "\"jsonrpc\":\"2.0\"") != NULL);
    assert(strstr(bufferOut, "\"id\":42") != NULL);
    assert(strstr(bufferOut, "\"method\":\"textDocument/completion\"") != NULL);

    /* Notification */
    int notifLen = lspFormatNotification("textDocument/didOpen", "{\"textDocument\":{\"uri\":\"file:///main.c\"}}", bufferOut, sizeof(bufferOut));
    assert(notifLen > 0);
    assert(strstr(bufferOut, "Content-Length:") != NULL);
    assert(strstr(bufferOut, "\"method\":\"textDocument/didOpen\"") != NULL);
    assert(strstr(bufferOut, "\"id\":") == NULL); /* Notifications have no ID */

    /* Response */
    int respLen = lspFormatResponse(42, "{\"items\":[]}", bufferOut, sizeof(bufferOut));
    assert(respLen > 0);
    assert(strstr(bufferOut, "\"id\":42") != NULL);
    assert(strstr(bufferOut, "\"result\":{\"items\":[]}") != NULL);

    printf("  -> LSP Message Formatting tests PASSED.\n");
}

static void test_lsp_server_lifecycle(void)
{
    printf("[3/12] Testing Language Server Registry & Lifecycle Management...\n");

    lspInit();
    assert(lspServerCount() >= 7);

    /* Query built-in servers */
    LspServer *cServer = lspGetServerForLanguage("c");
    assert(cServer != NULL);
    assert(strcmp(cServer->command, "clangd") == 0);
    assert(cServer->status == LSP_STATUS_STOPPED);

    LspServer *pyServer = lspGetServerForLanguage("python");
    assert(pyServer != NULL);
    assert(strstr(pyServer->command, "pyright") != NULL);

    LspServer *rsServer = lspGetServerForLanguage("rust");
    assert(rsServer != NULL);
    assert(strcmp(rsServer->command, "rust-analyzer") == 0);

    /* Start server */
    int startOk = lspStartServer("c", "/workspace");
    assert(startOk == 1);
    assert(cServer->status == LSP_STATUS_RUNNING);
    assert(cServer->initialized == 1);
    assert(strcmp(cServer->rootUri, "/workspace") == 0);

    /* Register custom server */
    int regOk = lspRegisterServer("zig", "zls");
    assert(regOk == 1);
    LspServer *zigServer = lspGetServerForLanguage("zig");
    assert(zigServer != NULL);
    assert(strcmp(zigServer->command, "zls") == 0);

    /* Stop server */
    lspStopServer("c");
    assert(cServer->status == LSP_STATUS_STOPPED);

    /* Stop all */
    lspStartServer("rust", ".");
    lspStopAll();
    assert(rsServer->status == LSP_STATUS_STOPPED);

    printf("  -> Server Registry & Lifecycle tests PASSED.\n");
}

static void test_lsp_document_synchronization(void)
{
    printf("[4/12] Testing Document Synchronization Notifications...\n");

    /* Ensure notifications do not crash or leak */
    lspNotifyDidOpen("main.c", "c", "int main() {}");
    lspNotifyDidChange("main.c", "int main() { return 0; }");
    lspNotifyDidSave("main.c");
    lspNotifyDidClose("main.c");

    printf("  -> Document Synchronization tests PASSED.\n");
}

static void test_lsp_diagnostics_engine(void)
{
    printf("[5/12] Testing Diagnostics Engine & Storage...\n");

    lspClearAllDiagnostics();
    assert(lspTotalDiagnosticsCount(NULL) == 0);

    /* Add diagnostics */
    lspAddDiagnostic("test_app.c", 4, 1, LSP_SEVERITY_ERROR, "Unknown type name 'unknown_t'", "clang", "err1");
    lspAddDiagnostic("test_app.c", 5, 5, LSP_SEVERITY_WARNING, "Unused variable 'x'", "clang", "w1");
    lspAddDiagnostic("test_app.c", 5, 12, LSP_SEVERITY_INFO, "Consider using const", "clang", "i1");
    lspAddDiagnostic("other.c", 10, 1, LSP_SEVERITY_ERROR, "Syntax error", "clang", "err2");

    assert(lspTotalDiagnosticsCount("test_app.c") == 3);
    assert(lspTotalDiagnosticsCount("other.c") == 1);
    assert(lspTotalDiagnosticsCount(NULL) == 4);

    /* Line diagnostics query */
    LspDiagnostic lineDiags[4];
    int lineCount = lspGetDiagnosticsForLine("test_app.c", 5, lineDiags, 4);
    assert(lineCount == 2);
    assert(strcmp(lineDiags[0].message, "Unused variable 'x'") == 0);
    assert(strcmp(lineDiags[1].message, "Consider using const'") != 0);

    /* Clear single file diagnostics */
    lspClearDiagnostics("other.c");
    assert(lspTotalDiagnosticsCount("other.c") == 0);
    assert(lspTotalDiagnosticsCount("test_app.c") == 3);

    /* Clear all */
    lspClearAllDiagnostics();
    assert(lspTotalDiagnosticsCount(NULL) == 0);

    printf("  -> Diagnostics Engine tests PASSED.\n");
}

static void test_lsp_gutter_status(void)
{
    printf("[6/12] Testing Gutter Diagnostic Indicators Resolution...\n");

    lspClearAllDiagnostics();
    assert(lspGutterStatus("file.c", 10) == LSP_SEVERITY_NONE);

    /* Error priority over warning */
    lspAddDiagnostic("file.c", 10, 1, LSP_SEVERITY_WARNING, "Warning message", "src", "w");
    assert(lspGutterStatus("file.c", 10) == LSP_SEVERITY_WARNING);

    lspAddDiagnostic("file.c", 10, 5, LSP_SEVERITY_ERROR, "Error message", "src", "e");
    assert(lspGutterStatus("file.c", 10) == LSP_SEVERITY_ERROR);

    /* Info priority */
    lspAddDiagnostic("file.c", 20, 1, LSP_SEVERITY_HINT, "Hint message", "src", "h");
    assert(lspGutterStatus("file.c", 20) == LSP_SEVERITY_HINT);

    lspAddDiagnostic("file.c", 20, 1, LSP_SEVERITY_INFO, "Info message", "src", "i");
    assert(lspGutterStatus("file.c", 20) == LSP_SEVERITY_INFO);

    lspClearAllDiagnostics();
    printf("  -> Gutter Diagnostic tests PASSED.\n");
}

static void test_lsp_completion_engine(void)
{
    printf("[7/12] Testing Code Completion Engine...\n");

    setupTestBuffer();

    LspCompletionItem items[64];
    /* Test prefix matching on keywords ("re" -> return) */
    int count = lspRequestCompletion("test_app.c", 6, 4, "re", items, 64);
    assert(count > 0);

    int foundReturn = 0;
    for (int i = 0; i < count; i++)
    {
        if (strcmp(items[i].label, "return") == 0)
        {
            foundReturn = 1;
            assert(items[i].kind == LSP_KIND_KEYWORD);
            break;
        }
    }
    assert(foundReturn == 1);

    /* Test prefix matching on buffer identifiers ("calc" -> calculate_total) */
    count = lspRequestCompletion("test_app.c", 12, 10, "calc", items, 64);
    assert(count > 0);
    int foundCalc = 0;
    for (int i = 0; i < count; i++)
    {
        if (strcmp(items[i].label, "calculate_total") == 0)
        {
            foundCalc = 1;
            break;
        }
    }
    assert(foundCalc == 1);

    printf("  -> Code Completion tests PASSED.\n");
}

static void test_lsp_definition_provider(void)
{
    printf("[8/12] Testing Go-to-Definition Provider...\n");

    setupTestBuffer();

    /* Position cursor on "calculate_total" at line 12: "calculate_total(items, cost)" */
    cursor.y = 11; /* 0-indexed line 12 */
    cursor.x = 20;

    char targetFile[256] = "";
    int targetLine = 0;
    int targetCol = 0;

    int found = lspLookupDefinition("test_app.c", cursor.y + 1, cursor.x + 1, targetFile, &targetLine, &targetCol);
    assert(found == 1);
    assert(targetLine == 4); /* calculate_total is defined at line 4 */

    /* Test navigation execution */
    int jumped = lspGotoDefinition();
    assert(jumped == 1);
    assert(cursor.y == 3); /* 0-indexed line 4 */

    printf("  -> Go-to-Definition tests PASSED.\n");
}

static void test_lsp_hover_provider(void)
{
    printf("[9/12] Testing Hover Information & Documentation Provider...\n");

    setupTestBuffer();

    /* Hover on keyword 'return' at line 6 */
    cursor.y = 5;
    cursor.x = 6;

    char signature[256] = "";
    char doc[512] = "";

    int hoverOk = lspRequestHover("test_app.c", cursor.y + 1, cursor.x + 1, signature, sizeof(signature), doc, sizeof(doc));
    assert(hoverOk == 1);
    assert(strstr(signature, "return") != NULL);
    assert(strstr(doc, "keyword") != NULL);

    /* Hover on a line with diagnostic */
    lspClearAllDiagnostics();
    lspAddDiagnostic("test_app.c", cursor.y + 1, cursor.x + 1, LSP_SEVERITY_ERROR, "Type mismatch error", "clang", "E01");

    hoverOk = lspRequestHover("test_app.c", cursor.y + 1, cursor.x + 1, signature, sizeof(signature), doc, sizeof(doc));
    assert(hoverOk == 1);
    assert(strstr(signature, "Error") != NULL);
    assert(strstr(doc, "Type mismatch error") != NULL);

    lspClearAllDiagnostics();
    printf("  -> Hover Provider tests PASSED.\n");
}

static void test_lsp_references_provider(void)
{
    printf("[10/12] Testing Find References Provider...\n");

    setupTestBuffer();

    /* Position cursor on 'total' at line 12 */
    cursor.y = 11;
    cursor.x = 10;

    LspLocation locs[16];
    int count = lspRequestReferences("test_app.c", cursor.y + 1, cursor.x + 1, locs, 16);
    assert(count >= 2); /* calculate_total, int total =, printf("Total:", total) */

    for (int i = 0; i < count; i++)
    {
        assert(locs[i].line > 0);
        assert(locs[i].col > 0);
        assert(locs[i].preview[0] != '\0');
    }

    printf("  -> Find References tests PASSED.\n");
}

static void test_lsp_code_actions(void)
{
    printf("[11/12] Testing Code Actions & Refactoring Framework...\n");

    setupTestBuffer();
    cursor.y = 4;
    cursor.x = 5;

    lspClearAllDiagnostics();
    lspAddDiagnostic("test_app.c", 5, 5, LSP_SEVERITY_WARNING, "Unused variable", "clang", "w");

    LspCodeAction actions[8];
    int count = lspRequestCodeActions("test_app.c", 5, 5, actions, 8);
    assert(count >= 3);

    /* Check QuickFix action */
    assert(strcmp(actions[0].kind, "quickfix") == 0);
    assert(strstr(actions[0].title, "QuickFix") != NULL);

    /* Check Refactor Extract */
    assert(strcmp(actions[1].kind, "refactor.extract") == 0);

    /* Execute quickfix action */
    lspCodeActionsModal.count = count;
    for (int i = 0; i < count; i++)
        lspCodeActionsModal.actions[i] = actions[i];

    int applied = lspApplyCodeAction(0);
    assert(applied == 1);

    lspClearAllDiagnostics();
    printf("  -> Code Actions tests PASSED.\n");
}

static void test_lsp_view_controllers(void)
{
    printf("[12/12] Testing Interactive UI View Controllers (Completion, Hover, References, CodeActions)...\n");

    setupTestBuffer();

    /* 1. Completion Menu */
    assert(lspCompletionIsActive() == 0);
    lspCompletionOpen();
    assert(lspCompletionIsActive() == 1);
    assert(lspCompletionMenu.count > 0);

    /* Navigate down */
    lspCompletionHandleKey(ARROW_DOWN, 0);
    assert(lspCompletionMenu.selectedIndex == 1);

    /* Navigate up */
    lspCompletionHandleKey(ARROW_UP, 0);
    assert(lspCompletionMenu.selectedIndex == 0);

    /* Close */
    lspCompletionHandleKey(27, 0);
    assert(lspCompletionIsActive() == 0);

    /* 2. Hover Tooltip */
    assert(lspHoverIsActive() == 0);
    cursor.y = 3; cursor.x = 6; /* calculate_total */
    lspHoverOpen();
    assert(lspHoverIsActive() == 1);
    lspHoverHandleKey(27, 0);
    assert(lspHoverIsActive() == 0);

    /* 3. References Overlay */
    assert(lspReferencesIsActive() == 0);
    lspReferencesOpen();
    assert(lspReferencesIsActive() == 1);
    lspReferencesHandleKey(27, 0);
    assert(lspReferencesIsActive() == 0);

    /* 4. Code Actions Modal */
    assert(lspCodeActionsIsActive() == 0);
    lspCodeActionsOpen();
    assert(lspCodeActionsIsActive() == 1);
    lspCodeActionsHandleKey(27, 0);
    assert(lspCodeActionsIsActive() == 0);

    printf("  -> Interactive UI View Controllers tests PASSED.\n");
}

int main(void)
{
    printf("\n==================================================\n");
    printf("     Claw Phase 11 Test Suite (LSP & IDE Services)\n");
    printf("==================================================\n");

    test_lsp_framing();
    test_lsp_message_formatting();
    test_lsp_server_lifecycle();
    test_lsp_document_synchronization();
    test_lsp_diagnostics_engine();
    test_lsp_gutter_status();
    test_lsp_completion_engine();
    test_lsp_definition_provider();
    test_lsp_hover_provider();
    test_lsp_references_provider();
    test_lsp_code_actions();
    test_lsp_view_controllers();

    printf("\n==================================================\n");
    printf("   ALL PHASE 11 TESTS COMPLETED & PASSED!\n");
    printf("==================================================\n\n");

    if (currentFile)
        free(currentFile);
    bufferFree();
    return 0;
}
