#include <assert.h>
#include <stdio.h>
#include "input.h"
#include "keymap.h"

int main(void)
{
    /* Test 1: Enter / Return decoding */
    KeyEvent ev1 = decodeKeyChar('\r');
    assert(ev1.key == '\r');
    assert(ev1.modifiers == 0);

    KeyEvent ev2 = decodeKeyChar('\n');
    assert(ev2.key == '\r');
    assert(ev2.modifiers == 0);

    /* Test 2: Tab decoding */
    KeyEvent ev3 = decodeKeyChar('\t');
    assert(ev3.key == '\t');
    assert(ev3.modifiers == 0);

    /* Test 3: Backspace decoding (BS and DEL) */
    KeyEvent ev4 = decodeKeyChar(8);
    assert(ev4.key == 127);
    assert(ev4.modifiers == 0);

    KeyEvent ev5 = decodeKeyChar(127);
    assert(ev5.key == 127);
    assert(ev5.modifiers == 0);

    /* Test 4: Ctrl key combinations (Ctrl+A, Ctrl+S, Ctrl+X) */
    KeyEvent ev6 = decodeKeyChar(CTRL_KEY('a'));
    assert(ev6.key == 'a');
    assert(ev6.modifiers == MOD_CTRL);

    KeyEvent ev7 = decodeKeyChar(CTRL_KEY('s'));
    assert(ev7.key == 's');
    assert(ev7.modifiers == MOD_CTRL);

    KeyEvent ev8 = decodeKeyChar(CTRL_KEY('x'));
    assert(ev8.key == 'x');
    assert(ev8.modifiers == MOD_CTRL);

    /* Test 5: Normal characters */
    KeyEvent ev9 = decodeKeyChar('z');
    assert(ev9.key == 'z');
    assert(ev9.modifiers == 0);

    /* Test 6: Keymap lookups */
    keymapInit();

    /* Enter, Tab, Backspace MUST NOT map to any command */
    assert(keymapLookup('\r', 0) == CMD_NONE);
    assert(keymapLookup('\n', 0) == CMD_NONE);
    assert(keymapLookup('\t', 0) == CMD_NONE);
    assert(keymapLookup(127, 0) == CMD_NONE);

    /* Standard editor commands */
    assert(keymapLookup('s', MOD_CTRL) == CMD_SAVE);
    assert(keymapLookup('x', MOD_CTRL) == CMD_QUIT);

    /* Plugin and line operation commands accessible via Alt */
    assert(keymapLookup('m', MOD_ALT) == CMD_PLUGIN_MANAGER);
    assert(keymapLookup('p', MOD_ALT) == CMD_PLUGIN_STATS);
    assert(keymapLookup('i', MOD_ALT) == CMD_PLUGIN_INFO);
    assert(keymapLookup('j', MOD_ALT) == CMD_JOIN_LINES);
    assert(keymapLookup('h', MOD_ALT) == CMD_PLUGIN_HEALTH);

    /* Phase 5 workspace navigation commands */
    assert(keymapLookup('g', MOD_ALT) == CMD_GOTO_LINE);
    assert(keymapLookup('b', MOD_ALT) == CMD_BOOKMARK_TOGGLE);
    assert(keymapLookup('c', MOD_ALT) == CMD_BOOKMARK_CLEAR);
    assert(keymapLookup('q', MOD_ALT) == CMD_QUICK_SWITCH);
    assert(keymapLookup('e', MOD_ALT) == CMD_FILE_EXPLORER);
    assert(keymapLookup('w', MOD_ALT) == CMD_WORKSPACE_OPEN);
    assert(keymapLookup('s', MOD_ALT) == CMD_SESSION_SAVE);
    assert(keymapLookup('r', MOD_ALT) == CMD_SESSION_RESTORE);

    printf("All input and keymap tests passed!\n");
    return 0;
}
