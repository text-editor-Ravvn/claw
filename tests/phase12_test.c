/* Claw Phase 12 Test Suite — Advanced Platform Services (DAP, Remote VFS, Templates) */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "dap.h"
#include "remote.h"
#include "templates.h"
#include "buffer.h"
#include "cursor.h"
#include "viewport.h"
#include "buffer_manager.h"
#include "input.h"

#include "split.h"

/* --- Mock Editor State --- */
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

/* ========================================================================= */
/* Test Cases                                                                */
/* ========================================================================= */

static void test_dap_protocol_framing(void)
{
    printf("[1/12] Testing DAP Protocol Framing & Message Serialization...\n");

    char buf[1024];

    /* Request */
    int len = dapFormatRequest(1, "initialize", "{\"clientID\":\"claw\"}", buf, sizeof(buf));
    assert(len > 0);
    assert(strstr(buf, "Content-Length:") != NULL);
    assert(strstr(buf, "\"type\":\"request\"") != NULL);
    assert(strstr(buf, "\"command\":\"initialize\"") != NULL);
    assert(strstr(buf, "\"seq\":1") != NULL);

    /* Response */
    len = dapFormatResponse(2, 1, 1, "initialize", "{\"supportsConfigurationDone\":true}", buf, sizeof(buf));
    assert(len > 0);
    assert(strstr(buf, "\"type\":\"response\"") != NULL);
    assert(strstr(buf, "\"request_seq\":1") != NULL);
    assert(strstr(buf, "\"success\":true") != NULL);

    /* Event */
    len = dapFormatEvent(3, "stopped", "{\"reason\":\"breakpoint\",\"threadId\":1}", buf, sizeof(buf));
    assert(len > 0);
    assert(strstr(buf, "\"type\":\"event\"") != NULL);
    assert(strstr(buf, "\"event\":\"stopped\"") != NULL);

    printf("  -> DAP Protocol Framing tests PASSED.\n");
}

static void test_dap_breakpoint_management(void)
{
    printf("[2/12] Testing DAP Breakpoint Management...\n");

    dapInit();
    dapClearAllBreakpoints();
    assert(dapBreakpointCount() == 0);

    /* Toggle on */
    int toggled = dapToggleBreakpoint("main.c", 15);
    assert(toggled == 1);
    assert(dapBreakpointCount() == 1);
    assert(dapHasBreakpoint("main.c", 15) == 1);
    assert(dapHasBreakpoint("main.c", 16) == 0);

    /* Toggle off */
    toggled = dapToggleBreakpoint("main.c", 15);
    assert(toggled == 0);
    assert(dapBreakpointCount() == 0);
    assert(dapHasBreakpoint("main.c", 15) == 0);

    /* Add conditional breakpoint */
    int bpId = dapAddBreakpoint("main.c", 42, "x > 100");
    assert(bpId > 0);
    assert(dapBreakpointCount() == 1);
    const DapBreakpoint *bp = dapGetBreakpoint(0);
    assert(bp != NULL);
    assert(bp->line == 42);
    assert(strcmp(bp->condition, "x > 100") == 0);

    /* Remove breakpoint */
    assert(dapRemoveBreakpoint("main.c", 42) == 1);
    assert(dapBreakpointCount() == 0);

    printf("  -> DAP Breakpoint tests PASSED.\n");
}

static void test_dap_gutter_status(void)
{
    printf("[3/12] Testing DAP Gutter Indicators (Breakpoints & Exec Pointer)...\n");

    dapInit();
    dapClearAllBreakpoints();

    assert(dapGutterStatus("main.c", 10) == DAP_GUTTER_NONE);

    /* Breakpoint set */
    dapAddBreakpoint("main.c", 10, NULL);
    assert(dapGutterStatus("main.c", 10) == DAP_GUTTER_BREAKPOINT);

    /* Execution pointer stopped at line 20 */
    dapMockSetExecutionState("main.c", 20, DAP_STOP_STEP);
    assert(dapGutterStatus("main.c", 20) == DAP_GUTTER_EXEC_POINT);

    /* Both breakpoint and execution pointer on line 10 */
    dapMockSetExecutionState("main.c", 10, DAP_STOP_BREAKPOINT);
    assert(dapGutterStatus("main.c", 10) == DAP_GUTTER_BREAK_AND_EXEC);

    dapStopSession();
    dapClearAllBreakpoints();
    printf("  -> DAP Gutter tests PASSED.\n");
}

static void test_dap_stepping_controls(void)
{
    printf("[4/12] Testing DAP Debugger Session & Stepping Controls...\n");

    dapInit();
    assert(dapGetState() == DAP_STATE_STOPPED);

    /* Start debugging */
    assert(dapStartSession("test_app.c", NULL) == 1);
    assert(dapGetState() == DAP_STATE_PAUSED);
    assert(dapCurrentLine() == 1);
    assert(dapStopReason() == DAP_STOP_ENTRY);

    /* Step Over */
    assert(dapStepOver() == 1);
    assert(dapCurrentLine() == 2);
    assert(dapStopReason() == DAP_STOP_STEP);

    /* Step Into */
    assert(dapStepInto() == 1);
    assert(dapCurrentLine() == 3);
    assert(dapStopReason() == DAP_STOP_STEP);

    /* Step Out */
    assert(dapStepOut() == 1);
    assert(dapCurrentLine() == 8);

    /* Add breakpoint ahead and continue */
    dapAddBreakpoint("test_app.c", 25, NULL);
    assert(dapContinue() == 1);
    assert(dapCurrentLine() == 25);
    assert(dapStopReason() == DAP_STOP_BREAKPOINT);

    /* Stop session */
    assert(dapStopSession() == 1);
    assert(dapGetState() == DAP_STATE_STOPPED);

    printf("  -> DAP Stepping Controls tests PASSED.\n");
}

static void test_dap_call_stack_and_variables(void)
{
    printf("[5/12] Testing DAP Call Stack & Variable Scopes...\n");

    dapStartSession("test_app.c", NULL);

    /* Call stack */
    DapStackFrame frames[8];
    int frameCount = 0;
    int got = dapGetStackFrames(frames, 8, &frameCount);
    assert(got >= 1);
    assert(frameCount >= 1);
    assert(strstr(frames[0].name, "main") != NULL);

    /* Scopes */
    DapScope scopes[4];
    int scopeCount = 0;
    assert(dapGetScopes(scopes, 4, &scopeCount) >= 1);
    assert(scopeCount >= 2); /* Locals and Globals */
    assert(strcmp(scopes[0].name, "Locals") == 0);
    assert(strcmp(scopes[1].name, "Globals") == 0);

    /* Variables in Locals */
    DapVariable vars[8];
    int varCount = 0;
    assert(dapGetVariables(0, vars, 8, &varCount) >= 1);
    assert(varCount >= 2);
    assert(strcmp(vars[0].name, "argc") == 0);
    assert(strcmp(vars[0].value, "1") == 0);

    dapStopSession();
    printf("  -> Call Stack & Variables tests PASSED.\n");
}

static void test_dap_dashboard_controller(void)
{
    printf("[6/12] Testing DAP Debugger Dashboard Controller...\n");

    assert(dapDashboardIsActive() == 0);
    dapDashboardOpen();
    assert(dapDashboardIsActive() == 1);
    assert(dapDashboard.selectedSection == 0);

    /* Section tab switching */
    dapDashboardHandleKey('\t', 0);
    assert(dapDashboard.selectedSection == 1);
    dapDashboardHandleKey('\t', 0);
    assert(dapDashboard.selectedSection == 2);
    dapDashboardHandleKey('\t', 0);
    assert(dapDashboard.selectedSection == 3);
    dapDashboardHandleKey('\t', 0);
    assert(dapDashboard.selectedSection == 0);

    /* Execution hotkeys in dashboard */
    dapDashboardHandleKey('c', 0); /* Start/continue */
    assert(dapGetState() == DAP_STATE_PAUSED);

    dapDashboardHandleKey('n', 0); /* Step next */
    assert(dapCurrentLine() == 2);

    dapDashboardHandleKey('x', 0); /* Stop */
    assert(dapGetState() == DAP_STATE_STOPPED);

    /* Close */
    dapDashboardHandleKey(27, 0);
    assert(dapDashboardIsActive() == 0);

    printf("  -> DAP Dashboard Controller tests PASSED.\n");
}

static void test_remote_uri_parsing(void)
{
    printf("[7/12] Testing Remote URI Parser & Canonical Formatter...\n");

    RemoteUri u;

    /* Full URI with user, host, port, path */
    int ok = remoteParseUri("claw://dev@myhost.org:9000/app/src/main.c", &u);
    assert(ok == 1);
    assert(strcmp(u.scheme, "claw") == 0);
    assert(strcmp(u.user, "dev") == 0);
    assert(strcmp(u.host, "myhost.org") == 0);
    assert(u.port == 9000);
    assert(strcmp(u.path, "/app/src/main.c") == 0);

    /* Default port for claw protocol (8765) */
    ok = remoteParseUri("claw://cloud.server/logs/syslog", &u);
    assert(ok == 1);
    assert(u.port == 8765);
    assert(strcmp(u.host, "cloud.server") == 0);

    /* SFTP default port (22) */
    ok = remoteParseUri("sftp://root@192.168.1.100/etc/hosts", &u);
    assert(ok == 1);
    assert(strcmp(u.scheme, "sftp") == 0);
    assert(u.port == 22);

    /* Canonical URI reconstruction */
    char built[256];
    assert(remoteBuildUri(&u, built, sizeof(built)) > 0);
    assert(strcmp(built, "sftp://root@192.168.1.100:22/etc/hosts") == 0);

    /* remoteIsRemotePath check */
    assert(remoteIsRemotePath("claw://box/file.txt") == 1);
    assert(remoteIsRemotePath("sftp://box/file.txt") == 1);
    assert(remoteIsRemotePath("/local/path/file.txt") == 0);
    assert(remoteIsRemotePath("C:\\local\\file.txt") == 0);

    printf("  -> Remote URI Parsing tests PASSED.\n");
}

static void test_remote_caching_and_vfs(void)
{
    printf("[8/12] Testing Remote File Fetch, Cache & Save...\n");

    remoteInit();

    char cachePath[512] = "";
    int fetched = remoteFetchFile("claw://remotebox:8765/workspace/sample.c", cachePath, sizeof(cachePath));
    assert(fetched == 1);
    assert(cachePath[0] != '\0');
    assert(strstr(cachePath, ".claw/cache/remote") != NULL);

    /* Verify cached file was written */
    FILE *fp = fopen(cachePath, "r");
    assert(fp != NULL);
    char buf[256];
    assert(fgets(buf, sizeof(buf), fp) != NULL);
    assert(strstr(buf, "Remote Virtual File") != NULL);
    fclose(fp);

    /* Stat remote file */
    RemoteFileInfo info;
    assert(remoteStatFile("claw://remotebox/sample.c", &info) == 1);
    assert(info.size > 0);

    /* List remote directory */
    RemoteDirEntry entries[8];
    int count = 0;
    assert(remoteListDirectory("claw://remotebox/workspace", entries, 8, &count) >= 1);
    assert(count >= 3);
    assert(strcmp(entries[0].name, "src") == 0);

    /* Save remote file */
    assert(remoteSaveFile("claw://remotebox:8765/workspace/sample.c", cachePath) == 1);

    remoteShutdown();
    printf("  -> Remote Caching & VFS tests PASSED.\n");
}

static void test_remote_prompt_controller(void)
{
    printf("[9/12] Testing Remote Open Dialog Controller...\n");

    assert(remotePromptIsActive() == 0);
    remoteOpenPrompt();
    assert(remotePromptIsActive() == 1);
    assert(remotePrompt.inputLen == 0);

    /* Type URI characters */
    remotePromptHandleKey('c', 0);
    remotePromptHandleKey('l', 0);
    remotePromptHandleKey('a', 0);
    remotePromptHandleKey('w', 0);
    assert(strcmp(remotePrompt.input, "claw") == 0);

    /* Backspace */
    remotePromptHandleKey(127, 0);
    assert(strcmp(remotePrompt.input, "cla") == 0);

    /* Cancel */
    remotePromptHandleKey(27, 0);
    assert(remotePromptIsActive() == 0);

    printf("  -> Remote Open Controller tests PASSED.\n");
}

static void test_project_template_registry(void)
{
    printf("[10/12] Testing Project Template Registry...\n");

    int total = templatesInit();
    assert(total >= 5);
    assert(templatesCount() >= 5);

    /* Discover templates */
    const TemplateInfo *cTmpl = templatesFindById("c");
    assert(cTmpl != NULL);
    assert(strcmp(cTmpl->language, "c") == 0);
    assert(cTmpl->fileCount >= 5);

    const TemplateInfo *pyTmpl = templatesFindById("python");
    assert(pyTmpl != NULL);
    assert(strcmp(pyTmpl->language, "python") == 0);

    const TemplateInfo *rsTmpl = templatesFindById("rust");
    assert(rsTmpl != NULL);
    assert(strcmp(rsTmpl->language, "rust") == 0);

    const TemplateInfo *goTmpl = templatesFindById("go");
    assert(goTmpl != NULL);
    assert(strcmp(goTmpl->language, "go") == 0);

    const TemplateInfo *webTmpl = templatesFindById("web");
    assert(webTmpl != NULL);
    assert(strcmp(webTmpl->language, "html") == 0);

    printf("  -> Project Template Registry tests PASSED.\n");
}

static void test_scaffolding_engine(void)
{
    printf("[11/12] Testing Project Scaffolding Engine (C, Python, Rust, Go, Web)...\n");

    const char *testBase = ".claw/test_scaffold";

    /* Scaffold C Application */
    char cTarget[256];
    snprintf(cTarget, sizeof(cTarget), "%s/test_c_app", testBase);
    int ok = templatesScaffold("c", cTarget, "my_c_project", "Test Author");
    assert(ok == 1);

    char path[512];
    snprintf(path, sizeof(path), "%s/Makefile", cTarget);
    FILE *fp = fopen(path, "r");
    assert(fp != NULL);
    char buf[256];
    assert(fgets(buf, sizeof(buf), fp) != NULL);
    fclose(fp);

    snprintf(path, sizeof(path), "%s/src/main.c", cTarget);
    fp = fopen(path, "r");
    assert(fp != NULL);
    fclose(fp);

    /* Scaffold Python Package */
    char pyTarget[256];
    snprintf(pyTarget, sizeof(pyTarget), "%s/test_py_app", testBase);
    ok = templatesScaffold("python", pyTarget, "my_py_package", "Python Dev");
    assert(ok == 1);
    snprintf(path, sizeof(path), "%s/pyproject.toml", pyTarget);
    fp = fopen(path, "r");
    assert(fp != NULL);
    fclose(fp);

    /* Scaffold Rust Binary */
    char rsTarget[256];
    snprintf(rsTarget, sizeof(rsTarget), "%s/test_rs_app", testBase);
    ok = templatesScaffold("rust", rsTarget, "my_rs_binary", "Rustacean");
    assert(ok == 1);
    snprintf(path, sizeof(path), "%s/Cargo.toml", rsTarget);
    fp = fopen(path, "r");
    assert(fp != NULL);
    fclose(fp);

    /* Scaffold Go Module */
    char goTarget[256];
    snprintf(goTarget, sizeof(goTarget), "%s/test_go_app", testBase);
    ok = templatesScaffold("go", goTarget, "my_go_mod", "Gopher");
    assert(ok == 1);
    snprintf(path, sizeof(path), "%s/go.mod", goTarget);
    fp = fopen(path, "r");
    assert(fp != NULL);
    fclose(fp);

    /* Scaffold Web Frontend */
    char webTarget[256];
    snprintf(webTarget, sizeof(webTarget), "%s/test_web_app", testBase);
    ok = templatesScaffold("web", webTarget, "my_web_site", "Frontend Dev");
    assert(ok == 1);
    snprintf(path, sizeof(path), "%s/index.html", webTarget);
    fp = fopen(path, "r");
    assert(fp != NULL);
    fclose(fp);

    printf("  -> Scaffolding Engine tests PASSED.\n");
}

static void test_template_wizard_controller(void)
{
    printf("[12/12] Testing Interactive Project Wizard Modal Controller...\n");

    assert(templateWizardIsActive() == 0);
    templateWizardOpen();
    assert(templateWizardIsActive() == 1);
    assert(templateWizard.focusedField == 0);

    /* Cycle templates using Arrow Right */
    int initialIdx = templateWizard.selectedIndex;
    templateWizardHandleKey(ARROW_RIGHT, 0);
    assert(templateWizard.selectedIndex == (initialIdx + 1) % templatesCount());

    /* Field cycling */
    templateWizardHandleKey('\t', 0);
    assert(templateWizard.focusedField == 1); /* Project name */
    templateWizardHandleKey('\t', 0);
    assert(templateWizard.focusedField == 2); /* Target dir */
    templateWizardHandleKey('\t', 0);
    assert(templateWizard.focusedField == 3); /* Author */
    templateWizardHandleKey('\t', 0);
    assert(templateWizard.focusedField == 0); /* Template selector */

    /* Cancel */
    templateWizardHandleKey(27, 0);
    assert(templateWizardIsActive() == 0);

    printf("  -> Template Wizard Controller tests PASSED.\n");
}

int main(void)
{
    printf("\n==================================================\n");
    printf("     Claw Phase 12 Test Suite (DAP, Remote, Templates)\n");
    printf("==================================================\n");

    test_dap_protocol_framing();
    test_dap_breakpoint_management();
    test_dap_gutter_status();
    test_dap_stepping_controls();
    test_dap_call_stack_and_variables();
    test_dap_dashboard_controller();
    test_remote_uri_parsing();
    test_remote_caching_and_vfs();
    test_remote_prompt_controller();
    test_project_template_registry();
    test_scaffolding_engine();
    test_template_wizard_controller();

    printf("\n==================================================\n");
    printf("   ALL PHASE 12 TESTS COMPLETED & PASSED!\n");
    printf("==================================================\n\n");

    return 0;
}
