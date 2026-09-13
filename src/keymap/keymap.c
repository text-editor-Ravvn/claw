#include <string.h>

#include "input.h"
#include "keymap.h"

static KeyBinding bindings[KEYMAP_MAX_BINDINGS];
static int bindingCount = 0;

void keymapInit(void) {
  /* Default bindings matching the original hardcoded keys. */
  bindingCount = 0;

  bindings[bindingCount].action = CMD_SAVE;
  bindings[bindingCount].key = 's';
  bindings[bindingCount].modifiers = MOD_CTRL;
  bindingCount++;

  bindings[bindingCount].action = CMD_GIT_ADD;
  bindings[bindingCount].key = 'g';
  bindings[bindingCount].modifiers = MOD_CTRL;
  bindingCount++;

  bindings[bindingCount].action = CMD_GIT_RESTORE;
  bindings[bindingCount].key = 't';
  bindings[bindingCount].modifiers = MOD_CTRL;
  bindingCount++;

  bindings[bindingCount].action = CMD_GIT_UNSTAGE;
  bindings[bindingCount].key = 'u';
  bindings[bindingCount].modifiers = MOD_CTRL;
  bindingCount++;

  bindings[bindingCount].action = CMD_GIT_BLAME;
  bindings[bindingCount].key = 'b';
  bindings[bindingCount].modifiers = MOD_CTRL;
  bindingCount++;

  bindings[bindingCount].action = CMD_PLUGIN_LIST;
  bindings[bindingCount].key = 'l';
  bindings[bindingCount].modifiers = MOD_CTRL;
  bindingCount++;

  bindings[bindingCount].action = CMD_PLUGIN_RELOAD;
  bindings[bindingCount].key = 'k';
  bindings[bindingCount].modifiers = MOD_CTRL;
  bindingCount++;

  bindings[bindingCount].action = CMD_PLUGIN_INFO;
  bindings[bindingCount].key = 'i';
  bindings[bindingCount].modifiers = MOD_CTRL;
  bindingCount++;

  bindings[bindingCount].action = CMD_PLUGIN_HELLO;
  bindings[bindingCount].key = 'h';
  bindings[bindingCount].modifiers = MOD_CTRL;
  bindingCount++;

  bindings[bindingCount].action = CMD_PLUGIN_STATS;
  bindings[bindingCount].key = 'j';
  bindings[bindingCount].modifiers = MOD_CTRL;
  bindingCount++;

  bindings[bindingCount].action = CMD_PLUGIN_FORMAT;
  bindings[bindingCount].key = 'o';
  bindings[bindingCount].modifiers = MOD_CTRL;
  bindingCount++;

  bindings[bindingCount].action = CMD_PLUGIN_CLEAR_LOGS;
  bindings[bindingCount].key = 'd';
  bindings[bindingCount].modifiers = MOD_CTRL;
  bindingCount++;

  bindings[bindingCount].action = CMD_PLUGIN_LOG;
  bindings[bindingCount].key = 'e';
  bindings[bindingCount].modifiers = MOD_CTRL;
  bindingCount++;

  bindings[bindingCount].action = CMD_PLUGIN_MANAGER;
  bindings[bindingCount].key = 'm';
  bindings[bindingCount].modifiers = MOD_CTRL;
  bindingCount++;

  bindings[bindingCount].action = CMD_PLUGIN_TOGGLE;
  bindings[bindingCount].key = 'v';
  bindings[bindingCount].modifiers = MOD_CTRL;
  bindingCount++;

  bindings[bindingCount].action = CMD_PLUGIN_HEALTH;
  bindings[bindingCount].key = 'h';
  bindings[bindingCount].modifiers = MOD_ALT;
  bindingCount++;

  bindings[bindingCount].action = CMD_PLUGIN_HELP;
  bindings[bindingCount].key = 'w';
  bindings[bindingCount].modifiers = MOD_CTRL;
  bindingCount++;
  
  bindings[bindingCount].action = CMD_QUIT;
  bindings[bindingCount].key = 'x';
  bindings[bindingCount].modifiers = MOD_CTRL;
  bindingCount++;

  bindings[bindingCount].action = CMD_SEARCH;
  bindings[bindingCount].key = 'f';
  bindings[bindingCount].modifiers = MOD_CTRL;
  bindingCount++;

  bindings[bindingCount].action = CMD_REPLACE;
  bindings[bindingCount].key = 'r';
  bindings[bindingCount].modifiers = MOD_CTRL;
  bindingCount++;

  bindings[bindingCount].action = CMD_REPLACE_ALL;
  bindings[bindingCount].key = 'a';
  bindings[bindingCount].modifiers = MOD_CTRL;
  bindingCount++;

  bindings[bindingCount].action = CMD_UNDO;
  bindings[bindingCount].key = 'z';
  bindings[bindingCount].modifiers = MOD_CTRL;
  bindingCount++;

  bindings[bindingCount].action = CMD_REDO;
  bindings[bindingCount].key = 'y';
  bindings[bindingCount].modifiers = MOD_CTRL;
  bindingCount++;

  bindings[bindingCount].action = CMD_NEXT_MATCH;
  bindings[bindingCount].key = 'n';
  bindings[bindingCount].modifiers = MOD_CTRL;
  bindingCount++;

  bindings[bindingCount].action = CMD_PREV_MATCH;
  bindings[bindingCount].key = 'p';
  bindings[bindingCount].modifiers = MOD_CTRL;
  bindingCount++;
}

int keymapLookup(
    int key,
    int modifiers
) {
  for (int i = 0;
     i < bindingCount;
     i++)
{
    if (
        bindings[i].key == key &&
        bindings[i].modifiers == modifiers
    )
    {
        return bindings[i].action;
    }
}

return CMD_NONE;
}

/* Update an existing binding or append a new one. */
static void keymapSetBinding(
    int action,
    int key,
    int modifiers
)
{
    for (
        int i = 0;
        i < bindingCount;
        i++
    )
    {
        if (
            bindings[i].action ==
            action
        )
        {
            bindings[i].key = key;
            bindings[i].modifiers =
                modifiers;

            return;
        }
    }

    if (
        bindingCount <
        KEYMAP_MAX_BINDINGS
    )
    {
        bindings[bindingCount].action =
            action;

        bindings[bindingCount].key =
            key;

        bindings[bindingCount].modifiers =
            modifiers;

        bindingCount++;
    }
}

int keymapLoad(const char *path) {
  /* Reuse the config-style parser from keymap/parser.c. */
  extern int keymapParseFile(
    const char *path,
    void (*setter)(
        int,
        int,
        int
    )
);
  return keymapParseFile(path, keymapSetBinding);
}
