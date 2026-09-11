#include <string.h>

#include "input.h"
#include "keymap.h"

static KeyBinding bindings[KEYMAP_MAX_BINDINGS];
static int bindingCount = 0;

void keymapInit(void) {
  /* Default bindings matching the original hardcoded keys. */
  bindingCount = 0;

  bindings[bindingCount].action = CMD_SAVE;
  bindings[bindingCount].key = CTRL_KEY('s');
  bindingCount++;

  bindings[bindingCount].action = CMD_GIT_ADD;
  bindings[bindingCount].key = CTRL_KEY('g');
  bindingCount++;

  bindings[bindingCount].action = CMD_GIT_RESTORE;
  bindings[bindingCount].key = CTRL_KEY('t');
  bindingCount++;

  bindings[bindingCount].action = CMD_GIT_UNSTAGE;
  bindings[bindingCount].key = CTRL_KEY('u');
  bindingCount++;

  bindings[bindingCount].action = CMD_GIT_BLAME;
  bindings[bindingCount].key = CTRL_KEY('b');
  bindingCount++;

  bindings[bindingCount].action = CMD_PLUGIN_LIST;
  bindings[bindingCount].key = CTRL_KEY('l');
  bindingCount++;

  bindings[bindingCount].action = CMD_PLUGIN_RELOAD;
  bindings[bindingCount].key = CTRL_KEY('k');
  bindingCount++;

  bindings[bindingCount].action = CMD_PLUGIN_HELLO;
  bindings[bindingCount].key = CTRL_KEY('h');
  bindingCount++;

  bindings[bindingCount].action = CMD_PLUGIN_STATS;
  bindings[bindingCount].key = CTRL_KEY('j');
  bindingCount++;

  bindings[bindingCount].action = CMD_PLUGIN_FORMAT;
  bindings[bindingCount].key = CTRL_KEY('o');
  bindingCount++;

  bindings[bindingCount].action = CMD_QUIT;
  bindings[bindingCount].key = CTRL_KEY('x');
  bindingCount++;

  bindings[bindingCount].action = CMD_SEARCH;
  bindings[bindingCount].key = CTRL_KEY('f');
  bindingCount++;

  bindings[bindingCount].action = CMD_REPLACE;
  bindings[bindingCount].key = CTRL_KEY('r');
  bindingCount++;

  bindings[bindingCount].action = CMD_REPLACE_ALL;
  bindings[bindingCount].key = CTRL_KEY('a');
  bindingCount++;

  bindings[bindingCount].action = CMD_UNDO;
  bindings[bindingCount].key = CTRL_KEY('z');
  bindingCount++;

  bindings[bindingCount].action = CMD_REDO;
  bindings[bindingCount].key = CTRL_KEY('y');
  bindingCount++;

  bindings[bindingCount].action = CMD_NEXT_MATCH;
  bindings[bindingCount].key = CTRL_KEY('n');
  bindingCount++;

  bindings[bindingCount].action = CMD_PREV_MATCH;
  bindings[bindingCount].key = CTRL_KEY('p');
  bindingCount++;
}

int keymapLookup(int key) {
  for (int i = 0; i < bindingCount; i++) {
    if (bindings[i].key == key)
      return bindings[i].action;
  }

  return CMD_NONE;
}

/* Update an existing binding or append a new one. */
static void keymapSetBinding(int action, int key) {
  for (int i = 0; i < bindingCount; i++) {
    if (bindings[i].action == action) {
      bindings[i].key = key;
      return;
    }
  }

  if (bindingCount < KEYMAP_MAX_BINDINGS) {
    bindings[bindingCount].action = action;
    bindings[bindingCount].key = key;
    bindingCount++;
  }
}

int keymapLoad(const char *path) {
  /* Reuse the config-style parser from keymap/parser.c. */
  extern int keymapParseFile(const char *path, void (*setter)(int, int));

  return keymapParseFile(path, keymapSetBinding);
}
