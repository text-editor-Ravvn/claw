#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "buffer.h"
#include "cursor.h"
#include "viewport.h"
#include "editor.h"
#include "buffer_manager.h"
#include "split.h"
#include "terminal.h"
#include "symbols.h"
#include "input.h"
#include "keymap.h"
#include "workspace.h"
#include "project.h"

/* Mock / stub global dependencies required by modules when tested in isolation */
Viewport viewport = {0};
char *currentFile = NULL;
SavePrompt savePrompt = {0};
GotoLinePrompt gotoPrompt = {0};
GitViewState gitView = {0};

static char testStatusMsg[256] = "";

const char *editorStatusMessage(void)
{
    return testStatusMsg;
}

void editorSetStatusMessage(const char *message)
{
    if (message)
        snprintf(testStatusMsg, sizeof(testStatusMsg), "%s", message);
}

void highlightInit(void)
{
}

void gitRefreshStatus(const char *file)
{
    (void)file;
}

int openFile(const char *filename)
{
    if (!filename)
        return 0;

    FILE *fp = fopen(filename, "r");
    if (!fp)
        return 0;

    bufferFree();
    bufferInit();

    char line[512];
    int first = 1;
    while (fgets(line, sizeof(line), fp))
    {
        line[strcspn(line, "\r\n")] = '\0';
        if (first)
        {
            free(buffer.rows[0].chars);
            buffer.rows[0].chars = strdup(line);
            buffer.rows[0].size = (int)strlen(line);
            first = 0;
        }
        else
        {
            buffer.rows = realloc(buffer.rows, sizeof(Row) * (buffer.numRows + 1));
            buffer.rows[buffer.numRows].chars = strdup(line);
            buffer.rows[buffer.numRows].size = (int)strlen(line);
            buffer.rows[buffer.numRows].hl = NULL;
            buffer.rows[buffer.numRows].hlSize = 0;
            buffer.numRows++;
        }
    }
    fclose(fp);
    return 1;
}

int saveFile(const char *filename)
{
    if (!filename)
        return 0;
    FILE *fp = fopen(filename, "w");
    if (!fp)
        return 0;
    for (int i = 0; i < buffer.numRows; i++)
    {
        fprintf(fp, "%s\n", buffer.rows[i].chars);
    }
    fclose(fp);
    return 1;
}

/* Helper to simulate escape sequences without requiring raw stdin */
static KeyEvent parseCsiSeq(const char *seq)
{
    KeyEvent event = {0, 0};
    int code = 0, mod = 0;

    if (sscanf(seq, "[27;%d;%d~", &mod, &code) == 2)
    {
        int m = 0;
        if (mod == 2) m = MOD_SHIFT;
        else if (mod == 3) m = MOD_ALT;
        else if (mod == 5) m = MOD_CTRL;
        else if (mod == 6) m = MOD_CTRL | MOD_SHIFT;
        event.key = code;
        event.modifiers = m;
        return event;
    }

    if (sscanf(seq, "[%d;%du", &code, &mod) == 2)
    {
        int m = 0;
        if (mod == 2) m = MOD_SHIFT;
        else if (mod == 3) m = MOD_ALT;
        else if (mod == 5) m = MOD_CTRL;
        else if (mod == 6) m = MOD_CTRL | MOD_SHIFT;
        event.key = code;
        event.modifiers = m;
        return event;
    }

    if (sscanf(seq, "[1;%dI", &mod) == 1)
    {
        int m = (mod == 5) ? MOD_CTRL : (mod == 6 ? (MOD_CTRL | MOD_SHIFT) : 0);
        event.key = '\t';
        event.modifiers = m;
        return event;
    }

    if (sscanf(seq, "[1;%dZ", &mod) == 1)
    {
        int m = (mod == 5) ? MOD_CTRL : (mod == 6 ? (MOD_CTRL | MOD_SHIFT) : MOD_SHIFT);
        event.key = '\t';
        event.modifiers = m;
        return event;
    }

    if (strcmp(seq, "[5~") == 0)
    {
        event.key = PAGE_UP;
        return event;
    }

    if (strcmp(seq, "[6~") == 0)
    {
        event.key = PAGE_DOWN;
        return event;
    }

    return event;
}

int main(void)
{
    printf("==========================================\n");
    printf("     Claw Phase 8 Test Suite (IDE)        \n");
    printf("==========================================\n");

    /* -------------------------------------------------- */
    /* 1. Test Buffer Manager                             */
    /* -------------------------------------------------- */
    printf("[1/5] Testing Buffer Manager (Multi-Buffer)...\n");

    bufferInit();
    bufferManagerInit();

    assert(bufferManagerCount() == 1);
    assert(bufferManagerActiveIndex() == 0);
    assert(strcmp(bufferManagerGetDisplayName(0), "[Untitled]") == 0);

    /* Create secondary and tertiary buffers */
    int b2 = bufferManagerNew("notes.txt");
    assert(b2 == 1);
    assert(bufferManagerCount() == 2);
    assert(bufferManagerActiveIndex() == 1);
    assert(strcmp(bufferManagerGetDisplayName(1), "notes.txt") == 0);

    int b3 = bufferManagerNew(NULL); /* Untitled buffer */
    assert(b3 == 2);
    assert(bufferManagerCount() == 3);
    assert(bufferManagerActiveIndex() == 2);

    /* Buffer cycling */
    assert(bufferManagerNext() == 1);
    assert(bufferManagerActiveIndex() == 0);

    assert(bufferManagerNext() == 1);
    assert(bufferManagerActiveIndex() == 1);

    assert(bufferManagerPrev() == 1);
    assert(bufferManagerActiveIndex() == 0);

    /* Direct jump */
    assert(bufferManagerSwitchTo(2) == 1);
    assert(bufferManagerActiveIndex() == 2);
    assert(bufferManagerSwitchTo(99) == 0); /* Out of bounds */

    /* Buffer close */
    assert(bufferManagerClose(1) == 1); /* Close notes.txt */
    assert(bufferManagerCount() == 2);

    /* Test opening real file into bufferManager */
    FILE *sample = fopen("tests/sample_p8.txt", "w");
    assert(sample != NULL);
    fprintf(sample, "hello phase 8\nsecond line\n");
    fclose(sample);

    int bOpen = bufferManagerOpen("tests/sample_p8.txt");
    assert(bOpen >= 0);
    assert(bufferManagerFind("tests/sample_p8.txt") == bOpen);
    assert(buffer.numRows == 2);
    assert(strcmp(buffer.rows[0].chars, "hello phase 8") == 0);

    /* Opening the same file again should switch to it, not duplicate */
    int curCount = bufferManagerCount();
    int bOpenAgain = bufferManagerOpen("tests/sample_p8.txt");
    assert(bOpenAgain == bOpen);
    assert(bufferManagerCount() == curCount);

    /* Edge Case: Fill buffers up to BUFFER_MAX_COUNT (32) */
    while (bufferManagerCount() < BUFFER_MAX_COUNT)
    {
        char nameBuf[32];
        snprintf(nameBuf, sizeof(nameBuf), "buf_%d.txt", bufferManagerCount());
        int idx = bufferManagerNew(nameBuf);
        assert(idx >= 0);
    }
    assert(bufferManagerCount() == BUFFER_MAX_COUNT);
    /* 33rd buffer creation must be safely rejected */
    assert(bufferManagerNew("overflow.txt") == -1);

    /* Arbitrary closing order test across all 32 buffers */
    assert(bufferManagerClose(10) == 1);
    assert(bufferManagerCount() == BUFFER_MAX_COUNT - 1);
    assert(bufferManagerClose(0) == 1);
    assert(bufferManagerClose(bufferManagerCount() - 1) == 1);

    /* Clean up buffers back to 1 */
    while (bufferManagerCount() > 1)
    {
        bufferManagerClose(bufferManagerCount() - 1);
    }
    assert(bufferManagerCount() == 1);

    remove("tests/sample_p8.txt");
    printf("  -> Buffer Manager tests PASSED.\n");

    /* -------------------------------------------------- */
    /* 2. Test Split-Screen Architecture                  */
    /* -------------------------------------------------- */
    printf("[2/5] Testing Split-Screen Layout (Panes)...\n");

    splitInit();
    assert(splitIsActive() == 0);
    assert(splitGetMode() == SPLIT_NONE);

    viewport.screenRows = 30;
    viewport.screenCols = 100;

    /* Create second buffer for pane 1 */
    bufferManagerNew("pane2_file.txt");

    /* Horizontal Split */
    splitHorizontal();
    assert(splitIsActive() == 1);
    assert(splitGetMode() == SPLIT_HORIZONTAL);
    assert(splitLayout.panes[0].height > 0);
    assert(splitLayout.panes[1].height > 0);
    assert(splitLayout.panes[0].height + 1 + splitLayout.panes[1].height == 30 - 2);
    assert(splitLayout.activePane == 0);

    /* Switch Pane Focus */
    splitSwitchPane();
    assert(splitLayout.activePane == 1);
    splitSwitchPane();
    assert(splitLayout.activePane == 0);

    /* Vertical Split with standard terminal */
    splitVertical();
    assert(splitIsActive() == 1);
    assert(splitGetMode() == SPLIT_VERTICAL);
    assert(splitLayout.panes[0].width > 0);
    assert(splitLayout.panes[1].width > 0);
    assert(splitLayout.panes[0].width + 1 + splitLayout.panes[1].width == 100);

    /* Edge Case: Narrow terminal width (< 40 cols) */
    splitUpdateDimensions(30, 31);
    assert(splitLayout.panes[0].width == 15);
    assert(splitLayout.panes[1].width == 15);

    /* Close Split returns to single pane */
    splitClose();
    assert(splitIsActive() == 0);
    assert(splitGetMode() == SPLIT_NONE);
    printf("  -> Split-Screen tests PASSED.\n");

    /* -------------------------------------------------- */
    /* 3. Test Integrated Terminal & Runner               */
    /* -------------------------------------------------- */
    printf("[3/5] Testing Integrated Terminal...\n");

    terminalInit();
    assert(terminalIsActive() == 0);

    terminalToggle();
    assert(terminalIsActive() == 1);

    terminalAppendOutput("Output line 1");
    terminalAppendOutput("Output line 2");
    assert(integratedTerminal.lineCount >= 2);

    /* Test builtins */
    terminalExecuteCommand("clear");
    assert(integratedTerminal.lineCount == 0);

    terminalExecuteCommand("pwd");
    assert(integratedTerminal.lineCount == 2); /* prompt + pwd */

    terminalExecuteCommand("help");
    assert(integratedTerminal.lineCount > 2);

    /* Test history */
    assert(integratedTerminal.historyCount >= 3);
    assert(strcmp(integratedTerminal.history[0], "clear") == 0);

    /* Test shell execution */
    terminalExecuteCommand("echo claw_terminal_ok");
    int foundEcho = 0;
    for (int i = 0; i < integratedTerminal.lineCount; i++)
    {
        if (strstr(integratedTerminal.lines[i], "claw_terminal_ok"))
            foundEcho = 1;
    }
    assert(foundEcho == 1);

    /* Test in-line terminal cursor navigation and editing */
    KeyEvent keyEv = {0, 0};

    /* Type 'abc' */
    keyEv.key = 'a'; terminalHandleKey(keyEv);
    keyEv.key = 'b'; terminalHandleKey(keyEv);
    keyEv.key = 'c'; terminalHandleKey(keyEv);
    assert(strcmp(integratedTerminal.currentCommand, "abc") == 0);
    assert(integratedTerminal.cursorCol == 3);

    /* Arrow Left to between 'b' and 'c' */
    keyEv.key = ARROW_LEFT; terminalHandleKey(keyEv);
    assert(integratedTerminal.cursorCol == 2);

    /* Insert 'X' -> 'abXc' */
    keyEv.key = 'X'; terminalHandleKey(keyEv);
    assert(strcmp(integratedTerminal.currentCommand, "abXc") == 0);
    assert(integratedTerminal.cursorCol == 3);

    /* Home key -> cursorCol = 0 */
    keyEv.key = HOME_KEY; terminalHandleKey(keyEv);
    assert(integratedTerminal.cursorCol == 0);

    /* End key -> cursorCol = 4 */
    keyEv.key = END_KEY; terminalHandleKey(keyEv);
    assert(integratedTerminal.cursorCol == 4);

    /* Backspace at end -> 'abX' */
    keyEv.key = 127; terminalHandleKey(keyEv);
    assert(strcmp(integratedTerminal.currentCommand, "abX") == 0);
    assert(integratedTerminal.cursorCol == 3);

    /* Ctrl+C clears command line */
    keyEv.key = 'c'; keyEv.modifiers = MOD_CTRL; terminalHandleKey(keyEv);
    assert(integratedTerminal.commandLen == 0);
    assert(integratedTerminal.cursorCol == 0);

    /* Output scrolling */
    int beforeScroll = integratedTerminal.scrollOffset;
    keyEv.key = PAGE_UP; keyEv.modifiers = 0; terminalHandleKey(keyEv);
    assert(integratedTerminal.scrollOffset >= beforeScroll);

    terminalToggle();
    assert(terminalIsActive() == 0);
    printf("  -> Integrated Terminal tests PASSED.\n");

    /* -------------------------------------------------- */
    /* 4. Test Code Symbol Indexing & Navigation          */
    /* -------------------------------------------------- */
    printf("[4/5] Testing Symbol Indexer & Navigation...\n");

    FILE *cfile = fopen("tests/sample_code.c", "w");
    assert(cfile != NULL);
    fprintf(cfile,
        "#define CLAW_MAX_BUFFERS 64\n"
        "struct EditorState {\n"
        "    int width;\n"
        "};\n"
        "enum EditorMode { MODE_NORMAL };\n"
        "void editorInitializeWorkspace(void)\n"
        "int calculateScore(int a, int b)\n"
        "    printf(\"function call inside body\");\n"
        "    int res = calculateScore(1, 2);\n");
    fclose(cfile);

    int bCodeIdx = bufferManagerOpen("tests/sample_code.c");
    assert(bCodeIdx >= 0);

    symbolsOpen();
    assert(symbolsIsActive() == 1);
    assert(symbolState.count >= 4);

    /* Verify that indented calls (printf, calculateScore inside body) are NOT added as symbols */
    int printfFound = 0;
    for (int i = 0; i < symbolState.count; i++)
    {
        if (strcmp(symbolState.symbols[i].name, "printf") == 0)
            printfFound = 1;
    }
    assert(printfFound == 0);

    /* Filter symbols */
    snprintf(symbolState.query, sizeof(symbolState.query), "initialize");
    symbolState.queryLen = strlen(symbolState.query);
    symbolsFilter();

    assert(symbolState.filteredCount == 1);
    const Symbol *s = symbolsGetSelected();
    assert(s != NULL);
    assert(strstr(s->name, "editorInitializeWorkspace") != NULL);
    assert(s->kind == SYMBOL_FUNCTION);
    assert(s->line == 6);

    /* Jump to symbol */
    int jumpOk = symbolsJumpToSelected();
    assert(jumpOk == 1);
    assert(symbolsIsActive() == 0);
    assert(cursor.y == 5); /* 0-indexed line 6 */

    remove("tests/sample_code.c");
    printf("  -> Symbol Indexer tests PASSED.\n");

    /* -------------------------------------------------- */
    /* 5. Test Keymap & Input Decoder Integration         */
    /* -------------------------------------------------- */
    printf("[5/5] Testing Keymap & Input Decoding...\n");

    keymapInit();

    /* Keymap lookup for Phase 8 commands */
    assert(keymapLookup('w', MOD_CTRL) == CMD_BUFFER_CLOSE);
    assert(keymapLookup('\t', MOD_CTRL) == CMD_BUFFER_NEXT);
    assert(keymapLookup('\t', MOD_CTRL | MOD_SHIFT) == CMD_BUFFER_PREV);
    assert(keymapLookup('\\', MOD_CTRL) == CMD_SPLIT_VERTICAL);
    assert(keymapLookup('-', MOD_CTRL) == CMD_SPLIT_HORIZONTAL);
    assert(keymapLookup('`', MOD_CTRL) == CMD_TERMINAL_TOGGLE);
    assert(keymapLookup('o', MOD_ALT) == CMD_PANE_SWITCH);
    assert(keymapLookup('s', MOD_CTRL | MOD_SHIFT) == CMD_SYMBOL_SEARCH);
    assert(keymapLookup('1', MOD_CTRL) == CMD_BUFFER_1);
    assert(keymapLookup('2', MOD_CTRL) == CMD_BUFFER_2);

    /* Input decoding for special control keys */
    KeyEvent ev1 = decodeKeyChar(28); /* Ctrl+\ */
    assert(ev1.key == '\\');
    assert(ev1.modifiers == MOD_CTRL);

    KeyEvent ev2 = decodeKeyChar(31); /* Ctrl+- */
    assert(ev2.key == '-');
    assert(ev2.modifiers == MOD_CTRL);

    KeyEvent ev3 = decodeKeyChar(0);  /* Ctrl+` */
    assert(ev3.key == '`');
    assert(ev3.modifiers == MOD_CTRL);

    /* CSI 27 and CSI u modern terminal sequence decoding */
    KeyEvent csiTab = parseCsiSeq("[27;5;9~");
    assert(csiTab.key == '\t');
    assert(csiTab.modifiers == MOD_CTRL);

    KeyEvent csiShiftTab = parseCsiSeq("[27;6;9~");
    assert(csiShiftTab.key == '\t');
    assert(csiShiftTab.modifiers == (MOD_CTRL | MOD_SHIFT));

    KeyEvent csi1 = parseCsiSeq("[27;5;49~");
    assert(csi1.key == '1');
    assert(csi1.modifiers == MOD_CTRL);

    KeyEvent csiKittyTab = parseCsiSeq("[9;5u");
    assert(csiKittyTab.key == '\t');
    assert(csiKittyTab.modifiers == MOD_CTRL);

    KeyEvent csiPgUp = parseCsiSeq("[5~");
    assert(csiPgUp.key == PAGE_UP);

    KeyEvent csiPgDn = parseCsiSeq("[6~");
    assert(csiPgDn.key == PAGE_DOWN);

    printf("  -> Keymap & Input tests PASSED.\n");

    /* Free all buffers before exit to ensure zero leaks under ASan */
    bufferManagerFreeAll();
    bufferFree();

    printf("\n==========================================\n");
    printf("   ALL PHASE 8 TESTS COMPLETED & PASSED!  \n");
    printf("==========================================\n");
    return 0;
}
