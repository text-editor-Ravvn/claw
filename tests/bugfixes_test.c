#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "buffer.h"
#include "cursor.h"
#include "history.h"
#include "search.h"
#include "viewport.h"
#include "config.h"
#include "fileio.h"

extern Buffer buffer;
extern Cursor cursor;
extern Viewport viewport;
extern SearchState searchState;

/* Stubs for standalone testing */
static char lastStatusMsg[256];
void editorSetStatusMessage(const char *message)
{
    if (message)
        snprintf(lastStatusMsg, sizeof(lastStatusMsg), "%s", message);
}

void pluginTriggerEvent(int event)
{
    (void)event;
}

int terminalIsActive(void)
{
    return 0;
}

int splitIsActive(void)
{
    return 0;
}

void *splitGetActivePane(void)
{
    return NULL;
}

typedef struct {
    int showLineNumbers;
} DummyLayout;
DummyLayout layoutState = {0};

typedef struct {
    int height;
} DummyTerminal;
DummyTerminal integratedTerminal = {0};

void test_search_replace_infinite_loop_prevention(void)
{
    printf("[1/4] Testing Search & Replace infinite loop prevention...\n");

    bufferInit();
    /* Set up a row with multiple instances of "cat" */
    free(buffer.rows[0].chars);
    buffer.rows[0].chars = strdup("cat cat cat");
    buffer.rows[0].size = strlen(buffer.rows[0].chars);

    /* Search for "cat" */
    snprintf(searchState.query, sizeof(searchState.query), "cat");
    searchState.length = strlen(searchState.query);

    /* Replace with "cat dog" which contains "cat" */
    snprintf(searchState.replacement, sizeof(searchState.replacement), "cat dog");
    searchState.replacementLength = strlen(searchState.replacement);

    int count = replaceAllMatches();
    assert(count == 3);
    assert(strcmp(buffer.rows[0].chars, "cat dog cat dog cat dog") == 0);
    assert(buffer.rows[0].size == (int)strlen("cat dog cat dog cat dog"));
    assert(buffer.rows[0].hl == NULL); /* Highlight properly invalidated */

    bufferFree();
    clearSearchResults();
    printf("  -> Infinite loop prevention test PASSED.\n");
}

void test_search_replace_long_line_no_truncation(void)
{
    printf("[2/4] Testing Search & Replace long lines (>2048 bytes)...\n");

    bufferInit();
    /* Build a line with 3000 characters */
    char longLine[3001];
    memset(longLine, 'x', 3000);
    longLine[3000] = '\0';
    /* Put a match at the beginning and end */
    memcpy(longLine, "TARGET ", 7);
    memcpy(longLine + 2993, " TARGET", 7);

    free(buffer.rows[0].chars);
    buffer.rows[0].chars = strdup(longLine);
    buffer.rows[0].size = 3000;

    snprintf(searchState.query, sizeof(searchState.query), "TARGET");
    searchState.length = strlen(searchState.query);

    snprintf(searchState.replacement, sizeof(searchState.replacement), "REPLACED");
    searchState.replacementLength = strlen(searchState.replacement);

    int count = replaceAllMatches();
    assert(count == 2);
    /* Ensure no truncation occurred: 3000 - 2*6 + 2*8 = 3004 bytes */
    assert(buffer.rows[0].size == 3004);
    assert(strncmp(buffer.rows[0].chars, "REPLACED ", 9) == 0);

    bufferFree();
    clearSearchResults();
    printf("  -> Long line test PASSED.\n");
}

void test_smart_backspace_undo_integrity(void)
{
    printf("[3/4] Testing Smart Backspace and Undo integrity...\n");

    bufferInit();
    historyInit();

    /* Put 4 spaces */
    insertChar(' ');
    insertChar(' ');
    insertChar(' ');
    insertChar(' ');

    assert(buffer.rows[0].size == 4);
    assert(cursor.x == 4);

    /* Undo once: should only undo the 4th space, leaving 3 spaces! */
    historyUndo();

    assert(buffer.rows[0].size == 3);
    assert(cursor.x == 3);

    /* Undo remaining spaces */
    historyUndo();
    assert(buffer.rows[0].size == 2);
    historyUndo();
    assert(buffer.rows[0].size == 1);
    historyUndo();
    assert(buffer.rows[0].size == 0);

    bufferFree();
    printf("  -> Smart Backspace undo integrity test PASSED.\n");
}

void test_viewport_scroll_jitter_prevention(void)
{
    printf("[4/4] Testing Viewport Scroll jitter prevention in narrow viewports...\n");

    viewport.screenRows = 24;
    viewport.screenCols = 15; /* Narrow screen / pane */
    viewport.gutterWidth = 0;
    viewport.rowOffset = 0;
    viewport.colOffset = 0;

    cursor.y = 0;
    cursor.x = 7;

    scrollEditor();
    int off1 = viewport.colOffset;

    /* Call scrollEditor again with same cursor position */
    scrollEditor();
    int off2 = viewport.colOffset;

    /* Offset must be stable, not oscillating */
    assert(off1 == off2);

    printf("  -> Viewport scroll jitter test PASSED.\n");
}

void test_search_operator_and_symbol_matching(void)
{
    printf("[5/8] Testing Search matching for operators, punctuation & symbols...\n");

    bufferInit();
    free(buffer.rows[0].chars);
    buffer.rows[0].chars = strdup("int x = a->b + c.d == arr[0];");
    buffer.rows[0].size = (int)strlen(buffer.rows[0].chars);

    /* 1. Arrow operator -> */
    snprintf(searchState.query, sizeof(searchState.query), "->");
    searchState.length = strlen(searchState.query);
    performSearch();
    assert(searchState.matchCount == 1);
    assert(searchState.matchCols[0] == 9);

    /* 2. Plus operator + */
    snprintf(searchState.query, sizeof(searchState.query), "+");
    searchState.length = strlen(searchState.query);
    performSearch();
    assert(searchState.matchCount == 1);
    assert(searchState.matchCols[0] == 13);

    /* 3. Dot operator . */
    snprintf(searchState.query, sizeof(searchState.query), ".");
    searchState.length = strlen(searchState.query);
    performSearch();
    assert(searchState.matchCount == 1);
    assert(searchState.matchCols[0] == 16);

    /* 4. Equality operator == */
    snprintf(searchState.query, sizeof(searchState.query), "==");
    searchState.length = strlen(searchState.query);
    performSearch();
    assert(searchState.matchCount == 1);
    assert(searchState.matchCols[0] == 19);

    /* 5. Bracket [ */
    snprintf(searchState.query, sizeof(searchState.query), "[");
    searchState.length = strlen(searchState.query);
    performSearch();
    assert(searchState.matchCount == 1);
    assert(searchState.matchCols[0] == 25);

    /* Replace -> with --> */
    snprintf(searchState.query, sizeof(searchState.query), "->");
    searchState.length = strlen(searchState.query);
    snprintf(searchState.replacement, sizeof(searchState.replacement), "-->");
    searchState.replacementLength = strlen(searchState.replacement);
    performSearch();
    assert(searchState.matchCount == 1);
    replaceCurrentMatch();
    assert(strstr(buffer.rows[0].chars, "a-->b") != NULL);

    bufferFree();
    clearSearchResults();
    printf("  -> Operator and symbol search test PASSED.\n");
}

void test_negative_cursor_and_bounds_safety(void)
{
    printf("[6/8] Testing Negative cursor coordinates and boundary safety...\n");

    bufferInit();

    /* Put some initial text */
    insertChar('h');
    insertChar('i');
    assert(buffer.rows[0].size == 2);

    /* Corrupt cursor coordinates to negative values */
    cursor.x = -10;
    cursor.y = -5;

    /* Functions must safely guard or clamp without crashing */
    insertChar('!');
    deleteChar();
    deleteForward();
    moveCursorRight();
    moveCursorWordLeft();
    moveCursorWordRight();
    moveCursorUp();
    moveCursorDown();

    /* Extreme positive cursor coordinates */
    cursor.x = 99999;
    cursor.y = 99999;

    insertChar('?');
    deleteChar();
    deleteForward();
    moveCursorRight();
    moveCursorWordLeft();
    moveCursorWordRight();
    moveCursorUp();
    moveCursorDown();

    bufferFree();
    printf("  -> Negative cursor and boundary safety test PASSED.\n");
}

void test_replace_current_match_bounds_safety(void)
{
    printf("[7/8] Testing ReplaceCurrentMatch out-of-bounds safety...\n");

    bufferInit();

    /* Set up an out-of-bounds match record */
    searchState.matchCount = 1;
    searchState.currentMatch = 0;
    searchState.matchRows[0] = 50;  /* buffer only has 1 row */
    searchState.matchCols[0] = 500;
    snprintf(searchState.query, sizeof(searchState.query), "target");
    searchState.length = strlen(searchState.query);
    snprintf(searchState.replacement, sizeof(searchState.replacement), "replacement");
    searchState.replacementLength = strlen(searchState.replacement);

    /* Must safely return without segfaulting */
    replaceCurrentMatch();

    /* Now valid row but col out of bounds */
    searchState.matchRows[0] = 0;
    searchState.matchCols[0] = 500;
    replaceCurrentMatch();

    bufferFree();
    clearSearchResults();
    printf("  -> ReplaceCurrentMatch out-of-bounds safety test PASSED.\n");
}

void test_fileio_and_search_refresh_safety(void)
{
    printf("[8/8] Testing FileIO NULL guards and line movement search refresh...\n");

    /* FileIO NULL guards */
    assert(openFile(NULL) == 0);
    assert(saveFile(NULL) == 0);

    /* Line movement search refresh */
    bufferInit();
    free(buffer.rows[0].chars);
    buffer.rows[0].chars = strdup("alpha target");
    buffer.rows[0].size = strlen(buffer.rows[0].chars);

    /* Insert a second row at the end of the first line */
    cursor.y = 0;
    cursor.x = buffer.rows[0].size;
    insertNewLine();
    free(buffer.rows[1].chars);
    buffer.rows[1].chars = strdup("beta");
    buffer.rows[1].size = strlen(buffer.rows[1].chars);

    snprintf(searchState.query, sizeof(searchState.query), "target");
    searchState.length = strlen(searchState.query);
    performSearch();
    assert(searchState.matchCount == 1);
    assert(searchState.matchRows[0] == 0);

    /* Move line 0 down: row 0 becomes beta, row 1 becomes alpha target */
    cursor.y = 0;
    moveLineDown();
    assert(searchState.matchRows[0] == 1);

    bufferFree();
    clearSearchResults();
    printf("  -> FileIO and search refresh safety tests PASSED.\n");
}

void test_comment_toggle_search_refresh(void)
{
    printf("[9/9] Testing toggleCommentLine search refresh synchronization...\n");

    bufferInit();
    free(buffer.rows[0].chars);
    buffer.rows[0].chars = strdup("alpha target");
    buffer.rows[0].size = strlen(buffer.rows[0].chars);

    snprintf(searchState.query, sizeof(searchState.query), "target");
    searchState.length = strlen(searchState.query);
    performSearch();
    assert(searchState.matchCount == 1);
    assert(searchState.matchRows[0] == 0);
    assert(searchState.matchCols[0] == 6);

    /* Comment line: becomes "// alpha target", match col shifts from 6 to 9 */
    cursor.y = 0;
    cursor.x = 0;
    toggleCommentLine();
    assert(searchState.matchCount == 1);
    assert(searchState.matchRows[0] == 0);
    assert(searchState.matchCols[0] == 9);

    /* Uncomment line: returns to "alpha target", match col shifts back to 6 */
    toggleCommentLine();
    assert(searchState.matchCount == 1);
    assert(searchState.matchRows[0] == 0);
    assert(searchState.matchCols[0] == 6);

    bufferFree();
    clearSearchResults();
    printf("  -> Comment toggle search refresh test PASSED.\n");
}

void test_vertical_scroll_underflow_and_bounds(void)
{
    printf("[10/10] Testing Vertical scroll offset underflow and row bounds...\n");

    bufferInit();
    viewport.rowOffset = 5;
    viewport.colOffset = 5;

    /* Negative cursor coordinates */
    cursor.y = -10;
    cursor.x = -20;
    scrollEditor();
    assert(viewport.rowOffset == 0);
    assert(viewport.colOffset == 0);

    /* Normal scrolling bounds check */
    cursor.y = 50;
    cursor.x = 30;
    scrollEditor();
    assert(viewport.rowOffset > 0);
    assert(viewport.colOffset > 0);

    /* Bring back cursor and ensure no underflow */
    cursor.y = -1;
    cursor.x = -1;
    scrollEditor();
    assert(viewport.rowOffset == 0);
    assert(viewport.colOffset == 0);

    bufferFree();
    printf("  -> Vertical scroll underflow and bounds test PASSED.\n");
}

int main(void)
{
    printf("==================================================\n");
    printf("     Claw Bug Fixes Verification Test Suite       \n");
    printf("==================================================\n");

    test_search_replace_infinite_loop_prevention();
    test_search_replace_long_line_no_truncation();
    test_smart_backspace_undo_integrity();
    test_viewport_scroll_jitter_prevention();
    test_search_operator_and_symbol_matching();
    test_negative_cursor_and_bounds_safety();
    test_replace_current_match_bounds_safety();
    test_fileio_and_search_refresh_safety();
    test_comment_toggle_search_refresh();
    test_vertical_scroll_underflow_and_bounds();

    printf("==================================================\n");
    printf("        ALL BUG FIX TESTS PASSED!                 \n");
    printf("==================================================\n");
    return 0;
}
